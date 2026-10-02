# -*- coding: utf-8 -*-
"""把 `out/package/` 下的发布包挂到 Gitee 的「发行版」上（OpenAPI v5，只用标准库）。

两步调用（见 Gitee OpenAPI v5 的 repositories 组）：

    1. `POST /api/v5/repos/{owner}/{repo}/releases`        建发行版（`tag_name` 必须是远端已有的 tag）
    2. `POST /api/v5/repos/{owner}/{repo}/releases/{id}/attach_files`   逐个上传附件（multipart，字段名 `file`）

`{owner}/{repo}` 默认从 `git remote get-url origin` 解析，不用手填。发行版说明默认取该 tag 的
注解原文——tag 说明里已经写明了包内容与验证结论，两处共用一份文本，免得各写一遍再对不上。
**但 tag 一旦推上去，其注解就改不动了**（改它要 force-push）；注解里若写了「未推送」这类**会过期**的
状态，就别拿它当发行版说明——用 `--body-file` 另给一份（写得过期的状态不入 tag 注解，或推 tag 前先
把状态句删掉）。

**令牌**：先看环境变量 `GITEE_TOKEN`，为空再看 `--token-file` 指的文件，都没给就看默认的
`out/gitee_token.txt`（`out/*` 在 `.gitignore` 里，令牌放这儿不会被提交）。**令牌可长期复用**——
写一次这个文件，此后发布就只是 `python tools/publish_release.py`。令牌只放进**请求体**、
不进 URL（URL 会进服务端访问日志），也不打印；出错回显服务端报文前会先把令牌字样抹掉。

**不会重复发版**：同一个 tag 已经发过发行版时 Gitee 会报错，脚本据此提示改用
`--release-id <id>` 直接往已有发行版补传附件，而不是再建一个。

**发布是外发动作**：附件一旦上传即公开可下载。故先跑 `--dry-run` 看清要传什么、传到哪。

用法（从仓根运行，`package_release` 与它同目录，靠脚本目录进 `sys.path` 导入）：

    python tools/publish_release.py --dry-run                 # 只打印计划，不发任何请求
    python tools/publish_release.py                           # 建发行版 + 传附件（令牌看环境变量或 out/gitee_token.txt）
    python tools/publish_release.py --release-id 123456       # 只往已有发行版补传附件
    python tools/publish_release.py --body-file out/release_body.txt        # 说明另给一份
    GITEE_TOKEN=xxx python tools/publish_release.py --token-file out/other_token.txt

退出码：
    0  发行版与附件都已就绪（或 `--dry-run` 走完）
    1  发行版或某个附件没成（HTTP 非 2xx）
    2  用法/环境错误（缺令牌、tag 没推到远端、包里找不到附件、参数不认识）

**只用标准库**，无第三方依赖。
"""
from __future__ import annotations

import argparse
import json
import os
import subprocess
import sys
import urllib.error
import urllib.request
import uuid
from pathlib import Path

import package_release

EXIT_OK = 0
EXIT_PUBLISH_FAILED = 1
EXIT_USAGE = 2

GITEE_API_ROOT = 'https://gitee.com/api/v5'
TOKEN_ENVIRONMENT_VARIABLE = 'GITEE_TOKEN'
DEFAULT_TOKEN_FILE = package_release.REPO_ROOT / 'out' / 'gitee_token.txt'
REQUEST_TIMEOUT_SECONDS = 120
REDACTED_TOKEN_TEXT = '***'


class PublishError(Exception):
    """发布未成。`exit_code` 决定进程退出码：1＝HTTP 没成，2＝用法/环境错误。"""

    def __init__(self, message: str, exit_code: int) -> None:
        super().__init__(message)
        self.exit_code = exit_code


def usage_error(message: str) -> PublishError:
    return PublishError(message, EXIT_USAGE)


def publish_error(message: str) -> PublishError:
    return PublishError(message, EXIT_PUBLISH_FAILED)


def run_git(arguments: list[str]) -> str:
    completed = subprocess.run(['git', *arguments], capture_output=True, text=True, encoding='utf-8')
    if completed.returncode != 0:
        raise usage_error(f'`git {" ".join(arguments)}` 失败：{completed.stderr.strip()}')
    return completed.stdout.strip()


def repository_slug_from_origin() -> tuple[str, str]:
    remote_url = run_git(['remote', 'get-url', 'origin'])
    path_text = remote_url.split(':', 1)[-1] if remote_url.startswith('git@') else remote_url.split('://', 1)[-1]
    pieces = [piece for piece in path_text.replace(':', '/').split('/') if piece]
    if len(pieces) < 3:
        raise usage_error(f'从 origin 地址 {remote_url} 解析不出 owner/repo，请用 --owner / --repo 明确给出')
    return pieces[-2], pieces[-1][:-len('.git')] if pieces[-1].endswith('.git') else pieces[-1]


def read_access_token(token_file: Path | None) -> str:
    from_environment = os.environ.get(TOKEN_ENVIRONMENT_VARIABLE, '').strip()
    if from_environment:
        return from_environment
    if token_file is not None and token_file.is_file():
        return token_file.read_text(encoding='utf-8').strip()
    raise usage_error(
        f'没拿到 Gitee 令牌：环境变量 {TOKEN_ENVIRONMENT_VARIABLE} 为空'
        + (f'，且 {token_file} 不存在' if token_file is not None else '')
        + '（可在 Gitee「设置 → 私人令牌」生成，勾 projects 权限；别写进命令行或仓库）'
    )


def release_tag_of(version: str) -> str:
    return f'v{version}'


def read_release_note(tag: str) -> str:
    return run_git(['tag', '-l', '--format=%(contents)', tag])


def read_release_body(body_file: Path | None, tag: str) -> str:
    """发行版说明：`body_file` 给了就用它（UTF-8 文本），否则取 `tag` 的注解原文。"""
    if body_file is None:
        return read_release_note(tag)
    if not body_file.is_file():
        raise usage_error(f'--body-file 指的 {body_file} 不存在')
    return body_file.read_text(encoding='utf-8').strip()


def ensure_tag_on_remote(tag: str) -> None:
    remote_tags = run_git(['ls-remote', '--tags', 'origin'])
    if f'refs/tags/{tag}' not in remote_tags:
        raise usage_error(f'远端还没有 tag {tag}——先 `git push origin {tag}` 再发版（发行版要挂在这个 tag 上）')


def collect_package_files(version: str) -> list[Path]:
    package_files = sorted(path for path in package_release.DEFAULT_STAGE_ROOT.glob(f'Spark-{version}-*') if path.is_file())
    if not package_files:
        raise usage_error(
            f'在 {package_release.DEFAULT_STAGE_ROOT} 里没找到 Spark-{version}-* 的发布包'
            f'——先跑 `python tools/package_release.py` 出包'
        )
    return package_files


def redact_token(text: str, token: str) -> str:
    return text.replace(token, REDACTED_TOKEN_TEXT) if token else text


def post_json(path: str, payload: dict[str, object], token: str) -> dict[str, object]:
    request = urllib.request.Request(
        GITEE_API_ROOT + path,
        data=json.dumps(payload).encode('utf-8'),
        headers={'Content-Type': 'application/json;charset=UTF-8'},
        method='POST',
    )
    return read_json_response(request, token)


def encode_multipart(file_field_name: str, file_path: Path, fields: list[tuple[str, str]]) -> tuple[bytes, str]:
    boundary = f'----SparkReleaseBoundary{uuid.uuid4().hex}'
    chunks: list[bytes] = []
    for name, value in fields:
        chunks.append(f'--{boundary}\r\nContent-Disposition: form-data; name="{name}"\r\n\r\n{value}\r\n'.encode('utf-8'))
    chunks.append(
        f'--{boundary}\r\nContent-Disposition: form-data; name="{file_field_name}"; '
        f'filename="{file_path.name}"\r\nContent-Type: application/octet-stream\r\n\r\n'.encode('utf-8')
    )
    chunks.append(file_path.read_bytes())
    chunks.append(f'\r\n--{boundary}--\r\n'.encode('utf-8'))
    return b''.join(chunks), boundary


def post_multipart(path: str, file_path: Path, fields: list[tuple[str, str]], token: str) -> dict[str, object]:
    payload, boundary = encode_multipart('file', file_path, fields)
    request = urllib.request.Request(
        GITEE_API_ROOT + path,
        data=payload,
        headers={'Content-Type': f'multipart/form-data; boundary={boundary}'},
        method='POST',
    )
    return read_json_response(request, token)


def read_json_response(request: urllib.request.Request, token: str) -> dict[str, object]:
    try:
        with urllib.request.urlopen(request, timeout=REQUEST_TIMEOUT_SECONDS) as response:
            return json.loads(response.read().decode('utf-8'))
    except urllib.error.HTTPError as error:
        received = error.read().decode('utf-8', 'replace')
        raise publish_error(f'Gitee 回了 HTTP {error.code}：{redact_token(received, token)}') from error
    except urllib.error.URLError as error:
        raise publish_error(f'连不上 Gitee：{error.reason}') from error


def create_release(owner: str, repo: str, tag: str, name: str, note: str, token: str) -> int:
    created = post_json(
        f'/repos/{owner}/{repo}/releases',
        {'access_token': token, 'tag_name': tag, 'name': name, 'body': note, 'target_commitish': 'master'},
        token,
    )
    release_id = created.get('id')
    if not isinstance(release_id, int):
        raise publish_error(f'建发行版没拿到 id：{created}')
    return release_id


def upload_attachment(owner: str, repo: str, release_id: int, package_file: Path, token: str) -> None:
    post_multipart(
        f'/repos/{owner}/{repo}/releases/{release_id}/attach_files',
        package_file,
        [('access_token', token)],
        token,
    )


def parse_arguments() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description='把 out/package/ 下的发布包挂到 Gitee 发行版上')
    parser.add_argument('--owner', help='仓库所属空间地址，默认从 origin 解析')
    parser.add_argument('--repo', help='仓库路径，默认从 origin 解析')
    parser.add_argument('--tag', help='要挂的 tag，默认按 CMakeLists 的版本推成 v<版本>')
    parser.add_argument('--name', help='发行版标题，默认 `Spark <版本>`')
    parser.add_argument('--body-file', help='发行版说明的 UTF-8 文本文件，默认取 tag 注解原文（tag 注解推上去就改不动了）')
    parser.add_argument('--release-id', type=int, help='已有发行版的 id：跳过建发行版、只补传附件')
    parser.add_argument('--token-file', help=f'只含令牌的文件，默认 {DEFAULT_TOKEN_FILE}（环境变量 {TOKEN_ENVIRONMENT_VARIABLE} 优先）')
    parser.add_argument('--dry-run', action='store_true', help='只打印要发的请求与要传的文件，不发任何请求')
    return parser.parse_args()


def main() -> None:
    arguments = parse_arguments()
    try:
        default_owner, default_repo = repository_slug_from_origin()
        owner = arguments.owner or default_owner
        repo = arguments.repo or default_repo
        version = package_release.read_release_version()
        tag = arguments.tag or release_tag_of(version)
        release_name = arguments.name or f'Spark {version}'
        package_files = collect_package_files(version)
        ensure_tag_on_remote(tag)
        release_body = read_release_body(Path(arguments.body_file) if arguments.body_file else None, tag)

        if arguments.dry_run:
            body_source = arguments.body_file or f'tag {tag} 的注解原文'
            print(f'---- 干跑：仓库 {owner}/{repo}、tag {tag}、发行版名 {release_name}')
            print('     会先建发行版' if arguments.release_id is None else f'     会用已有发行版 id={arguments.release_id}')
            print(f'     说明取自 {body_source}（{len(release_body.encode("utf-8"))} 字节，未打印内容）')
            for package_file in package_files:
                print(f'     会传附件 {package_file.name}（{package_file.stat().st_size / 1024:.1f} KB）')
            print('     令牌未读取；去掉 --dry-run 才真正发请求')
            raise SystemExit(EXIT_OK)

        token = read_access_token(Path(arguments.token_file) if arguments.token_file else DEFAULT_TOKEN_FILE)
        if arguments.release_id is None:
            release_id = create_release(owner, repo, tag, release_name, release_body, token)
            print(f'---- 发行版已建：{owner}/{repo} tag {tag}，id={release_id}')
        else:
            release_id = arguments.release_id
            print(f'---- 往已有发行版 id={release_id} 补传附件')
        for package_file in package_files:
            upload_attachment(owner, repo, release_id, package_file, token)
            print(f'     已上传 {package_file.name}（{package_file.stat().st_size / 1024:.1f} KB）')
    except PublishError as error:
        print(f'发版未成：{error}', file=sys.stderr)
        if '已存在' in str(error) or 'exist' in str(error).lower():
            print('  若该 tag 已有发行版，用 --release-id <id> 只补传附件，不要重复建。', file=sys.stderr)
        raise SystemExit(error.exit_code) from None
    print()
    print(f'---- 完成：https://gitee.com/{owner}/{repo}/releases/tag/{tag}')
    raise SystemExit(EXIT_OK)


if __name__ == '__main__':
    main()

# -*- coding: utf-8 -*-
"""把 `out/package/` 下的发布包挂到 Gitee 与 GitHub 的「发行版」上（只用标准库，不引第三方）。

两个平台各发一份（本仓口径：Gitee 是主库、GitHub 是镜像，发行版两边都要），各走各的两步：

    Gitee   `POST /api/v5/repos/{owner}/{repo}/releases`（令牌在 **JSON 体**里）
            `POST /api/v5/repos/{owner}/{repo}/releases/{id}/attach_files`（multipart，字段名 `file`）
    GitHub  `POST https://api.github.com/repos/{owner}/{repo}/releases`（令牌在 **请求头** `Authorization: Bearer`）
            `POST https://uploads.github.com/repos/{owner}/{repo}/releases/{id}/assets?name=<文件名>`
            （原始字节流，不是 multipart——这是 GitHub 与 Gitee 的主要差异）

`{owner}/{repo}` 从各自的 remote 地址解析：Gitee 看 `origin`、GitHub 看 `github`（HTTPS 与
`git@host:owner/repo` 两种写法都认），不用手填。发行版说明默认取该 tag 的注解原文——tag 说明里已经写明了
包内容与验证结论，两处共用一份文本，免得各写一遍再对不上。**但 tag 一旦推上去，其注解就改不动了**
（改它要 force-push）；注解里若写了「未推送」这类**会过期**的状态，就别拿它当发行版说明——
用 `--body-file` 另给一份（会过期的状态不入 tag 注解，或推 tag 前先把状态句删掉）。

**令牌**：先看环境变量（Gitee `GITEE_TOKEN`、GitHub `GITHUB_TOKEN`），为空再看 `--gitee-token-file` /
`--github-token-file` 指的文件，都没给就看默认的 `../Resource/GiteeToken.txt` / `../Resource/GithubToken.txt`
——与本仓**同级**的 `Resource/` 目录，`D:\Gitee` 下各项目共用一份，不必每个仓各放一个（该目录自己有
`.gitignore` 把这两个文件排除在外）。**令牌可长期复用**——写一次这两个文件，此后发布就只是
`python tools/publish_release.py`。令牌只放进请求体或请求头、**不进 URL**（URL 会进服务端访问日志），
也不打印；出错回显服务端报文前会先把令牌字样抹掉。

**不会重复发版**：同一个 tag 已经发过发行版时平台会报错，脚本据此提示改用 `--release-id <id>`
直接往已有发行版补传附件，而不是再建一个。

**发布是外发动作**：附件一旦上传即公开可下载。故先跑 `--dry-run` 看清要传什么、传到哪。

用法（从仓根运行，`package_release` 与它同目录，靠脚本目录进 `sys.path` 导入）：

    python tools/publish_release.py --dry-run                 # 只打印计划，不发任何请求、不读令牌
    python tools/publish_release.py                           # Gitee 与 GitHub 各建发行版 + 各传附件
    python tools/publish_release.py --host gitee              # 只发 Gitee
    python tools/publish_release.py --host github             # 只发 GitHub
    python tools/publish_release.py --host github --release-id 123456   # 只往已有发行版补传附件
    python tools/publish_release.py --body-file out/release_body.txt    # 说明另给一份

退出码：
    0  各平台的发行版与附件都已就绪（或 `--dry-run` 走完）
    1  某个平台没成（HTTP 非 2xx）；此时另一个平台可能已经发好了，报文里逐平台写明
    2  用法/环境错误（缺令牌、tag 没推到对应远端、包里找不到附件、参数不认识）

**只用标准库**，无第三方依赖。
"""
from __future__ import annotations

import argparse
import json
import os
import subprocess
import sys
import urllib.error
import urllib.parse
import urllib.request
import uuid
from pathlib import Path

import package_release

EXIT_OK = 0
EXIT_PUBLISH_FAILED = 1
EXIT_USAGE = 2

GITEE_HOST_NAME = 'gitee'
GITHUB_HOST_NAME = 'github'
USER_AGENT = 'Spark-tools-publish_release'
REQUEST_TIMEOUT_SECONDS = 120
REDACTED_TOKEN_TEXT = '***'
SHARED_TOKEN_DIRECTORY = package_release.REPO_ROOT.parent / 'Resource'


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


def redact_token(text: str, token: str) -> str:
    return text.replace(token, REDACTED_TOKEN_TEXT) if token else text


def remote_slug(remote_name: str) -> tuple[str, str]:
    """把 remote 地址拆成 (owner, repo)。认三种写法：`https://host/owner/repo.git`、
    `git@host:owner/repo.git`（scp 形式，host 在 `@` 与 `:` 之间）、`ssh://git@host[:port]/owner/repo.git`；
    本机路径之类认不出的写法一律报错，不猜。"""
    try:
        remote_url = run_git(['remote', 'get-url', remote_name])
    except PublishError as error:
        raise usage_error(f'取不到 remote `{remote_name}` 的地址：{error}\n  （该 remote 是否还没加？`git remote -v` 看一眼）') from None
    if '://' in remote_url:
        path_text = remote_url.split('://', 1)[1].split('/', 1)[-1]
    elif '@' in remote_url and ':' in remote_url.split('@', 1)[1]:
        path_text = remote_url.split('@', 1)[1].split(':', 1)[1]
    else:
        path_text = ''
    pieces = [piece for piece in path_text.split('/') if piece]
    if len(pieces) < 2:
        raise usage_error(
            f'从 {remote_name} 的地址 {remote_url} 解析不出 owner/repo（只认 https://host/owner/repo 与 '
            f'git@host:owner/repo 两种写法），请用 --owner / --repo 明确给出'
        )
    return pieces[-2], pieces[-1][:-len('.git')] if pieces[-1].endswith('.git') else pieces[-1]


def post_json(url: str, payload: dict[str, object], headers: dict[str, str], token: str) -> dict[str, object]:
    request = urllib.request.Request(url, data=json.dumps(payload).encode('utf-8'), headers=headers, method='POST')
    return read_json_response(request, token)


def post_bytes(url: str, body: bytes, headers: dict[str, str], token: str) -> dict[str, object]:
    request = urllib.request.Request(url, data=body, headers=headers, method='POST')
    return read_json_response(request, token)


def read_json_response(request: urllib.request.Request, token: str) -> dict[str, object]:
    try:
        with urllib.request.urlopen(request, timeout=REQUEST_TIMEOUT_SECONDS) as response:
            return json.loads(response.read().decode('utf-8'))
    except urllib.error.HTTPError as error:
        received = error.read().decode('utf-8', 'replace')
        raise publish_error(f'平台回了 HTTP {error.code}：{redact_token(received, token)}') from error
    except urllib.error.URLError as error:
        raise publish_error(f'连不上平台：{error.reason}') from error


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


class ReleaseHost:
    """一个代码托管平台的发行版接口：地址、令牌来源、建发行版、传附件。子类只填平台差异。"""

    display_name = ''
    host_name = ''
    remote_name = ''
    api_root = ''
    upload_root = ''
    token_environment_variable = ''
    default_token_file = Path()
    token_page_hint = ''

    def __init__(self, owner: str, repo: str, token_file: Path) -> None:
        self.owner = owner
        self.repo = repo
        self.token_file = token_file

    @property
    def slug(self) -> str:
        return f'{self.owner}/{self.repo}'

    def read_token(self) -> str:
        from_environment = os.environ.get(self.token_environment_variable, '').strip()
        if from_environment:
            return from_environment
        if self.token_file.is_file():
            return self.token_file.read_text(encoding='utf-8').strip()
        raise usage_error(
            f'没拿到 {self.display_name} 令牌：环境变量 {self.token_environment_variable} 为空，'
            f'且 {self.token_file} 不存在（{self.token_page_hint}；别写进命令行或仓库）'
        )

    def request_headers(self, token: str, content_type: str) -> dict[str, str]:
        return {'Content-Type': content_type}

    def create_release(self, tag: str, release_name: str, release_body: str, token: str) -> int:
        raise NotImplementedError

    def upload_asset(self, release_id: int, package_file: Path, token: str) -> None:
        raise NotImplementedError

    def release_page_url(self, tag: str) -> str:
        raise NotImplementedError


class GiteeHost(ReleaseHost):
    display_name = 'Gitee'
    host_name = GITEE_HOST_NAME
    remote_name = 'origin'
    api_root = 'https://gitee.com/api/v5'
    upload_root = 'https://gitee.com/api/v5'
    token_environment_variable = 'GITEE_TOKEN'
    default_token_file = SHARED_TOKEN_DIRECTORY / 'GiteeToken.txt'
    token_page_hint = '在 Gitee「设置 → 私人令牌」生成，令牌类型选仓库级、仓库范围选全部仓库，勾 projects 权限'

    def create_release(self, tag: str, release_name: str, release_body: str, token: str) -> int:
        created = post_json(
            f'{self.api_root}/repos/{self.slug}/releases',
            {
                'access_token': token, 'tag_name': tag, 'name': release_name,
                'body': release_body, 'target_commitish': 'master',
            },
            {'Content-Type': 'application/json;charset=UTF-8'},
            token,
        )
        return require_release_id(created, self.display_name)

    def upload_asset(self, release_id: int, package_file: Path, token: str) -> None:
        payload, boundary = encode_multipart('file', package_file, [('access_token', token)])
        post_bytes(
            f'{self.upload_root}/repos/{self.slug}/releases/{release_id}/attach_files',
            payload,
            {'Content-Type': f'multipart/form-data; boundary={boundary}'},
            token,
        )

    def release_page_url(self, tag: str) -> str:
        return f'https://gitee.com/{self.slug}/releases/tag/{tag}'


class GithubHost(ReleaseHost):
    display_name = 'GitHub'
    host_name = GITHUB_HOST_NAME
    remote_name = 'github'
    api_root = 'https://api.github.com'
    upload_root = 'https://uploads.github.com'
    token_environment_variable = 'GITHUB_TOKEN'
    default_token_file = SHARED_TOKEN_DIRECTORY / 'GithubToken.txt'
    token_page_hint = ('在 GitHub「Settings → Developer settings → Personal access tokens → Fine-grained tokens」'
                       '生成，仓库范围选 All repositories，权限给 Contents: Read and write')

    def request_headers(self, token: str, content_type: str) -> dict[str, str]:
        return {
            'Content-Type': content_type,
            'Authorization': f'Bearer {token}',
            'Accept': 'application/vnd.github+json',
            'X-GitHub-Api-Version': '2022-11-28',
            'User-Agent': USER_AGENT,
        }

    def create_release(self, tag: str, release_name: str, release_body: str, token: str) -> int:
        created = post_json(
            f'{self.api_root}/repos/{self.slug}/releases',
            {
                'tag_name': tag, 'name': release_name, 'body': release_body,
                'target_commitish': 'master', 'draft': False, 'prerelease': False,
            },
            self.request_headers(token, 'application/json;charset=UTF-8'),
            token,
        )
        return require_release_id(created, self.display_name)

    def upload_asset(self, release_id: int, package_file: Path, token: str) -> None:
        asset_name = urllib.parse.quote(package_file.name)
        post_bytes(
            f'{self.upload_root}/repos/{self.slug}/releases/{release_id}/assets?name={asset_name}',
            package_file.read_bytes(),
            self.request_headers(token, 'application/octet-stream'),
            token,
        )

    def release_page_url(self, tag: str) -> str:
        return f'https://github.com/{self.slug}/releases/tag/{tag}'


RELEASE_HOST_CLASSES = {GITEE_HOST_NAME: GiteeHost, GITHUB_HOST_NAME: GithubHost}


def looks_like_existing_release(error_message: str) -> bool:
    """平台报「这个 tag 已经有发行版了」的样子：GitHub 是 422＋`"code":"already_exists"`，
    Gitee 的报文里带「已存在」。**不能拿 `exist` 当判据**——令牌无效的 401 报文里
    有 `Access token does not exist`，那样会把认证失败误报成「已发过版」。"""
    lowered = error_message.lower()
    return '已存在' in error_message or 'already_exists' in lowered or 'already exist' in lowered


def require_release_id(created: dict[str, object], display_name: str) -> int:
    release_id = created.get('id')
    if not isinstance(release_id, int):
        raise publish_error(f'{display_name} 建发行版没拿到 id：{created}')
    return release_id


def read_release_note(tag: str) -> str:
    return run_git(['tag', '-l', '--format=%(contents)', tag])


def read_release_body(body_file: Path | None, tag: str) -> str:
    """发行版说明：`body_file` 给了就用它（UTF-8 文本），否则取 `tag` 的注解原文。"""
    if body_file is None:
        return read_release_note(tag)
    if not body_file.is_file():
        raise usage_error(f'--body-file 指的 {body_file} 不存在')
    return body_file.read_text(encoding='utf-8').strip()


def ensure_tag_on_remote(remote_name: str, tag: str) -> None:
    remote_tags = run_git(['ls-remote', '--tags', remote_name])
    if f'refs/tags/{tag}' not in remote_tags:
        raise usage_error(f'{remote_name} 远端还没有 tag {tag}——先 `git push {remote_name} {tag}` 再发版（发行版要挂在这个 tag 上）')


def collect_package_files(version: str) -> list[Path]:
    package_files = sorted(path for path in package_release.DEFAULT_STAGE_ROOT.glob(f'Spark-{version}-*') if path.is_file())
    if not package_files:
        raise usage_error(
            f'在 {package_release.DEFAULT_STAGE_ROOT} 里没找到 Spark-{version}-* 的发布包'
            f'——先跑 `python tools/package_release.py` 出包'
        )
    return package_files


def build_hosts(host_names: list[str], owner_override: str | None, repo_override: str | None,
                token_file_overrides: dict[str, str | None]) -> list[ReleaseHost]:
    hosts: list[ReleaseHost] = []
    for host_name in host_names:
        host_class = RELEASE_HOST_CLASSES[host_name]
        default_owner, default_repo = remote_slug(host_class.remote_name)
        token_file = Path(token_file_overrides[host_name]) if token_file_overrides[host_name] else host_class.default_token_file
        hosts.append(host_class(owner_override or default_owner, repo_override or default_repo, token_file))
    return hosts


def publish_packages(host: ReleaseHost, tag: str, release_name: str, release_body: str,
                     release_id: int | None, package_files: list[Path], token: str) -> None:
    if release_id is None:
        release_id = host.create_release(tag, release_name, release_body, token)
        print(f'---- {host.display_name} 发行版已建：{host.slug} tag {tag}，id={release_id}')
    else:
        print(f'---- {host.display_name} 往已有发行版 id={release_id} 补传附件')
    for package_file in package_files:
        host.upload_asset(release_id, package_file, token)
        print(f'     {host.display_name} 已上传 {package_file.name}（{package_file.stat().st_size / 1024:.1f} KB）')
    print(f'     {host.display_name} 发行版页面：{host.release_page_url(tag)}')


def print_dry_run(hosts: list[ReleaseHost], tag: str, release_name: str, release_body: str,
                  body_source: str, release_id: int | None, package_files: list[Path]) -> None:
    print(f'---- 干跑：tag {tag}、发行版名 {release_name}、说明取自 {body_source}'
          f'（{len(release_body.encode("utf-8"))} 字节，未打印内容）')
    for host in hosts:
        print(f'     {host.display_name}（remote `{host.remote_name}`）：仓库 {host.slug}、'
              + ('会先建发行版' if release_id is None else f'会用已有发行版 id={release_id}'))
        for package_file in package_files:
            print(f'       会传附件 {package_file.name}（{package_file.stat().st_size / 1024:.1f} KB）')
    print('     令牌未读取、未发任何请求；去掉 --dry-run 才真正发请求')


def parse_arguments() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description='把 out/package/ 下的发布包挂到 Gitee 与 GitHub 的发行版上')
    parser.add_argument('--host', choices=[GITEE_HOST_NAME, GITHUB_HOST_NAME, 'all'], default='all',
                        help='发到哪个平台，默认 all（两边各发一份）')
    parser.add_argument('--owner', help='仓库所属空间地址，默认从对应 remote 解析（多平台时不可用）')
    parser.add_argument('--repo', help='仓库路径，默认从对应 remote 解析（多平台时不可用）')
    parser.add_argument('--tag', help='要挂的 tag，默认按 CMakeLists 的版本推成 v<版本>')
    parser.add_argument('--name', help='发行版标题，默认 `Spark <版本>`')
    parser.add_argument('--body-file', help='发行版说明的 UTF-8 文本文件，默认取 tag 注解原文（tag 注解推上去就改不动了）')
    parser.add_argument('--release-id', type=int, help='已有发行版的 id：跳过建发行版、只补传附件（多平台时须用 --host 指明一个）')
    parser.add_argument('--gitee-token-file', help=f'Gitee 令牌文件，默认 {GiteeHost.default_token_file}')
    parser.add_argument('--github-token-file', help=f'GitHub 令牌文件，默认 {GithubHost.default_token_file}')
    parser.add_argument('--dry-run', action='store_true', help='只打印要发的请求与要传的文件，不发任何请求')
    return parser.parse_args()


def main() -> None:
    arguments = parse_arguments()
    host_names = [GITEE_HOST_NAME, GITHUB_HOST_NAME] if arguments.host == 'all' else [arguments.host]

    try:
        if len(host_names) > 1 and (arguments.owner or arguments.repo):
            raise usage_error(f'--owner/--repo 只在单平台时可用（--host {GITEE_HOST_NAME} 或 --host {GITHUB_HOST_NAME}）'
                              f'——两个平台的 owner/repo 各自从 remote 解析')
        if len(host_names) > 1 and arguments.release_id is not None:
            raise usage_error(f'--release-id 只在单平台时可用（--host {GITEE_HOST_NAME} 或 --host {GITHUB_HOST_NAME}）'
                              f'——两个平台的发行版 id 不是同一个')
        version = package_release.read_release_version()
        tag = arguments.tag or f'v{version}'
        release_name = arguments.name or f'Spark {version}'
        package_files = collect_package_files(version)
        release_body = read_release_body(Path(arguments.body_file) if arguments.body_file else None, tag)
        body_source = arguments.body_file or f'tag {tag} 的注解原文'

        hosts = build_hosts(
            host_names,
            arguments.owner,
            arguments.repo,
            {GITEE_HOST_NAME: arguments.gitee_token_file, GITHUB_HOST_NAME: arguments.github_token_file},
        )
        for host in hosts:
            ensure_tag_on_remote(host.remote_name, tag)

        if arguments.dry_run:
            print_dry_run(hosts, tag, release_name, release_body, body_source, arguments.release_id, package_files)
            raise SystemExit(EXIT_OK)

        tokens = {host.display_name: host.read_token() for host in hosts}
        failures: list[str] = []
        for host in hosts:
            try:
                publish_packages(host, tag, release_name, release_body, arguments.release_id, package_files,
                                 tokens[host.display_name])
            except PublishError as error:
                print(f'{host.display_name} 未成：{error}', file=sys.stderr)
                if looks_like_existing_release(str(error)):
                    print(f'  若该 tag 已有发行版，用 `--host {host.host_name}` 配合 --release-id <id> 只补传附件，'
                          f'不要重复建。', file=sys.stderr)
                failures.append(host.display_name)
        if failures:
            remaining = [host.display_name for host in hosts if host.display_name not in failures]
            if remaining:
                print(f'（{"、".join(remaining)} 那份已发好，只有 {"、".join(failures)} 未成）', file=sys.stderr)
            raise SystemExit(EXIT_PUBLISH_FAILED)
    except PublishError as error:
        print(f'发版未成：{error}', file=sys.stderr)
        raise SystemExit(error.exit_code) from None
    print()
    print(f'---- 完成：{tag} 在 {"、".join(host.display_name for host in hosts)} 各发了一份')


if __name__ == '__main__':
    main()

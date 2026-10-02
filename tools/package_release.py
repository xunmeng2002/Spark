# -*- coding: utf-8 -*-
"""发布打包：只把 Release 档装进暂存目录，再按平台压成发布包。

**为什么不能直接压已安装目录**（`../Libs/Spark` / `~/.vs/Libs/Spark`）：那一份是 Debug 与
Release 两次 `cmake --install` 叠出来的，产物同目录并排——`bin/Core.dll` 旁边就是
`bin/Cored.dll`、`lib/Core.lib` 旁边就是 `lib/Cored.lib`，另有 `Cored.pdb` 等三个
Debug 符号文件。按实测，Windows 侧整个安装树 70.10 MB，其中那三个 pdb 占 68 MB；
只留 Release 则是 1.44 MB（Linux 侧 9.4 MB → 约 1.43 MB）。手挑文件名区分
`Core.lib` 与 `Cored.lib` 是常态性出错源，故换个做法。

**做法**：让 CMake 从 Release 的**构建目录**重新 `--install` 到一个干净前缀，拷什么完全由
`install()` 规则决定，天然只含那一档；Release 的 pdb 本来就不在安装规则里
（`submodules/CMakeCommon/CMakeCommon.cmake` 那行写了 `CONFIGURATIONS Debug RelWithDebInfo`）。
装完立即自检：暂存目录里不得出现 Debug 后缀的二进制（`Cored.dll` / `libCored.so` 之类），
也不得出现 `SparkTargets-debug.cmake`——出现即报错退出。宁可不给包，也不给一个混着 Debug 的包。

**包内容与 `SparkTargets.cmake` 的关系**：导出文件按配置分文件，主文件用
`file(GLOB ...)` 把每档都收进来。Release-only 的包因此只剩 `SparkTargets-release.cmake`，
这是预期结果，但**包只应被 Release 配置的消费方使用**：MSVC 下 Debug 工程若硬拿它，
会链到 `/MD` 的 Release 导入库而自己用 `/MDd`，运行库混用会踩堆损坏。

**本脚本不删任何东西**：暂存目录已存在就报错并打印路径，由人决定怎么处理
（`out/` 是 git 之外的构建产物，不做递归删除）。

**Linux 侧的两个前缀是同一份东西的两种来历**：VS 的 WSL 模式先把源码拷到 `~/.vs/Spark`，
preset 又把前缀写成 `${sourceDir}/../Libs/Spark/x64-linux`，于是落到 `~/.vs/Libs/Spark`；
在 `/mnt/d/Gitee/Spark` 下手工跑 `Install.sh` 则装到 `/mnt/d/Gitee/Libs/Spark/x64-linux`。
本脚本不依赖 preset，**每次都显式给 `--prefix`**。

用法：
    python tools/package_release.py                     # 两个平台都出包
    python tools/package_release.py windows             # 只出 Windows 包
    python tools/package_release.py linux
    python tools/package_release.py --stage-root out/package2

产物（都在 git 之外的 `out/` 下，脚本只报路径，不上传、不移动）：
    out/package/Spark-<版本>-x64-windows.zip     包根为 `x64-windows/`，解开即得
    out/package/Spark-<版本>-x64-linux.tar.gz    包根为 `x64-linux/`，解开即得
版本号取自 `CMakeLists.txt` 的 `project(... VERSION x.y.z)`，与 tag 名对齐。

退出码：
    0  指定平台全部出包成功
    1  自检未过（暂存目录里出现 Debug 产物或 Debug 导出文件、缺 Release 导出文件）
    2  用法/环境错误（平台名不认识、构建目录不存在、暂存目录已存在、cmake 或 wsl 不可用、安装失败）

**只用标准库**，无第三方依赖。
"""
from __future__ import annotations

import argparse
import os
import re
import shutil
import subprocess
import sys
import tarfile
import zipfile
from pathlib import Path

EXIT_OK = 0
EXIT_SELF_CHECK_FAILED = 1
EXIT_USAGE = 2

REPO_ROOT = Path(__file__).resolve().parent.parent
CMAKE_LISTS_PATH = REPO_ROOT / 'CMakeLists.txt'
DEFAULT_STAGE_ROOT = REPO_ROOT / 'out' / 'package'

WINDOWS_PLATFORM = 'windows'
LINUX_PLATFORM = 'linux'
ALL_PLATFORMS = 'all'
PLATFORM_DIRECTORY_NAMES = {
    WINDOWS_PLATFORM: 'x64-windows',
    LINUX_PLATFORM: 'x64-linux',
}
RELEASE_BUILD_DIRECTORIES = {
    WINDOWS_PLATFORM: REPO_ROOT / 'out' / 'build' / 'x64-Release',
    LINUX_PLATFORM: REPO_ROOT / 'out' / 'build' / 'WSL-GCC-Release',
}
ARCHIVE_SUFFIXES = {
    WINDOWS_PLATFORM: '.zip',
    LINUX_PLATFORM: '.tar.gz',
}
BINARY_SUFFIXES = frozenset({'.dll', '.lib', '.pdb', '.so'})
DEBUG_POSTFIX = 'd'
DEBUG_EXPORT_FILE_NAME = 'SparkTargets-debug.cmake'
RELEASE_EXPORT_FILE_NAME = 'SparkTargets-release.cmake'


class PackagingError(Exception):
    """打包未成。`exit_code` 决定进程退出码：1＝自检未过，2＝用法/环境错误。"""

    def __init__(self, message: str, exit_code: int) -> None:
        super().__init__(message)
        self.exit_code = exit_code


def usage_error(message: str) -> PackagingError:
    return PackagingError(message, EXIT_USAGE)


def self_check_error(message: str) -> PackagingError:
    return PackagingError(message, EXIT_SELF_CHECK_FAILED)


def read_release_version() -> str:
    project_match = re.search(r'project\([^)]*?VERSION\s+(\d+\.\d+\.\d+)', CMAKE_LISTS_PATH.read_text(encoding='utf-8'))
    if project_match is None:
        raise usage_error(f'在 {CMAKE_LISTS_PATH} 里没找到 `project(... VERSION x.y.z)`，无法定包名')
    return project_match.group(1)


def to_wsl_path(windows_path: Path) -> str:
    posix_text = windows_path.as_posix()
    if len(posix_text) < 2 or posix_text[1] != ':':
        raise usage_error(f'{windows_path} 不是「盘符:\\…」形式，无法折算成 WSL 路径')
    return f'/mnt/{posix_text[0].lower()}{posix_text[2:]}'


def stage_directory_of(stage_root: Path, version: str, platform: str) -> Path:
    return stage_root / f'Spark-{version}' / PLATFORM_DIRECTORY_NAMES[platform]


def archive_path_of(stage_root: Path, version: str, platform: str) -> Path:
    return stage_root / f'Spark-{version}-{PLATFORM_DIRECTORY_NAMES[platform]}{ARCHIVE_SUFFIXES[platform]}'


def ensure_installable(platform: str, stage_dir: Path) -> Path:
    build_dir = RELEASE_BUILD_DIRECTORIES[platform]
    if not build_dir.is_dir():
        raise usage_error(f'Release 构建目录不存在：{build_dir}（先构出这一档再来打包）')
    if stage_dir.exists():
        raise usage_error(
            f'暂存目录已存在：{stage_dir}\n'
            '  本脚本不删任何东西（out/ 是 git 之外的构建产物）——请你确认后自行删掉它，'
            '或用 --stage-root 换一个落点。'
        )
    return build_dir


def run_install(platform: str, build_dir: Path, stage_dir: Path) -> None:
    if platform == WINDOWS_PLATFORM:
        command = ['cmake', '--install', str(build_dir), '--prefix', str(stage_dir)]
        if shutil.which('cmake') is None:
            raise usage_error('PATH 里没有 cmake，无法安装（Windows 侧可先跑 out\\build_msvc.bat 那套环境）')
    else:
        if shutil.which('wsl') is None:
            raise usage_error('PATH 里没有 wsl，无法安装 Linux 档')
        command = [
            'wsl', '-e', 'bash', '-lc',
            f'cmake --install {to_wsl_path(build_dir)} --prefix {to_wsl_path(stage_dir)}',
        ]
    environment = {**os.environ, 'MSYS_NO_PATHCONV': '1'}
    try:
        completed = subprocess.run(command, capture_output=True, text=True, env=environment)
    except OSError as error:
        raise usage_error(f'执行安装命令失败：{error}') from error
    if completed.returncode != 0:
        raise usage_error(
            f'安装失败（退出码 {completed.returncode}）：{" ".join(command)}\n'
            f'  stdout: {completed.stdout.strip()}\n'
            f'  stderr: {completed.stderr.strip()}'
        )


def collect_debug_artifacts(stage_dir: Path) -> list[Path]:
    found: list[Path] = []
    for path in sorted(stage_dir.rglob('*')):
        if not path.is_file():
            continue
        if path.name == DEBUG_EXPORT_FILE_NAME:
            found.append(path)
            continue
        if path.suffix in BINARY_SUFFIXES and path.stem.endswith(DEBUG_POSTFIX):
            found.append(path)
    return found


def verify_release_only(stage_dir: Path) -> None:
    debug_artifacts = collect_debug_artifacts(stage_dir)
    if debug_artifacts:
        listing = '\n'.join(f'  {path.relative_to(stage_dir)}' for path in debug_artifacts)
        raise self_check_error(f'暂存目录里混进了 Debug 产物，已放弃出包：\n{listing}')
    export_files = [path.name for path in stage_dir.rglob('SparkTargets-*.cmake')]
    if RELEASE_EXPORT_FILE_NAME not in export_files:
        raise self_check_error(
            f'暂存目录里没有 {RELEASE_EXPORT_FILE_NAME}（只找到 {export_files}），'
            '说明这一档不是从 Release 构建目录装出来的，已放弃出包'
        )


def summarize(stage_dir: Path) -> tuple[int, int]:
    files = [path for path in stage_dir.rglob('*') if path.is_file()]
    return len(files), sum(path.stat().st_size for path in files)


def write_zip_archive(stage_dir: Path, archive_path: Path) -> None:
    with zipfile.ZipFile(archive_path, 'w', zipfile.ZIP_DEFLATED) as archive:
        for path in sorted(stage_dir.rglob('*')):
            if path.is_file():
                archive.write(path, path.relative_to(stage_dir.parent).as_posix())


def write_tar_archive(stage_dir: Path, archive_path: Path) -> None:
    with tarfile.open(archive_path, 'w:gz') as archive:
        for path in sorted(stage_dir.rglob('*')):
            if path.is_file():
                archive.add(path, arcname=path.relative_to(stage_dir.parent).as_posix())


def pack_platform(stage_root: Path, version: str, platform: str) -> None:
    stage_dir = stage_directory_of(stage_root, version, platform)
    build_dir = ensure_installable(platform, stage_dir)
    stage_dir.parent.mkdir(parents=True, exist_ok=True)
    print(f'---- {PLATFORM_DIRECTORY_NAMES[platform]}：{build_dir} -> {stage_dir}')
    run_install(platform, build_dir, stage_dir)
    verify_release_only(stage_dir)
    file_count, total_bytes = summarize(stage_dir)
    archive_path = archive_path_of(stage_root, version, platform)
    if platform == WINDOWS_PLATFORM:
        write_zip_archive(stage_dir, archive_path)
    else:
        write_tar_archive(stage_dir, archive_path)
    print(f'     文件 {file_count} 个、未压缩 {total_bytes / 1048576:.2f} MB（自检：无 Debug 产物）')
    print(f'     发布包 {archive_path}（{archive_path.stat().st_size / 1048576:.2f} MB）')


def parse_arguments() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description='只把 Release 档装进暂存目录并压成发布包（不用已安装目录，避免带上 Debug 产物）')
    parser.add_argument('platform', nargs='?', choices=[WINDOWS_PLATFORM, LINUX_PLATFORM, ALL_PLATFORMS],
                        default=ALL_PLATFORMS, help='出哪个平台的包，默认两个都出')
    parser.add_argument('--stage-root', default=str(DEFAULT_STAGE_ROOT),
                        help=f'暂存与产物的落点，默认 {DEFAULT_STAGE_ROOT}')
    return parser.parse_args()


def main() -> None:
    arguments = parse_arguments()
    try:
        version = read_release_version()
        stage_root = Path(arguments.stage_root).resolve()
        platforms = [WINDOWS_PLATFORM, LINUX_PLATFORM] if arguments.platform == ALL_PLATFORMS else [arguments.platform]
        for platform in platforms:
            pack_platform(stage_root, version, platform)
    except PackagingError as error:
        print(f'出包未成：{error}', file=sys.stderr)
        raise SystemExit(error.exit_code) from None
    print()
    print(f'---- 出包完成（版本 {version}）。包内只有 Release 档，且只应被 Release 配置的消费方使用：')
    print('     MSVC 下 Debug 工程若硬拿这个包，会链到 /MD 的 Release 导入库而自己用 /MDd，运行库混用会踩堆损坏。')
    print('     脚本只把包放在上面的路径里，没上传、没移动、没删任何东西。')
    raise SystemExit(EXIT_OK)


if __name__ == '__main__':
    main()

# -*- coding: utf-8 -*-
"""STEP 端到端冒烟：让 `TestServer` / `TestClient` 走真正的 STEP 帧收发，验证**读路径**。

为什么需要它：`TestServer` + `TestClient` 的默认协议（`TestProtocol` 默认 `Tcp`）走的是
`ServerIoSubscriberImpl::OnRecv` 的**原样回显**——两端都不解析，`Package::FromStepStream`
一次都没被调用。真正"发出 → 成帧 → 解析 → 断言字段"的路径只存在于
`TestStepClient` / `TestStepServer`（`ProtocolTypeType::Step`），而它们此前只能靠手改
`TestUtility.cpp` 里的 `TestProtocol` 初值来选中。故本脚本配合 `TestClient.exe Step` /
`TestServer.exe Step` 这一个命令行参数（见 `ApplyTestProtocolFromCommandLine`）一起使用。

断言依据（来自 `StepClient::SendReqInsertOrder`）：
    Price = 100 + index, Volume = index, ClientOrderId = index
且 `AccountId` / `ExchangeId` / `InstrumentId` 为固定串、三个枚举字段恒为 0。
服务端每 1000 条记一行、客户端每 10000 条记一行，故只要跑够秒数就必有样本；
样本整齐递增 1000 也顺带证明流没有错位或重连。

两处需要知道的实现取舍：

- **就绪判定靠日志而非固定等待**：`listen()` 成功没有对应的日志行，可轮询的唯一证据是
  `CreateIo ServerType:Server`。故先轮询到该行（证明进程起来了），再留一个短常量等
  bind+listen。留短常量而不做端口探测，是因为探测连接会在服务端留下一个真实会话，
  可能反过来污染"日志里不得出现 ERROR"这条断言。
- **样本不足即响亮失败**：间隔断言至少要 2 帧才有意义（客户端每 10000 条才记一行，
  跑太短就只剩 1 帧）。此时不做间隔校验会让"条数与字段全部吻合"这句话**名不副实**，
  故按帧数不足直接判失败，而不是照常给绿灯。

`--io-model` 选后端（见 `ApplyIoModelFromCommandLine`，即命令行第二个参数）。默认
`Select`；`Iocp` 走 Windows 完成端口那套收发（`TcpIocp*`），其 `OnRecv` 回调拿到的是
每连接常驻的缓冲区视图而非池借出物——两条读路径的缓冲所有权契约不同，故**两种模型都要各跑
一轮**，只跑一种会漏掉另一套实现的回归。`Epoll` 按 `#ifdef __linux__` 编译，Windows 构建下
`IoFactory` 会静默回退到 `Select`，于是这一轮就成了 `Select` 的重复跑（结论标注成 Epoll 却是
Select 的成绩），故在此直接拒绝。

用法：
    python tools/step_e2e.py                          # 跑 40 秒，Debug 配置，Select 后端
    python tools/step_e2e.py --seconds 60
    python tools/step_e2e.py --config Release
    python tools/step_e2e.py --io-model Iocp

`--seconds` 有上限：服务端连上后 90 秒自停（`TestStepServer` 的 `sleep_for(90s)`），
跑过这个窗口的后果是**后段成了死时间**——帧仍然存在、序号仍然等间隔，断言看不出
「末段没在跑」，于是可能假绿。故这里直接挡在 80 秒。

退出码：
    0  断言全过
    1  断言未过（日志里有 ERROR、字段不符、样本不足、或压根没读到帧）
    2  用法/环境错误（缺可执行文件、服务端未就绪、本次没落下日志文件、参数越界）

**只用标准库**，无第三方依赖。
"""
import os
import re
import subprocess
import sys
import time
from typing import Optional

REPO_ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
LOG_DIR = os.path.join(REPO_ROOT, 'log')
# 默认值要给客户端那一侧留足余量：客户端每 10000 条才记一行，机器慢一倍时
# 20 秒只够凑出 2 帧，第 2 帧还可能被 terminate 连缓冲区一起丢掉
DEFAULT_SECONDS = 40
MAX_SECONDS = 80
DEFAULT_IO_MODEL = 'Select'
# TestClient/TestServer 接受的名字（与 GetIoModelString 同源）。Epoll 按 #ifdef __linux__ 编译，
# Windows 构建下 IoFactory 静默回退 Select，跑出来的是 Select 的成绩却会挂 Epoll 的名，故挡在用法层
PASSABLE_IO_MODELS = ('Select', 'Iocp')
READY_TIMEOUT_SECONDS = 15
READY_POLL_INTERVAL_SECONDS = 0.2
# listen() 成功无日志可轮询，进程就绪后再留一段等 bind+listen 完成
BIND_LISTEN_GRACE_SECONDS = 1
TERMINATE_GRACE_SECONDS = 10
MIN_FRAMES_PER_SIDE = 2
SERVER_READY_MARKER = 'CreateIo ServerType:Server'

EXIT_PASS = 0
EXIT_FAILURE = 1
EXIT_USAGE = 2

# 服务端 `StepServer::OnMessage` 与客户端 `StepClient::OnMessage` 打印的是同一种调试串，
# 故一条正则两端通用（客户端前缀是 "OnMessage: "，服务端是 "OnMessage SessionId:[..], "）
FRAME_PATTERN = re.compile(
    r'ReqInsertOrder:AccountId:\[([^\]]*)\], ExchangeId:\[([^\]]*)\], '
    r'InstrumentId:\[([^\]]*)\], Direction:\[(-?\d+)\], OffsetFlag:\[(-?\d+)\], '
    r'OrderPriceType:\[(-?\d+)\], Price:\[(-?[\d.]+)\], Volume:\[(-?\d+)\], '
    r'ClientOrderId:\[(-?\d+)\]')

# 读路径出问题时会打出来的字句（`src/Network/` 下的 WriteLog 原文），逐条断言其不出现
PROTOCOL_FAILURES = (
    'Garbage Stream Detected',
    'CheckSum not Match',
    'Protocol Version Not Match',
    'Inbound Package Not Accepted',
    'FieldId not Match',
    'Step Body Length Invalid',
    'Step Buffer Too Small',
    'Package Buffer Too Small',
    'Cannot Find PackageReader',
)

EXPECTED_ACCOUNT_ID = 'Xunmeng001'
EXPECTED_EXCHANGE_ID = 'SHSE'
EXPECTED_INSTRUMENT_ID = '600036'
EXPECTED_ENUM_ORDINALS = ('0', '0', '0')
SERVER_LOG_INTERVAL = 1000
CLIENT_LOG_INTERVAL = 10000


def parse_options(argv: list[str]) -> tuple[int, str, str]:
    seconds = DEFAULT_SECONDS
    config = 'Debug'
    io_model = DEFAULT_IO_MODEL
    index = 0
    while index < len(argv):
        argument = argv[index]
        if argument in ('-h', '--help'):
            print(__doc__)
            raise SystemExit(EXIT_PASS)
        if argument not in ('--seconds', '--config', '--io-model'):
            print(f'---- 无法识别的参数: {argument}（可用 --seconds N / --config Debug|Release / --io-model {"|".join(PASSABLE_IO_MODELS)} / -h）',
                  file=sys.stderr)
            raise SystemExit(EXIT_USAGE)
        if index + 1 >= len(argv):
            print(f'---- {argument} 缺少取值', file=sys.stderr)
            raise SystemExit(EXIT_USAGE)
        value = argv[index + 1]
        if argument == '--seconds':
            try:
                seconds = int(value)
            except ValueError:
                print(f'---- --seconds 需为正整数，收到 "{value}"', file=sys.stderr)
                raise SystemExit(EXIT_USAGE)
            if seconds <= 0 or seconds > MAX_SECONDS:
                print(f'---- --seconds 需为 1~{MAX_SECONDS} 的正整数，收到 "{value}"（上限见模块文档：服务端 90 秒自停）',
                      file=sys.stderr)
                raise SystemExit(EXIT_USAGE)
        elif argument == '--config':
            config = value
        else:
            if value not in PASSABLE_IO_MODELS:
                print(f'---- --io-model 需为 {"|".join(PASSABLE_IO_MODELS)}，收到 "{value}"'
                      f'（Epoll 按 #ifdef __linux__ 编译，Windows 下会静默回退 Select，故不接受）', file=sys.stderr)
                raise SystemExit(EXIT_USAGE)
            io_model = value
        index += 2
    return seconds, config, io_model


def snapshot_log_names() -> set[str]:
    if not os.path.isdir(LOG_DIR):
        return set()
    return set(os.listdir(LOG_DIR))


def read_log_text(name: str) -> str:
    with open(os.path.join(LOG_DIR, name), encoding='utf-8', errors='replace') as log_file:
        return log_file.read()


def new_log_names(names_before: set[str], process_name: str) -> list[str]:
    return sorted(name for name in snapshot_log_names() - names_before
                  if name.startswith(process_name) and name.endswith('.log'))


def start_test_process(executable_path: str, protocol_name: str, io_model: str) -> subprocess.Popen:
    # stdout 一律 DEVNULL：给 PIPE 而不读，日志线程写满管道后会阻塞，
    # 表现成"跑十几条就不动了"的假卡死（2026-09-18 已用同型实验复现过）
    return subprocess.Popen([executable_path, protocol_name, io_model], cwd=REPO_ROOT,
                            stdout=subprocess.DEVNULL, stderr=subprocess.STDOUT)


def terminate_process(process: Optional[subprocess.Popen]) -> None:
    if process is None:
        return
    if process.poll() is None:
        process.terminate()
    try:
        process.wait(timeout=TERMINATE_GRACE_SECONDS)
    except subprocess.TimeoutExpired:
        process.kill()
        process.wait(timeout=TERMINATE_GRACE_SECONDS)


def wait_for_server_ready(names_before: set[str], timeout_seconds: float) -> bool:
    deadline = time.monotonic() + timeout_seconds
    while time.monotonic() < deadline:
        for name in new_log_names(names_before, 'TestServer'):
            if SERVER_READY_MARKER in read_log_text(name):
                return True
        time.sleep(READY_POLL_INTERVAL_SECONDS)
    return False


def check_log_health(log_name: str, text: str, failures: list[str]) -> None:
    error_lines = [line for line in text.split('\n') if ' ERROR ' in line]
    for line in error_lines:
        print(f'  [ERROR 行] {log_name}: {line.strip()[:200]}')
    for marker in PROTOCOL_FAILURES:
        if marker in text:
            print(f'  [协议失败] {log_name}: 出现 {marker}')
            failures.append(f'{log_name} 出现协议失败字句 {marker}')
    if error_lines:
        failures.append(f'{log_name} 有 {len(error_lines)} 条 ERROR')
    for line in (line for line in text.split('\n') if ' WARNING ' in line):
        print(f'  [WARNING 行（不计失败）] {log_name}: {line.strip()[:200]}')


def check_io_model(role: str, log_name: str, text: str, io_model: str, failures: list[str]) -> None:
    """`CreateIo` 打的是**请求**的模型，故本断言只证明参数抵达了工厂，不证明工厂选中了哪个后端类。"""
    if f'IoModel:{io_model}' not in text:
        failures.append(f'{role}（{log_name}）日志里没有 IoModel:{io_model}——命令行第二个参数没传到进程，'
                        f'本轮并非所声明的后端')


def check_frames(role: str, log_name: str, text: str, log_interval: int, failures: list[str]) -> None:
    """按 `Price == 100 + Volume == 100 + ClientOrderId` 校验每一帧，并查等间隔递增。"""
    frames = FRAME_PATTERN.findall(text)
    print(f'  {role}（{log_name}）解析到 {len(frames)} 帧')
    if not frames:
        failures.append(f'{role} 没读到任何 STEP 帧')
        return
    volumes = []
    for account_id, exchange_id, instrument_id, direction, offset_flag, price_type, price, volume, client_order_id in frames:
        volume = int(volume)
        client_order_id = int(client_order_id)
        price = float(price)
        volumes.append(volume)
        if (account_id, exchange_id, instrument_id) != (EXPECTED_ACCOUNT_ID, EXPECTED_EXCHANGE_ID, EXPECTED_INSTRUMENT_ID):
            failures.append(f'{role} 字符串字段不符: [{account_id}][{exchange_id}][{instrument_id}]')
        if (direction, offset_flag, price_type) != EXPECTED_ENUM_ORDINALS:
            failures.append(f'{role} 枚举字段不符: [{direction}][{offset_flag}][{price_type}]')
        if client_order_id != volume:
            failures.append(f'{role} Volume[{volume}] != ClientOrderId[{client_order_id}]')
        if abs(price - (100 + volume)) > 1e-6:
            failures.append(f'{role} Price[{price:.6f}] != 100 + Volume[{volume}]')

    # 客户端跑满 1,000,000 条时 `StepClient::OnMessage` 会把收尾那一帧连记两遍，
    # 折掉相邻重复再算间隔，免得把收尾行为误诊成流错位
    distinct_volumes = [volume for index, volume in enumerate(volumes) if index == 0 or volume != volumes[index - 1]]
    if len(distinct_volumes) < MIN_FRAMES_PER_SIDE:
        failures.append(f'{role} 样本仅 {len(distinct_volumes)} 帧，不足以证明流未错位（需 ≥{MIN_FRAMES_PER_SIDE}）')
        return
    steps = {later - earlier for earlier, later in zip(distinct_volumes, distinct_volumes[1:])}
    if steps != {log_interval}:
        failures.append(f'{role} 帧序号间隔异常（期望恒为 {log_interval}）: {sorted(steps)}')


def main() -> None:
    seconds, config, io_model = parse_options(sys.argv[1:])
    executable_dir = os.path.join(REPO_ROOT, 'bin', config)
    server_exe = os.path.join(executable_dir, 'TestServer.exe')
    client_exe = os.path.join(executable_dir, 'TestClient.exe')
    for path in (server_exe, client_exe):
        if not os.path.isfile(path):
            print(f'---- 缺可执行文件: {path}（先构建 TestServer/TestClient）', file=sys.stderr)
            raise SystemExit(EXIT_USAGE)

    names_before = snapshot_log_names()
    print(f'---- STEP 端到端冒烟：{config} 配置，IO 模型 {io_model}，跑 {seconds} 秒')
    server_process = None
    client_process = None
    try:
        server_process = start_test_process(server_exe, 'Step', io_model)
        if not wait_for_server_ready(names_before, READY_TIMEOUT_SECONDS):
            print(f'---- 服务端 {READY_TIMEOUT_SECONDS} 秒内未就绪（日志里没出现 "{SERVER_READY_MARKER}"）', file=sys.stderr)
            raise SystemExit(EXIT_USAGE)
        time.sleep(BIND_LISTEN_GRACE_SECONDS)
        client_process = start_test_process(client_exe, 'Step', io_model)
        time.sleep(seconds)
    finally:
        terminate_process(client_process)
        terminate_process(server_process)

    server_logs = new_log_names(names_before, 'TestServer')
    client_logs = new_log_names(names_before, 'TestClient')
    if not server_logs or not client_logs:
        print(f'---- 没拿到本次运行的日志文件（{LOG_DIR}）——日志没落盘，结论不可信', file=sys.stderr)
        raise SystemExit(EXIT_USAGE)

    failures: list[str] = []
    for log_name in server_logs + client_logs:
        check_log_health(log_name, read_log_text(log_name), failures)
    server_text = '\n'.join(read_log_text(name) for name in server_logs)
    client_text = '\n'.join(read_log_text(name) for name in client_logs)
    if 'StepServer::OnConnect' not in server_text:
        failures.append('服务端没记录到客户端连接')
    if 'StepClient::OnConnect' not in client_text:
        failures.append('客户端没连上服务端（可能是启动竞态而非解析回归，先看两侧日志的时间戳）')
    check_io_model('服务端', server_logs[0], server_text, io_model, failures)
    check_io_model('客户端', client_logs[0], client_text, io_model, failures)
    check_frames('服务端读路径', server_logs[0], server_text, SERVER_LOG_INTERVAL, failures)
    check_frames('客户端读路径', client_logs[0], client_text, CLIENT_LOG_INTERVAL, failures)

    if failures:
        print(f'---- 未通过 {len(failures)} 项：')
        for failure in failures[:20]:
            print(f'  * {failure}')
        raise SystemExit(EXIT_FAILURE)
    print(f'---- 通过（IO 模型 {io_model}）：两端均解析成功，条数与字段全部吻合')


if __name__ == '__main__':
    main()

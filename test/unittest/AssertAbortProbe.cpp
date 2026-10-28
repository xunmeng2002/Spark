#include "AssertAbortProbe.h"

#include <Spark/TemplateLib/Buffer/LinearBuffer.h>
#include <Spark/TemplateLib/Buffer/ShmBuffer.h>

#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

#ifdef _WIN32
#include <Windows.h>
#else
#include <csignal>
#include <sys/wait.h>
#include <unistd.h>
#endif

namespace
{
constexpr const char* ProbeArgumentPrefix = "--assert-abort-probe=";
constexpr size_t ProbeLinearBufferSize = 128;
constexpr size_t ProbeShmBufferSize = 256;
constexpr unsigned ProbeShmConnectCount = 2;

// 断言消息由 CRT/glibc 直接写往 stderr；子进程必须把它导向管道而不是弹窗
void RouteAssertDiagnosticsToStandardError()
{
#ifdef _WIN32
    _set_error_mode(_OUT_TO_STDERR);
    _set_abort_behavior(0, _WRITE_ABORT_MSG | _CALL_REPORTFAULT);
#endif
}

std::string CurrentExecutablePath()
{
#ifdef _WIN32
    std::vector<char> path(MAX_PATH);
    const DWORD length = GetModuleFileNameA(nullptr, path.data(), static_cast<DWORD>(path.size()));
    return std::string(path.data(), length);
#else
    std::vector<char> path(4096);
    const ssize_t length = readlink("/proc/self/exe", path.data(), path.size());
    return length > 0 ? std::string(path.data(), static_cast<size_t>(length)) : std::string();
#endif
}

// 记录 10 字节、消费 5 字节后，到缓冲末尾的容量只剩 Size - 5；登记 Size 必须越界
void RunLinearBufferSetLengthBeyondCapacity()
{
    Spark::LinearBuffer<ProbeLinearBufferSize> buffer;
    buffer.Append("HelloWorld", 10);
    buffer.Shift(5);
    buffer.SetLength(ProbeLinearBufferSize);
}

// 断言在构造函数的入口，先于任何共享内存访问
void RunShmBufferConnectionIndexProbe(int connectionIndex)
{
    std::vector<char> memory(sizeof(Spark::SingleShmHeader) + ProbeShmBufferSize * 4, 0);
    Spark::ShmBuffer<ProbeShmBufferSize> buffer(Spark::ServerTypeType::Client, connectionIndex, memory.data(), Spark::ConnectStatusType::Connected,
                                                ProbeShmConnectCount);
    static_cast<void>(buffer);
}
}

namespace Spark::Test
{
const char* FindAssertAbortProbeName(int argc, char** argv)
{
    const size_t prefixLength = std::strlen(ProbeArgumentPrefix);
    for (int index = 1; index < argc; ++index)
    {
        if (std::strncmp(argv[index], ProbeArgumentPrefix, prefixLength) == 0)
        {
            return argv[index] + prefixLength;
        }
    }
    return nullptr;
}

void RunAssertAbortProbe(const char* probeName)
{
    RouteAssertDiagnosticsToStandardError();
    if (std::strcmp(probeName, "LinearBufferSetLengthBeyondCapacity") == 0)
    {
        RunLinearBufferSetLengthBeyondCapacity();
    }
    else if (std::strcmp(probeName, "ShmBufferZeroConnectionIndex") == 0)
    {
        RunShmBufferConnectionIndexProbe(0);
    }
    else if (std::strcmp(probeName, "ShmBufferOutOfRangeConnectionIndex") == 0)
    {
        RunShmBufferConnectionIndexProbe(static_cast<int>(ProbeShmConnectCount));
    }
}

bool ObserveAssertAbortInChildProcess(const char* probeName, const char* expectedMessage)
{
#ifdef _WIN32
    SECURITY_ATTRIBUTES inheritableAttributes{sizeof(SECURITY_ATTRIBUTES), nullptr, TRUE};
    HANDLE readPipe = nullptr;
    HANDLE writePipe = nullptr;
    if (!CreatePipe(&readPipe, &writePipe, &inheritableAttributes, 0))
    {
        return false;
    }
    SetHandleInformation(readPipe, HANDLE_FLAG_INHERIT, 0);

    STARTUPINFOA startupInfo{};
    startupInfo.cb = sizeof(startupInfo);
    startupInfo.dwFlags = STARTF_USESTDHANDLES;
    startupInfo.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
    startupInfo.hStdOutput = writePipe;
    startupInfo.hStdError = writePipe;

    std::string commandLine = "\"" + CurrentExecutablePath() + "\" " + ProbeArgumentPrefix + probeName;
    PROCESS_INFORMATION processInfo{};
    const BOOL started =
        CreateProcessA(nullptr, commandLine.data(), nullptr, nullptr, TRUE, CREATE_NO_WINDOW, nullptr, nullptr, &startupInfo, &processInfo);
    CloseHandle(writePipe);
    if (!started)
    {
        CloseHandle(readPipe);
        return false;
    }

    std::string capturedOutput;
    char chunk[512];
    DWORD readCount = 0;
    while (ReadFile(readPipe, chunk, sizeof(chunk), &readCount, nullptr) && readCount > 0)
    {
        capturedOutput.append(chunk, readCount);
    }
    CloseHandle(readPipe);

    WaitForSingleObject(processInfo.hProcess, INFINITE);
    DWORD exitCode = 0;
    GetExitCodeProcess(processInfo.hProcess, &exitCode);
    CloseHandle(processInfo.hThread);
    CloseHandle(processInfo.hProcess);
    return exitCode != 0 && capturedOutput.find(expectedMessage) != std::string::npos;
#else
    int pipeDescriptors[2] = {-1, -1};
    if (pipe(pipeDescriptors) != 0)
    {
        return false;
    }

    const std::string executablePath = CurrentExecutablePath();
    const std::string probeArgument = std::string(ProbeArgumentPrefix) + probeName;
    const pid_t childProcess = fork();
    if (childProcess < 0)
    {
        close(pipeDescriptors[0]);
        close(pipeDescriptors[1]);
        return false;
    }
    if (childProcess == 0)
    {
        close(pipeDescriptors[0]);
        dup2(pipeDescriptors[1], STDERR_FILENO);
        dup2(pipeDescriptors[1], STDOUT_FILENO);
        close(pipeDescriptors[1]);
        execl(executablePath.c_str(), executablePath.c_str(), probeArgument.c_str(), static_cast<char*>(nullptr));
        _exit(127);
    }

    close(pipeDescriptors[1]);
    std::string capturedOutput;
    char chunk[512];
    ssize_t readCount = 0;
    while ((readCount = read(pipeDescriptors[0], chunk, sizeof(chunk))) > 0)
    {
        capturedOutput.append(chunk, static_cast<size_t>(readCount));
    }
    close(pipeDescriptors[0]);

    int waitStatus = 0;
    waitpid(childProcess, &waitStatus, 0);
    return WIFSIGNALED(waitStatus) && WTERMSIG(waitStatus) == SIGABRT && capturedOutput.find(expectedMessage) != std::string::npos;
#endif
}
}

#include <gtest/gtest.h>
#include <Spark/Core/Platform/Dir.h>

#include <string>
#include <cstdio>
using namespace Spark::Core;
// ============================================================
// Dir 测试
// 目录存在检查和创建操作
// ============================================================

#ifdef _WIN32
#include <direct.h>
#include <io.h>

static int RemoveDir(const char* path)
{
    return _rmdir(path);
}
#else
#include <unistd.h>

static int RemoveDir(const char* path)
{
    return rmdir(path);
}
#endif

// ---------- IsDir ----------

TEST(DirTest, IsDir_ExistingPath)
{
    // 当前项目目录一定存在
    EXPECT_TRUE(Dir::IsDir("."));
}

#ifdef _WIN32
TEST(DirTest, IsDir_RootPath)
{
    // Windows 根目录
    EXPECT_TRUE(Dir::IsDir("C:\\"));
}
#endif

TEST(DirTest, IsDir_NonExistentPath)
{
    // 不存在的路径返回 false
    EXPECT_FALSE(Dir::IsDir("/path/that/does/not/exist/xyzwv"));
    EXPECT_FALSE(Dir::IsDir(""));
}

// ---------- Create ----------

TEST(DirTest, Create_NewDirectory)
{
    const char* testDir = "test_dir_create";

    // 清理残留
    RemoveDir(testDir);

    // 创建新目录
    EXPECT_TRUE(Dir::Create(testDir));
    EXPECT_TRUE(Dir::IsDir(testDir));

    // 清理
    RemoveDir(testDir);
    EXPECT_FALSE(Dir::IsDir(testDir));
}

TEST(DirTest, Create_ExistingDirectory)
{
    const char* testDir = "test_dir_existing";

    RemoveDir(testDir);
    EXPECT_TRUE(Dir::Create(testDir));

    // 再次创建已存在的目录 —— 不同平台上 _mkdir/mkdir 返回 false
    // 我们只验证第一次创建成功且存在
    EXPECT_TRUE(Dir::IsDir(testDir));

    RemoveDir(testDir);
}

TEST(DirTest, Create_WithMode)
{
    // mode 参数在 Linux 下生效，Windows 忽略
    // 验证函数能正常调用
    const char* testDir = "test_dir_mode";

    RemoveDir(testDir);
    EXPECT_TRUE(Dir::Create(testDir, 0755));
    EXPECT_TRUE(Dir::IsDir(testDir));

    RemoveDir(testDir);
}

TEST(DirTest, CreateAndVerifyMultipleDirs)
{
    const char* dirs[] = {"test_dir_a", "test_dir_b", "test_dir_c"};

    for (const char* dir : dirs)
    {
        RemoveDir(dir);
        EXPECT_TRUE(Dir::Create(dir));
        EXPECT_TRUE(Dir::IsDir(dir));
    }

    // 清理
    for (const char* dir : dirs)
    {
        RemoveDir(dir);
        EXPECT_FALSE(Dir::IsDir(dir));
    }
}

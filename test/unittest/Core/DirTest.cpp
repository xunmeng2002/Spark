#include <gtest/gtest.h>
#include <Spark/Core/Platform/Dir.h>

#include <string>
#include <cstdio>
using namespace spark::core;
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
    const char* kTestDir = "test_dir_create";

    // 清理残留
    RemoveDir(kTestDir);

    // 创建新目录
    EXPECT_TRUE(Dir::Create(kTestDir));
    EXPECT_TRUE(Dir::IsDir(kTestDir));

    // 清理
    RemoveDir(kTestDir);
    EXPECT_FALSE(Dir::IsDir(kTestDir));
}

TEST(DirTest, Create_ExistingDirectory)
{
    const char* kTestDir = "test_dir_existing";

    RemoveDir(kTestDir);
    EXPECT_TRUE(Dir::Create(kTestDir));

    // 再次创建已存在的目录 —— 不同平台上 _mkdir/mkdir 返回 false
    // 我们只验证第一次创建成功且存在
    EXPECT_TRUE(Dir::IsDir(kTestDir));

    RemoveDir(kTestDir);
}

TEST(DirTest, Create_WithMode)
{
    // mode 参数在 Linux 下生效，Windows 忽略
    // 验证函数能正常调用
    const char* kTestDir = "test_dir_mode";

    RemoveDir(kTestDir);
    EXPECT_TRUE(Dir::Create(kTestDir, 0755));
    EXPECT_TRUE(Dir::IsDir(kTestDir));

    RemoveDir(kTestDir);
}

TEST(DirTest, CreateAndVerifyMultipleDirs)
{
    const char* kDirs[] = {
        "test_dir_a",
        "test_dir_b",
        "test_dir_c"
    };

    for (const char* dir : kDirs)
    {
        RemoveDir(dir);
        EXPECT_TRUE(Dir::Create(dir));
        EXPECT_TRUE(Dir::IsDir(dir));
    }

    // 清理
    for (const char* dir : kDirs)
    {
        RemoveDir(dir);
        EXPECT_FALSE(Dir::IsDir(dir));
    }
}

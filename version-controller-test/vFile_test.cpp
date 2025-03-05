#include <filesystem>
#include <gtest/gtest.h>
#include <vFile.h>

class VFileTest : public ::testing::Test {
  protected:
    void SetUp() override {
        // 创建临时测试目录
        testPath = L"./test_files/";
        std::filesystem::create_directories(testPath);
    }

    void TearDown() override {
        // 清理测试目录
        std::filesystem::remove_all(L"./test_files/");
    }

    std::wstring testPath;
};

// 测试新建文件
TEST_F(VFileTest, CreateNewFile) {
    std::string content = "Hello World!";
    V_FILE      file(testPath, content);

    auto [found, version] = file.getVersion(0);
    EXPECT_FALSE(found);         // 因为还没有任何版本
    EXPECT_EQ(version, content); // 但应该返回原始内容
}

// 测试添加新版本
TEST_F(VFileTest, AddVersion) {
    std::string content1 = "Version 1";
    std::string content2 = "Version 2";
    V_FILE      file(testPath, content1);

    file.addVersion(content2, "Update to version 2", 1);

    auto [found, version] = file.getVersion(1);
    EXPECT_TRUE(found);
    EXPECT_EQ(version, content2);
}

// 测试获取多个版本
TEST_F(VFileTest, GetMultipleVersions) {
    std::string content1 = "Initial content";
    std::string content2 = "Modified content";
    std::string content3 = "Final content";

    V_FILE file(testPath, content1);
    file.addVersion(content2, "First modification", 1);
    file.addVersion(content3, "Second modification", 2);

    // 测试获取所有版本
    auto [found1, version1] = file.getVersion(0);
    auto [found2, version2] = file.getVersion(1);
    auto [found3, version3] = file.getVersion(2);

    EXPECT_FALSE(found1); // 版本0不存在
    EXPECT_EQ(version1, content1);

    EXPECT_TRUE(found2);
    EXPECT_EQ(version2, content2);

    EXPECT_TRUE(found3);
    EXPECT_EQ(version3, content3);
}

// 测试获取版本描述
TEST_F(VFileTest, GetDescriptions) {
    std::string content = "Test content";
    V_FILE      file(testPath, content);

    file.addVersion(content + " v1", "First update", 1);
    file.addVersion(content + " v2", "Second update", 2);

    auto descriptions = file.getDescriptions();

    EXPECT_EQ(descriptions.size(), 2);

    auto it = descriptions.begin();
    EXPECT_EQ(it->first, 1);
    EXPECT_EQ(it->second, "First update");

    ++it;
    EXPECT_EQ(it->first, 2);
    EXPECT_EQ(it->second, "Second update");
}

// 测试非法版本号
TEST_F(VFileTest, InvalidVersion) {
    std::string content = "Test content";
    V_FILE      file(testPath, content);

    file.addVersion(content + " v1", "Update", 1);

    auto [found, version] = file.getVersion(999); // 不存在的版本号
    EXPECT_FALSE(found);
}

// 测试加载现有文件
TEST_F(VFileTest, LoadExistingFile) {
    // 首先创建一个文件并添加一些版本
    {
        std::string content = "Original content";
        V_FILE      file(testPath, content);
        file.addVersion("Modified content", "First modification", 1);
    }

    // 然后尝试加载这个文件
    V_FILE loadedFile(testPath);
    auto [found, version] = loadedFile.getVersion(1);

    EXPECT_TRUE(found);
    EXPECT_EQ(version, "Modified content");

    auto descriptions = loadedFile.getDescriptions();
    EXPECT_EQ(descriptions.size(), 1);
    EXPECT_EQ(descriptions.begin()->second, "First modification");
}
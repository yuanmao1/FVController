// project_test.cpp
#include "project.h"
#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>

class ProjectTest : public ::testing::Test {
  protected:
    void SetUp() override {
        // 创建测试目录
        testPath = L"./test_project/";
        std::filesystem::create_directories(testPath);
    }

    void TearDown() override {
        // 清理测试目录
        std::filesystem::remove_all(L"./test_project/");
    }

    std::wstring       testPath;
    const std::wstring testProjectName = L"test_proj";
};

// 测试创建新项目
TEST_F(ProjectTest, CreateNewProject) {
    Project project = createProject(testPath, testProjectName);
    EXPECT_EQ(project.getClock(), 0);
    // 验证时钟文件被创建
    std::ifstream clockFile(testPath + L"/" + testProjectName + L"/clock");
    EXPECT_TRUE(clockFile.good());
    std::string content;
    std::getline(clockFile, content);
    EXPECT_EQ(content, "0");
}

// 测试加载现有项目
TEST_F(ProjectTest, LoadExistingProject) {
    // 首先创建一个项目
    Project project1 = createProject(testPath, testProjectName);
    // 然后尝试加载它
    Project project2(testPath + L"/" + testProjectName);
    EXPECT_EQ(project2.getClock(), 0);
}

// 测试添加版本
TEST_F(ProjectTest, AddVersion) {
    Project project = createProject(testPath, testProjectName);
    // 创建测试文件
    std::wstring testFile1 = testPath + L"/test1.txt";
    std::wstring testFile2 = testPath + L"/test2.txt";
    // 写入测试内容
    {
        std::ofstream file1(testFile1);
        file1 << "Content 1";
        std::ofstream file2(testFile2);
        file2 << "Content 2";
    }
    std::vector<std::wstring> files = {testFile1, testFile2};
    project.addVersion(files, "First version");
    EXPECT_EQ(project.getClock(), 1);
    // 检查版本信息
    std::vector<Project::Version> versions = project.getVersions();
    EXPECT_EQ(versions.size(), 1);
    EXPECT_EQ(versions[0].clock, 1);
    EXPECT_EQ(versions[0].description, "First version");
    EXPECT_EQ(versions[0].files.size(), 2);
}

// 测试多个版本
TEST_F(ProjectTest, MultipleVersions) {
    Project project = createProject(testPath, testProjectName);
    // 创建测试文件
    std::wstring testFile = testPath + L"/test.txt";
    // 第一个版本
    {
        std::ofstream file(testFile);
        file << "Version 1";
    }
    std::vector<std::wstring> files = {testFile};
    project.addVersion(files, "First version");
    // 第二个版本
    {
        std::ofstream file(testFile);
        file << "Version 2";
    }
    project.addVersion(files, "Second version");
    std::vector<Project::Version> versions = project.getVersions();
    EXPECT_EQ(versions.size(), 2);
    // 验证版本内容
    EXPECT_EQ(versions[0].clock, 1);
    EXPECT_EQ(versions[0].description, "First version");
    EXPECT_EQ(versions[1].clock, 2);
    EXPECT_EQ(versions[1].description, "Second version");
}

// 测试文件不存在的情况
TEST_F(ProjectTest, NonExistentFile) {
    Project                   project = createProject(testPath, testProjectName);
    std::vector<std::wstring> files   = {L"non_existent_file.txt"};
    project.addVersion(files, "Should handle non-existent file");
    EXPECT_EQ(project.getClock(), 1); // 时钟仍应该增加
    std::vector<Project::Version> versions = project.getVersions();
    EXPECT_EQ(versions[0].files.size(), 0); // 但不应该有文件被添加
}

// 测试项目持久化
TEST_F(ProjectTest, ProjectPersistence) {
    // 创建并添加版本
    {
        Project      project  = createProject(testPath, testProjectName);
        std::wstring testFile = testPath + L"/test.txt";
        {
            std::ofstream file(testFile);
            file << "Test content";
        }
        std::vector<std::wstring> files = {testFile};
        project.addVersion(files, "Test version");
    }

    // 重新加载项目并验证内容
    {
        Project project(testPath + L"/" + testProjectName);
        EXPECT_EQ(project.getClock(), 1);

        std::vector<Project::Version> versions = project.getVersions();
        EXPECT_EQ(versions.size(), 1);
        EXPECT_EQ(versions[0].description, "Test version");
    }
}
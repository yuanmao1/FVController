#include "project.h"
#include <filesystem>
#include <fstream>
#include <iostream>

Project::Project(std::wstring path) {
    this->path = path;
    // 读取clock文件
    std::ifstream fileStream(path + L"/clock", std::ios::in | std::ios::binary);
    if (fileStream) {
        std::string content((std::istreambuf_iterator<char>(fileStream)),
            std::istreambuf_iterator<char>());
        this->clock = std::stoi(content);
        fileStream.close();

        std::filesystem::path dirPath(path);
        if (std::filesystem::exists(dirPath) && std::filesystem::is_directory(dirPath)) {
            // 遍历目录
            for (const auto& entry : std::filesystem::directory_iterator(dirPath)) {
                // 如果是子目录
                if (entry.is_directory()) {
                    std::wstring subPath = entry.path().wstring();
                    std::string  filename = entry.path().filename().string();
                    V_FILE       newFile(subPath + L"/");
                    this->files[filename] = newFile;
                }
            }
        }
    }
    else {
        this->clock = 0;
    }
}

void Project::addVersion(std::vector<std::wstring>& pathes, std::string description) {
    ++this->clock;
    for (auto& path : pathes) {
        std::ifstream fileStream(path, std::ios::in | std::ios::binary);
        if (fileStream) {
            std::string content((std::istreambuf_iterator<char>(fileStream)), std::istreambuf_iterator<char>());
            std::wstring fileName = std::filesystem::path(path).filename().wstring();
            std::string filename = std::string(fileName.begin(), fileName.end());
            if (this->files.find(filename) != this->files.end()) {
                this->files[filename].addVersion(content, description, this->clock);
            }
            else {
                std::wstring subPath = this->path + L"/" + fileName + L"/";
                std::string subPathStr(subPath.begin(), subPath.end());
                V_FILE newFile(subPath, content);
                newFile.addVersion(content, description, this->clock);
                this->files[filename] = newFile;
            }
        }
        else {
            // 处理文件读取失败的情况
            std::cerr << "Failed to read file: " << std::string(path.begin(), path.end()) << std::endl;
        }
    }
    // 更新clock
    std::ofstream clockFile(this->path + L"/clock", std::ios::out);
    clockFile << this->clock;
    clockFile.close();
}

std::vector<Project::Version> Project::getVersions() {
    if (!this->clock) return std::vector<Project::Version>();
    std::vector<Project::Version> res(this->clock);
    for (auto i = 1; i <= this->clock; ++i) {
        res[i - 1].clock = i;
        for (auto& [name, file] : this->files) {
            auto [ok, content] = file.getVersion(i);
            if (ok && !name.empty() && !content.empty()) {
                res[i - 1].files.emplace_back(std::make_pair(name, content));
            }
        }
    }
    for (auto& [name, file] : this->files) {
        auto li = file.getDescriptions();
        for (auto& [clock, description] : li) {
            res[clock - 1].description = description;
        }
    }
    return res;
}

[[nodiscard]]
Project createProject(std::wstring path, std::wstring name) {
    std::filesystem::path projectPath = path + L"/" + name;
    std::filesystem::create_directory(projectPath);
    std::ofstream clockFile(projectPath / L"clock", std::ios::out | std::ios::binary);
    if (clockFile) {
        clockFile << "0";
        clockFile.close();
    }
    return Project(projectPath);
};
#include "vFile.h"  
#include <fstream>  
#include <sstream>  
#include <iostream>  
#include <filesystem>  
#include <vector>  
#include <codecvt>
#include <algorithm>  

explicit V_FILE::V_FILE(std::wstring path, std::string_view content) {
    try {
        std::wofstream file(path + L"pc");
        if (!file.is_open()) {
            throw std::runtime_error("Unable to create file");
        }

        std::wstring wContent(content.begin(), content.end());
        file << wContent;
        file.close();

        this->primitiveContent = content;
        this->nodes.clear();
    }
    catch (const std::exception& e) {
        std::cerr << "Error in V_FILE constructor: " << e.what() << std::endl;
        throw;
    }
};

explicit V_FILE::V_FILE(std::wstring path) {
    try {
        // 读取原始内容
        std::wifstream pcFile(path + L"pc");
        if (!pcFile.is_open()) {
            throw std::runtime_error("Unable to open pc file");
        }
        std::stringstream pcss;
        pcss << std::wstring_convert<std::codecvt_utf8<wchar_t>>().to_bytes(std::wstring(std::istreambuf_iterator<wchar_t>(pcFile), {}));
        this->primitiveContent = pcss.str();
        pcFile.close();

        // 获取所有版本文件
        std::vector<std::pair<uint32_t, std::wstring>> versionFiles;
        std::filesystem::path dirPath = path.substr(0, path.find_last_of(L"/\\"));

        for (const auto& entry : std::filesystem::directory_iterator(dirPath)) {
            std::wstring filename = entry.path().filename().wstring();
            if (filename.find(L"version") == 0) {
                uint32_t clock = std::stoul(filename.substr(7));
                versionFiles.emplace_back(clock, entry.path().wstring());
            }
        }

        // 按clock排序
        std::sort(versionFiles.begin(), versionFiles.end());

        // 读取各个版本文件内容
        for (const auto& [clock, filePath] : versionFiles) {
            std::wifstream vFile(filePath);
            if (!vFile.is_open()) continue;

            std::wstring patchLine;
            std::wstring description;

            // 读取第一行作为patches
            if (std::getline(vFile, patchLine)) {
                // 读取剩余行作为description
                std::wstring line;
                while (std::getline(vFile, line)) {
                    if (!description.empty()) description += L"\n";
                    description += line;
                }

                // 解析patches并创建node
                diff_match_patch<std::string>::Patches patches = 
                    V_FILE::diff.patch_fromText(std::wstring_convert<std::codecvt_utf8<wchar_t>>().to_bytes(patchLine));
                node vNode = std::make_tuple(clock, 
                    std::wstring_convert<std::codecvt_utf8<wchar_t>>().to_bytes(description),
                    patches);
                this->nodes.push_back(vNode);
            }

            vFile.close();
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Error in V_FILE constructor: " << e.what() << std::endl;
        throw;
    }
}

std::string V_FILE::getVersion(uint32_t clock) {
    std::string res = this->primitiveContent;
    for (auto const& [c, _, patches] : this->nodes) {
        if (c > clock) break;
        V_FILE::diff.patch_apply(patches, res);
    }
    return res;
};

std::list<std::pair<uint32_t, std::string>> V_FILE::getDescriptions() {
    std::list<std::pair<uint32_t, std::string>> res;
    for (auto const& [clock, description, _] : this->nodes) {
        res.emplace_back(std::make_pair(clock, description));
    }
    return res;
};

void V_FILE::addVersion(std::string currentContent, std::string_view description, uint32_t clock, std::wstring path) {
    std::string old = this->getVersion(clock - 1);
    diff_match_patch<std::string>::Patches patches = V_FILE::diff.patch_make(old, currentContent);
    V_FILE::node node = std::make_tuple(clock, std::string(description), patches);
    this->nodes.push_back(node);

    // 保存到磁盘
    try {
        std::wstring filename = path + L"version" + std::to_wstring(clock);
        std::wofstream file(filename);
        if (!file.is_open()) {
            throw std::runtime_error("Unable to create version file");
        }

        // 写入patches文本（第一行）
        std::string patchText = V_FILE::diff.patch_toText(patches);
        std::wstring wPatchText(patchText.begin(), patchText.end());
        file << wPatchText << L"\n";

        // 写入描述（从第二行开始）
        std::wstring wDescription(description.begin(), description.end());
        file << wDescription;

        file.close();
    }
    catch (const std::exception& e) {
        std::cerr << "Error saving version file: " << e.what() << std::endl;
        throw;
    }
};

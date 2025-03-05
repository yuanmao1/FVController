#include "diff-match-patch.h"
#include "vFile.h"
#include <Windows.h>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>

diff_match_patch<std::string> V_FILE::diff;

std::string wstring_to_string(const std::wstring& wstr) {
    if (wstr.empty())
        return std::string();
    int size_needed =
        WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), NULL, 0, NULL, NULL);
    std::string str(size_needed, 0);
    WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), &str[0], size_needed, NULL, NULL);
    return str;
}

std::pair<bool, std::string> V_FILE::getVersion(uint32_t clock) {
    std::string res = this->primitiveContent;
    for (auto const& [c, _, patches] : this->nodes) {
        if (c > clock) {
            return std::make_pair(false, res);
        }
        auto [temp, ok] = V_FILE::diff.patch_apply(patches, res);
        res = temp;
        if (c == clock)
            return std::make_pair(true, res);
    }
    return std::make_pair(false, res);
};

std::list<std::pair<uint32_t, std::string>> V_FILE::getDescriptions() {
    std::list<std::pair<uint32_t, std::string>> res;
    for (auto const& [clock, description, _] : this->nodes) {
        res.emplace_back(std::make_pair(clock, description));
    }
    return res;
};

V_FILE::V_FILE(std::wstring path) {
    std::ifstream file(path + L"primitiveContent.txt");
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << wstring_to_string(path) + "primitiveContent.txt"
            << std::endl;
        return;
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    this->primitiveContent = buffer.str();
    file.close();

    // load nodes
    std::filesystem::path p(path);
    for (const auto& entry : std::filesystem::directory_iterator(p)) {
        if (entry.is_directory()) {
            uint32_t clock = std::stoul(entry.path().filename().string());
            // 读入description
            std::ifstream file(entry.path().wstring() + L"/description.txt");
            if (!file.is_open()) {
                std::cerr << "Failed to open file: " << wstring_to_string(entry.path().wstring())
                    << "description.txt" << std::endl;
                continue;
            }
            std::stringstream buffer;
            buffer << file.rdbuf();
            std::string description = buffer.str();
            file.close();
            // 读入patches
            std::ifstream file2(entry.path().wstring() + L"/patches.txt");
            if (!file2.is_open()) {
                std::cerr << "Failed to open file: " << wstring_to_string(entry.path().wstring())
                    << "patches.txt" << std::endl;
                continue;
            }
            std::stringstream buffer2;
            buffer2 << file2.rdbuf();
            std::string patches = buffer2.str();
            file2.close();
            this->nodes.emplace_back(
                std::make_tuple(clock, description, V_FILE::diff.patch_fromText(patches)));
        }
    }
    this->path = path;
}

V_FILE::V_FILE(std::wstring path, std::string content) {
    this->primitiveContent = content;
    std::filesystem::create_directories(path);
    std::ofstream file(path + L"primitiveContent.txt");
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << wstring_to_string(path) + "primitiveContent.txt"
            << std::endl;
        return;
    }
    file << content;
    file.close();
    this->path = path;
}

void V_FILE::addVersion(std::string currentContent, std::string_view description, uint32_t clock) {
    std::string temp = this->primitiveContent;
    for (auto& [c, _, patches] : this->nodes) {
        auto [res, ok] = V_FILE::diff.patch_apply(patches, temp);
        temp = res;
    }
    diff_match_patch<std::string>::Patches patches = diff.patch_make(temp, currentContent);
    this->nodes.emplace_back(std::make_tuple(clock, description, patches));
    // save
    std::filesystem::create_directories(this->path + std::to_wstring(clock) + L"/");
    // 写入描述
    std::ofstream file(this->path + std::to_wstring(clock) + L"/description.txt");
    if (!file.is_open()) {
        std::cerr << "Failed to open file: "
            << wstring_to_string(this->path) + std::to_string(clock) << "description.txt"
            << std::endl;
        return;
    }
    file << description;
    file.close();
    // 写入patches
    std::ofstream file2(this->path + std::to_wstring(clock) + L"/patches.txt");
    if (!file2.is_open()) {
        std::cerr << "Failed to open file: "
            << wstring_to_string(this->path) + std::to_string(clock) << "patches.txt"
            << std::endl;
        return;
    }
    file2 << diff.patch_toText(patches);
    file2.close();
}
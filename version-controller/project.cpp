#include "project.h"
#include <filesystem>
#include <fstream>


Project::Project(std::wstring path) {
    this->path = path;
    // ��ȡclock�ļ�
    std::ifstream fileStream(path + L"/clock", std::ios::in | std::ios::binary);
    if (fileStream) {
        std::string content((std::istreambuf_iterator<char>(fileStream)), std::istreambuf_iterator<char>());
        this->clock = std::stoi(content);
        fileStream.close();

        // ���Ŀ¼�Ƿ����
        std::filesystem::path dirPath(path);
        if (std::filesystem::exists(dirPath) && std::filesystem::is_directory(dirPath)) {
            // ����Ŀ¼
            for (const auto& entry : std::filesystem::directory_iterator(dirPath)) {
                // ����clock�ļ�
                if (entry.path().filename() == L"clock") continue;

                if (entry.is_regular_file()) {
                    std::wstring fileName = entry.path().filename().wstring();
                    std::string filenameStr(fileName.begin(), fileName.end());

                    // ����V_FILE������map
                    V_FILE newFile(entry.path().wstring());
                    this->files[filenameStr] = newFile;
                }
            }
        }
    }
    else {
        this->clock = 0;
    }
}

std::map<std::string, V_FILE>& Project::getFiles() {
	return this->files;
};

void Project::addVersion(std::vector<std::wstring>& pathes, std::string description) {
    ++this->clock;
    for (auto& path : pathes) {
        std::ifstream fileStream(path, std::ios::in | std::ios::binary);
        if (fileStream) {
            std::string content((std::istreambuf_iterator<char>(fileStream)), std::istreambuf_iterator<char>());
            std::wstring fileName = std::filesystem::path(path).filename().wstring();
            std::wstring subPath = this->path + L"/" + fileName;
            std::string subPathStr(subPath.begin(), subPath.end());
            std::string filename = std::string(fileName.begin(), fileName.end());
            if (this->files.find(subPathStr) != this->files.end()) {
                this->files[filename].addVersion(content, description, this->clock, subPath);
            } else {
                V_FILE newFile(subPath, content, description, this->clock);
                this->files[filename] = newFile;
            }
        }
    }
}
#pragma once

#ifndef _PROJECT_H_
#define _PROJECT_H

#include "vFile.h"
#include <cstdint>
#include <map>
#include <string>
#include <vector>

class Project {
private:
    std::map<std::string, V_FILE> files; // name - object
    std::wstring                  path;
    uint32_t                      clock;

public:
    struct Version {
        std::string                                      description;
        std::vector<std::pair<std::string, std::string>> files;
        uint32_t                                         clock;
    };
    Project() = delete;
    Project(std::wstring path);
    void                 addVersion(std::vector<std::wstring>& pathes, std::string description);
    std::vector<Version> getVersions();
    uint32_t             getClock() const noexcept { return clock; };
};

[[nodiscard]]
Project createProject(std::wstring path, std::wstring name);

#endif
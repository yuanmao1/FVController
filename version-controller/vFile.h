#pragma once
#ifndef _VFILE_H_
#define _VFILE_H_

#include "diff-match-patch.h"
#include <cstdint>
#include <list>
#include <string>
#include <string_view>
#include <utility>

class V_FILE {
    // clock description patch
    typedef std::tuple<uint32_t, std::string, diff_match_patch<std::string>::Patches> node;

private:
    static diff_match_patch<std::string> diff;
    std::string                          primitiveContent;
    std::list<node>                      nodes;
    std::wstring                         path;

public:
    V_FILE(std::wstring path);
    V_FILE(std::wstring path, std::string content);
    V_FILE() = default;

    std::list<std::pair<uint32_t, std::string>> getDescriptions();
    std::pair<bool, std::string>                getVersion(uint32_t clock);

    void addVersion(std::string currentContent, std::string_view description, uint32_t clock);
};

#endif
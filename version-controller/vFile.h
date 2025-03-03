#pragma once  
#ifndef _VFILE_H_  
#define _VFILE_H_  

#include <string>
#include <vector>
#include <utility>
#include <cstdint>
#include <string_view>
#include <list>
#include "diff-match-patch.h"

class V_FILE {
	// clock description patch
	typedef std::tuple<uint32_t, std::string, diff_match_patch<std::string>::Patches> node;
private:
	static diff_match_patch<std::string> diff;
	std::string primitiveContent;
	std::list<node> nodes;
public:
	explicit V_FILE(std::wstring path);
	explicit V_FILE(std::wstring path, std::string_view content, std::string description, uint32_t clock);
	V_FILE() = delete;
	std::string getVersion(uint32_t clock);
	std::list<std::pair<uint32_t, std::string>> getDescriptions();
	void addVersion(std::string currentContent, std::string_view description, uint32_t clock, std::wstring path);
};

#endif
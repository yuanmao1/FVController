#pragma once

#ifndef _PROJECT_H_
#define _PROJECT_H

#include "vFile.h"
#include <string>
#include <vector>
#include <cstdint>
#include <map>

class Project {
private:
	std::map<std::string, V_FILE> files; // name - object
	uint32_t clock;
public:
	Project() = delete;
	Project(const Project& other) = delete; // 删除复制构造函数
	Project(const Project&& other) = delete; // 删除移动构造函数
	Project& operator=(Project& other) = delete; // 删除复制赋值运算符
	Project& operator=(const Project&& other) = delete; // 删除移动赋值运算符
	Project(std::wstring path);
	std::vector<V_FILE> getFiles();
	void addVersion(std::vector<std::wstring> pathes, std::string description);
};


#endif
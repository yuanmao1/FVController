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
	Project(const Project& other) = delete; // ɾ�����ƹ��캯��
	Project(const Project&& other) = delete; // ɾ���ƶ����캯��
	Project& operator=(Project& other) = delete; // ɾ�����Ƹ�ֵ�����
	Project& operator=(const Project&& other) = delete; // ɾ���ƶ���ֵ�����
	Project(std::wstring path);
	std::vector<V_FILE> getFiles();
	void addVersion(std::vector<std::wstring> pathes, std::string description);
};


#endif
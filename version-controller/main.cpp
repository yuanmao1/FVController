#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_win32.h"
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>
#include <GL/GL.h>
#include <tchar.h>
#include <shlobj.h>
#include <shellapi.h> // 添加此行以定义 HDROP
#include <string>
#include "project.h"

std::wstring GetDefaultPath() {
    wchar_t path[MAX_PATH];
    if (SUCCEEDED(SHGetFolderPathW(NULL, CSIDL_PROFILE, NULL, 0, path))) {
        return std::wstring(path) + L"\\.version-control";
    }
    return L"";
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    std::wstring defaultPath = GetDefaultPath();
    // 交互界面代码
}
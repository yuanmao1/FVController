#include "project.h"
#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_win32.h>
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>
#include <cwchar>
#include <filesystem>
#include <gl/gl.h>
#include <shellapi.h>
#include <shlobj.h>
#include <string>
#include <tchar.h>

// ǰ������
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

std::wstring GetDefaultPath() {
    wchar_t path[MAX_PATH];
    if (SUCCEEDED(SHGetFolderPathW(NULL, CSIDL_PROFILE, NULL, 0, path))) {
        return std::wstring(path) + L"\\.version-controller";
    }
    return L"";
}

// ȫ�ֱ���
Project* selectedProject = nullptr;
Project::Version* selectedVersion = nullptr;
std::vector<std::pair<std::string, Project>> projects;
std::string                                  selectedFile;
std::string                                  currentContent;

// ������ק�ļ��¼�
static bool pendingPopup = false;
static std::vector<std::wstring> pendingPaths;

// ɨ����Ŀ�ļ���
void ScanProjects(const std::wstring& defaultPath) {
    if (!std::filesystem::exists(defaultPath)) {
        std::filesystem::create_directories(defaultPath);
        return;
    }

    for (const auto& entry : std::filesystem::directory_iterator(defaultPath)) {
        if (entry.is_directory()) {
            std::wstring projectPath = entry.path().wstring();
            std::string  projectName = entry.path().filename().string();
            Project      project(projectPath);
            projects.emplace_back(std::make_pair(projectName, project));
        }
    }
}

// ��ȾGUI
void RenderGUI() {
    // ��Ŀ�б���
    ImGui::Begin("Projects");
    if (ImGui::Button("Add Project")) {
        ImGui::OpenPopup("Add Project Dialog");
    }

    static char projectName[256] = "";
    if (ImGui::BeginPopupModal("Add Project Dialog", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Enter project name:");
        ImGui::InputText("##project_name", projectName, 256);

        if (ImGui::Button("OK", ImVec2(120, 0))) {
            if (strlen(projectName) > 0) {
                std::wstring defaultPath = GetDefaultPath();
                std::wstring newProjectPath = defaultPath + L"\\" + std::wstring(projectName, projectName + strlen(projectName));

                if (!std::filesystem::exists(newProjectPath)) {
                    std::wstring name(projectName, projectName + strlen(projectName));
                    Project newProject = createProject(GetDefaultPath(), name);
                    projects.emplace_back(std::make_pair(std::string(name.begin(), name.end()), newProject));
                    projectName[0] = '\0';
                    ImGui::CloseCurrentPopup();
                }
                else {
                    MessageBoxW(NULL, L"Project already exists!", L"Error", MB_OK | MB_ICONERROR);
                }
            }
        }

        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(120, 0))) {
            projectName[0] = '\0';
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }

    for (auto& [name, project] : projects) {
        if (ImGui::Selectable(name.c_str(), (selectedProject != nullptr  && selectedProject == &project))) {
            if (!selectedProject) {
                delete selectedProject;
            }
            selectedProject = &project;
            selectedVersion = nullptr;
            selectedFile.clear();
        }
    }
    ImGui::End();

    // ���ѡ������Ŀ����ʾ�汾�б�
    if (selectedProject) {
        ImGui::Begin("Versions");
        auto versions = selectedProject->getVersions();
        for (auto& version : versions) {
            if (ImGui::Selectable(version.description.c_str(), selectedVersion == &version)) {
                if (!selectedVersion) {
                    delete selectedVersion;
                }
                selectedVersion = new Project::Version(version);
                selectedFile.clear();
            }
        }
        ImGui::End();

        // ���ѡ���˰汾����ʾ�ļ��б�
        if (selectedVersion) {
            ImGui::Begin("Files");
            for (auto& [filename, content] : selectedVersion->files) {
                if (ImGui::Selectable(filename.c_str(), !selectedFile.empty() && selectedFile == filename)) {
                    selectedFile = filename; 
                    currentContent = content;
                }
            }
            ImGui::End();

            // ���ѡ�����ļ�����ʾ����
            if (!selectedFile.empty()) {
                ImGui::Begin("Content");
                ImGui::TextWrapped("Filename: %s", selectedFile.c_str());
                ImGui::TextWrapped("%s", currentContent.c_str());
                ImGui::End();
            }
        }
    }
    if (pendingPopup) {
        ImGui::OpenPopup("Add Description Dialog");
        pendingPopup = false;
    }
    static char description[256] = "";
    if (ImGui::BeginPopupModal("Add Description Dialog", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Enter version description:");
        ImGui::InputText("##description", description, 256);

        if (ImGui::Button("OK", ImVec2(120, 0))) {
            if (strlen(description) > 0 && selectedProject != nullptr) {
                selectedProject->addVersion(pendingPaths, std::string(description));
                description[0] = '\0';
                pendingPaths.clear();
                ImGui::CloseCurrentPopup();
            }
        }

        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(120, 0))) {
            description[0] = '\0';
            pendingPaths.clear();
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}

// Win32��Ϣ����
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND   hWnd,
    UINT   msg,
    WPARAM wParam,
    LPARAM lParam);

LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    switch (msg) {
    case WM_DROPFILES: {
        if (selectedProject) {
            HDROP                     hDrop = (HDROP)wParam;
            UINT                      fileCount = DragQueryFileW(hDrop, 0xFFFFFFFF, NULL, 0);
            std::vector<std::wstring> paths;

            for (UINT i = 0; i < fileCount; i++) {
                wchar_t filePath[MAX_PATH];
                DragQueryFileW(hDrop, i, filePath, MAX_PATH);
                paths.push_back(filePath);
            }

            if (!paths.empty()) {
                pendingPaths = paths;
                pendingPopup = true;
            }

            DragFinish(hDrop);
        }
        return 0;
    }
    case WM_SIZE:
        if (wParam != SIZE_MINIMIZED) {
            // ����OpenGL�ӿ�
            glViewport(0, 0, LOWORD(lParam), HIWORD(lParam));
        }
        return 0;
    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU) // ����ALT�˵�
            return 0;
        break;
    case WM_DESTROY: PostQuitMessage(0); return 0;
    }
    return DefWindowProc(hWnd, msg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    // ע�ᴰ����
    WNDCLASSEXW wc = { sizeof(WNDCLASSEXW) };
    wc.style = CS_CLASSDC;
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = L"Version Controller";
    RegisterClassExW(&wc);

    // ��������
    HWND hwnd =
        CreateWindowW(wc.lpszClassName, L"Version Controller", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT,
            CW_USEDEFAULT, 1280, 720, NULL, NULL, wc.hInstance, NULL);

    // ��ʼ��OpenGL
    HDC                   hDC = GetDC(hwnd);
    PIXELFORMATDESCRIPTOR pfd = { sizeof(PIXELFORMATDESCRIPTOR) };
    pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 24;

    int pf = ChoosePixelFormat(hDC, &pfd);
    SetPixelFormat(hDC, pf, &pfd);
    HGLRC hglrc = wglCreateContext(hDC);
    wglMakeCurrent(hDC, hglrc);

    // �����ļ��Ϸ�
    DragAcceptFiles(hwnd, TRUE);

    // ��ʼ��ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplOpenGL3_Init();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\msyh.ttc", 18.0f, nullptr,
        io.Fonts->GetGlyphRangesChineseFull());

    // ɨ����Ŀ
    std::wstring defaultPath = GetDefaultPath();
    ScanProjects(defaultPath);

    // ��ʾ����
    ShowWindow(hwnd, SW_SHOWDEFAULT);
    UpdateWindow(hwnd);

    // ��ѭ��
    bool done = false;
    while (!done) {
        MSG msg;
        while (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
            if (msg.message == WM_QUIT)
                done = true;
        }
        if (done)
            break;

        // ��ʼ��֡
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        // ��ȾGUI
        RenderGUI();

        // ��Ⱦ
        ImGui::Render();
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        SwapBuffers(hDC);
    }

    // ����
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    wglMakeCurrent(NULL, NULL);
    wglDeleteContext(hglrc);
    ReleaseDC(hwnd, hDC);
    DestroyWindow(hwnd);
    UnregisterClassW(wc.lpszClassName, wc.hInstance);

    return 0;
}
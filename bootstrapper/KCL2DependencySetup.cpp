#include <windows.h>
#include <wininet.h>
#include <string>
#include <iostream>
#pragma comment(lib, "wininet.lib")

// KCL2 dependency bootstrapper (native, no .NET dependency).
// Downloads and silently installs the .NET Framework 4.8 runtime, then
// shows a completion dialog and removes itself on exit.

static const wchar_t* kDotNetUrl =
    L"https://go.microsoft.com/fwlink/?linkid=2088631";
static const wchar_t* kInstallerName = L"KCL2-dotnet48.exe";

static void PrintLine(const std::wstring& text) {
    DWORD written = 0;
    WriteConsoleW(GetStdHandle(STD_OUTPUT_HANDLE), (text + L"\r\n").c_str(),
                  (DWORD)(text.size() + 1), &written, nullptr);
}

static bool DownloadFile(const wchar_t* url, const std::wstring& path) {
    HINTERNET hInternet = InternetOpenW(L"KCL2DependencySetup/1.0",
        INTERNET_OPEN_TYPE_PRECONFIG, nullptr, nullptr, 0);
    if (!hInternet) return false;

    DWORD timeout = 30000;
    InternetSetOptionW(hInternet, INTERNET_OPTION_CONNECT_TIMEOUT, &timeout, sizeof(timeout));
    InternetSetOptionW(hInternet, INTERNET_OPTION_RECEIVE_TIMEOUT, &timeout, sizeof(timeout));

    bool ok = false;
    for (int attempt = 1; attempt <= 3 && !ok; ++attempt) {
        PrintLine(L"[KCL2] 正在下载 Microsoft .NET Framework 4.8，第 " +
                  std::to_wstring(attempt) + L" 次尝试，请耐心等待……");

        HINTERNET hUrl = InternetOpenUrlW(hInternet, url, nullptr, 0,
            INTERNET_FLAG_RELOAD | INTERNET_FLAG_NO_CACHE_WRITE |
            INTERNET_FLAG_SECURE | INTERNET_FLAG_NO_UI, 0);
        if (!hUrl) {
            PrintLine(L"[KCL2] 连接失败，稍后重试。");
            Sleep(3000);
            continue;
        }

        HANDLE hFile = CreateFileW(path.c_str(), GENERIC_WRITE, 0, nullptr,
                                   CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
        if (hFile == INVALID_HANDLE_VALUE) { InternetCloseHandle(hUrl); break; }

        char buffer[65536];
        DWORD readBytes = 0;
        ULONGLONG total = 0;
        ULONGLONG lastMark = 0;
        ok = true;
        while (InternetReadFile(hUrl, buffer, sizeof(buffer), &readBytes) && readBytes > 0) {
            DWORD written = 0;
            if (!WriteFile(hFile, buffer, readBytes, &written, nullptr) || written != readBytes) {
                ok = false;
                break;
            }
            total += readBytes;
            if (total - lastMark >= 10ULL * 1024 * 1024) {
                lastMark = total;
                PrintLine(L"[KCL2] 已下载 " + std::to_wstring(total / (1024 * 1024)) + L" MB");
            }
        }
        CloseHandle(hFile);
        InternetCloseHandle(hUrl);

        if (ok && total < 50ULL * 1024 * 1024) {
            ok = false; // real offline installer is ~100MB+, reject truncated file
            PrintLine(L"[KCL2] 下载不完整，重新尝试。");
            Sleep(3000);
        }
    }
    InternetCloseHandle(hInternet);
    return ok;
}

static void ScheduleSelfDelete(const std::wstring& selfPath) {
    wchar_t tempDir[MAX_PATH] = {0};
    GetTempPathW(MAX_PATH, tempDir);
    std::wstring batW = std::wstring(tempDir) + L"kcl2-cleanup.bat";

    std::wstring script;
    script += L"@echo off\r\n";
    script += L":kclwait\r\n";
    script += L"timeout /t 1 /nobreak >nul\r\n";
    script += L"del /f /q \"" + selfPath + L"\" >nul 2>nul\r\n";
    script += L"if exist \"" + selfPath + L"\" goto kclwait\r\n";
    script += L"(goto) 2>nul & del \"%~f0\"\r\n";

    int len = WideCharToMultiByte(CP_ACP, 0, script.c_str(), -1, nullptr, 0, nullptr, nullptr);
    std::string ansi(len > 0 ? len - 1 : 0, '\0');
    WideCharToMultiByte(CP_ACP, 0, script.c_str(), -1, &ansi[0], len, nullptr, nullptr);

    int blen = WideCharToMultiByte(CP_ACP, 0, batW.c_str(), -1, nullptr, 0, nullptr, nullptr);
    std::string batA(blen > 0 ? blen - 1 : 0, '\0');
    WideCharToMultiByte(CP_ACP, 0, batW.c_str(), -1, &batA[0], blen, nullptr, nullptr);

    HANDLE h = CreateFileA(batA.c_str(), GENERIC_WRITE, 0, nullptr,
                           CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (h != INVALID_HANDLE_VALUE) {
        DWORD written = 0;
        WriteFile(h, ansi.data(), (DWORD)ansi.size(), &written, nullptr);
        CloseHandle(h);
        std::string cmd = "/c \"" + batA + "\"";
        ShellExecuteA(nullptr, "open", "cmd.exe", cmd.c_str(), nullptr, SW_HIDE);
    }
}

int wmain() {
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleTitleW(L"KCL2 依赖安装");

    wchar_t selfPath[MAX_PATH] = {0};
    GetModuleFileNameW(nullptr, selfPath, MAX_PATH);
    std::wstring dir(selfPath);
    size_t slash = dir.find_last_of(L"\\/");
    dir = dir.substr(0, slash + 1);
    std::wstring installer = dir + kInstallerName;

    PrintLine(L"==================================================");
    PrintLine(L" KCL2 运行依赖安装程序（ETC Team）");
    PrintLine(L" 将下载并安装 Microsoft .NET Framework 4.8");
    PrintLine(L"==================================================");

    if (!DownloadFile(kDotNetUrl, installer)) {
        MessageBoxW(nullptr,
            L"依赖下载失败，请检查网络连接后重新运行本程序。\n"
            L"也可手动搜索安装 “.NET Framework 4.8 离线安装包”。",
            L"KCL2 依赖安装", MB_ICONERROR | MB_OK);
        return 1;
    }

    PrintLine(L"[KCL2] 下载完成，开始静默安装，期间请不要关闭窗口……");
    SHELLEXECUTEINFOW sei = { sizeof(sei) };
    sei.fMask = SEE_MASK_NOCLOSEPROCESS;
    sei.lpVerb = L"open";
    sei.lpFile = installer.c_str();
    sei.lpParameters = L"/q /norestart";
    sei.nShow = SW_SHOWNORMAL;
    DWORD code = 1;
    if (ShellExecuteExW(&sei) && sei.hProcess) {
        WaitForSingleObject(sei.hProcess, INFINITE);
        GetExitCodeProcess(sei.hProcess, &code);
        CloseHandle(sei.hProcess);
    }

    DeleteFileW(installer.c_str());

    bool success = (code == 0 || code == 3010 || code == 1638 ||
                    code == ERROR_SUCCESS_REBOOT_REQUIRED);
    if (success) {
        MessageBoxW(nullptr,
            L"已完成所有安装。安装程序已退出并自动清理，\n"
            L"现在可以重新启动 KCL2 正常使用了。",
            L"KCL2 依赖安装完成", MB_ICONINFORMATION | MB_OK);
        ScheduleSelfDelete(selfPath);
        return 0;
    }
    MessageBoxW(nullptr,
        (L"安装未正常完成，错误代码：" + std::to_wstring(code) +
         L"\n可稍后重试，或手动安装 .NET Framework 4.8。").c_str(),
        L"KCL2 依赖安装", MB_ICONWARNING | MB_OK);
    return 1;
}

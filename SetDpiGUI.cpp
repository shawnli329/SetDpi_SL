#include "DpiHelper.h"
#include <Windows.h>
#include <shellapi.h>
#include <iostream>
#include <vector>
#include <string>
#include <map>

// Base64������ش���
static const std::string base64_chars =
"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
"abcdefghijklmnopqrstuvwxyz"
"0123456789+/";

std::vector<BYTE> base64_decode(const std::string& encoded_string) {
    int in_len = encoded_string.size();
    int i = 0;
    int j = 0;
    int in = 0;
    unsigned char char_array_4[4], char_array_3[3];
    std::vector<BYTE> ret;

    while (in_len-- && (encoded_string[in] != '=') && (isalnum(encoded_string[in]) || (encoded_string[in] == '+') || (encoded_string[in] == '/'))) {
        char_array_4[i++] = encoded_string[in]; in++;
        if (i == 4) {
            for (i = 0; i < 4; i++)
                char_array_4[i] = base64_chars.find(char_array_4[i]);

            char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
            char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
            char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

            for (i = 0; (i < 3); i++)
                ret.push_back(char_array_3[i]);
            i = 0;
        }
    }

    if (i) {
        for (j = i; j < 4; j++)
            char_array_4[j] = 0;

        for (j = 0; j < 4; j++)
            char_array_4[j] = base64_chars.find(char_array_4[j]);

        char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
        char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
        char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

        for (j = 0; (j < i - 1); j++) ret.push_back(char_array_3[j]);
    }

    return ret;
}

HICON CreateIconFromBase64(const std::string& base64_data) {
    std::vector<BYTE> icon_data = base64_decode(base64_data);

    if (icon_data.empty()) {
        return NULL;
    }

    HICON hIcon = CreateIconFromResourceEx(
        icon_data.data(),
        icon_data.size(),
        TRUE,
        0x00030000,
        16, 16,
        LR_DEFAULTCOLOR
    );

    return hIcon;
}

// ������������ͼ���Base64����
// ����һ��ʾ��16x16͸��ͼ�꣬����Ҫ�滻Ϊ���Լ���ͼ������
const std::string TRAY_ICON_BASE64 =
"iVBORw0KGgoAAAANSUhEUgAAABAAAAAQCAYAAAAf8/9hAAABS2lUWHRYTUw6Y29tLmFkb2JlLnhtcAAAAAAAPD94cGFja2V0IGJlZ2luPSLvu78iIGlkPSJXNU0wTXBDZWhpSHpyZVN6TlRjemtjOWQiPz4KPHg6eG1wbWV0YSB4bWxuczp4PSJhZG9iZTpuczptZXRhLyIgeDp4bXB0az0iQWRvYmUgWE1QIENvcmUgNS42LWMxNDIgNzkuMTYwOTI0LCAyMDE3LzA3LzEzLTAxOjA2OjM5ICAgICAgICAiPgogPHJkZjpSREYgeG1sbnM6cmRmPSJodHRwOi8vd3d3LnczLm9yZy8xOTk5LzAyLzIyLXJkZi1zeW50YXgtbnMjIj4KICA8cmRmOkRlc2NyaXB0aW9uIHJkZjphYm91dD0iIi8+CiA8L3JkZjpSREY+CjwveDp4bXBtZXRhPgo8P3hwYWNrZXQgZW5kPSJyIj8+nhxg7wAAAVlJREFUOI2lU7FKBDEUnJWg2PkXWin4Awq2dnqFCOJ/+DdWBxZaWKugKFjZiaBf4TWXvDdjkWSze3LVbZNs3mTeZCYBVvy6q+cX3X39QBToAsooF0hBzHMQeZ0VQ5zu7SDcfn5j6+AIcoBGyAg3AS64C0pshC7QCFqeT9+fEGSExwyom+WECqj+0wRZxrmxJwpuAhP7TixgFnDuRsgBJYd7q8mI4Eb43LNMKyp8TNAUEHQMMESQCZ5Y5GF0xp5k4EX2qdUDjfA5B92KURVcOlaFrSYwCYEmeMwEYyMruKRTfKC1JNyYCZhyfLIcHarTfbTt/BwkJBMCE7OCYpYvmufoG7g3ZXXMCuLYoMVL8/846qMOk/1tTN8eIaK/tlKJVAKtrNcaMxEonB/uopO09KFc39wLAC4mx90yzNrKr3FRwcPrh35nM6QUkVJCjLHMIzY31nF5djJS8wfPdjqEIqzNZQAAAABJRU5ErkJggg==";

// ȫ�ֱ���
HINSTANCE g_hInstance;
HWND g_hWnd;
NOTIFYICONDATA g_NotifyIconData;
HICON g_hTrayIcon = NULL; // ���ڴ洢������ͼ����

// ������Ϣ����
#define WM_TRAYICON (WM_USER + 1)
#define ID_TRAY_EXIT 1001
#define ID_TRAY_REFRESH 1002
#define ID_DPI_BASE 2000  // DPI�˵������ID
// ��ʾ�����ݽṹ�����¶����Ա�����ԭ�г�ͻ��
struct DisplayDataGUI {
    LUID m_adapterId;
    int m_targetID;
    int m_sourceID;
    std::wstring m_displayName;
    int m_currentDPI;

    DisplayDataGUI() {
        m_adapterId = {};
        m_targetID = m_sourceID = -1;
        m_currentDPI = 100;
    }
};

std::vector<DisplayDataGUI> g_displayDataCache;

// ˢ����ʾ������
void RefreshDisplayData() {
    g_displayDataCache.clear();
    std::vector<DISPLAYCONFIG_PATH_INFO> pathsV;
    std::vector<DISPLAYCONFIG_MODE_INFO> modesV;
    int flags = QDC_ONLY_ACTIVE_PATHS;

    if (!DpiHelper::GetPathsAndModes(pathsV, modesV, flags)) {
        return;
    }

    g_displayDataCache.resize(pathsV.size());
    int idx = 0;

    for (const auto& path : pathsV) {
        auto adapterLUID = path.targetInfo.adapterId;
        auto targetID = path.targetInfo.id;
        auto sourceID = path.sourceInfo.id;

        DISPLAYCONFIG_TARGET_DEVICE_NAME deviceName;
        deviceName.header.size = sizeof(deviceName);
        deviceName.header.type = DISPLAYCONFIG_DEVICE_INFO_GET_TARGET_NAME;
        deviceName.header.adapterId = adapterLUID;
        deviceName.header.id = targetID;

        if (ERROR_SUCCESS == DisplayConfigGetDeviceInfo(&deviceName.header)) {
            DisplayDataGUI dd = {};
            dd.m_adapterId = adapterLUID;
            dd.m_sourceID = sourceID;
            dd.m_targetID = targetID;

            // ������ʾ������
            dd.m_displayName = L"��ʾ�� " + std::to_wstring(idx + 1) + L": " + deviceName.monitorFriendlyDeviceName;
            if (DISPLAYCONFIG_OUTPUT_TECHNOLOGY_INTERNAL == deviceName.outputTechnology) {
                dd.m_displayName += L" (������ʾ��)";
            }

            // ��ȡ��ǰDPI
            auto dpiInfo = DpiHelper::GetDPIScalingInfo(adapterLUID, sourceID);
            dd.m_currentDPI = dpiInfo.current;

            g_displayDataCache[idx] = dd;
        }
        idx++;
    }
}

// �����Ҽ��˵�
HMENU CreateTrayMenu() {
    HMENU hMenu = CreatePopupMenu();
    HMENU hSubMenu;

    // Ϊÿ����ʾ�������Ӳ˵�
    for (size_t i = 0; i < g_displayDataCache.size(); i++) {
        hSubMenu = CreatePopupMenu();

        // ��ӵ�ǰDPI��Ϣ
        std::wstring currentInfo = L"��ǰ: " + std::to_wstring(g_displayDataCache[i].m_currentDPI) + L"%";
        AppendMenuW(hSubMenu, MF_STRING | MF_GRAYED, 0, currentInfo.c_str());
        AppendMenuW(hSubMenu, MF_SEPARATOR, 0, NULL);

        // ���DPIѡ�� - ʹ��ԭ�е�DpiVals����
        for (int j = 0; j < 12; j++) { // DpiVals������12��Ԫ��
            std::wstring dpiText = std::to_wstring(DpiVals[j]) + L"%";
            UINT flags = MF_STRING;
            if (DpiVals[j] == g_displayDataCache[i].m_currentDPI) {
                flags |= MF_CHECKED;
            }
            AppendMenuW(hSubMenu, flags, ID_DPI_BASE + i * 100 + j, dpiText.c_str());
        }

        // ���Ӳ˵���ӵ����˵�
        AppendMenuW(hMenu, MF_STRING | MF_POPUP, (UINT_PTR)hSubMenu, g_displayDataCache[i].m_displayName.c_str());
    }

    AppendMenuW(hMenu, MF_SEPARATOR, 0, NULL);
    AppendMenuW(hMenu, MF_STRING, ID_TRAY_REFRESH, L"ˢ����ʾ���б�");
    AppendMenuW(hMenu, MF_STRING, ID_TRAY_EXIT, L"�˳�");

    return hMenu;
}

// ����DPI����
void HandleDPIChange(int menuId) {
    int displayIndex = (menuId - ID_DPI_BASE) / 100;
    int dpiIndex = (menuId - ID_DPI_BASE) % 100;

    if (displayIndex >= 0 && displayIndex < g_displayDataCache.size() &&
        dpiIndex >= 0 && dpiIndex < 12) { // DpiVals�����СΪ12

        UINT32 newDPI = DpiVals[dpiIndex];
        bool success = DpiHelper::SetDPIScaling(
            g_displayDataCache[displayIndex].m_adapterId,
            g_displayDataCache[displayIndex].m_sourceID,
            newDPI
        );

        if (success) {
            // ���������ʾ��������0��������ע���
            if (displayIndex == 0) {
                HKEY hKey;
                LPCWSTR sKeyPath = L"Control Panel\\Desktop\\WindowMetrics\\";
                DWORD value = static_cast<DWORD>(int(newDPI * 0.96));
                if (RegOpenKeyEx(HKEY_CURRENT_USER, sKeyPath, NULL, KEY_ALL_ACCESS, &hKey) == ERROR_SUCCESS) {
                    RegSetValueEx(hKey, L"AppliedDPI", NULL, REG_DWORD, (const BYTE*)&value, sizeof(value));
                    RegCloseKey(hKey);
                }
            }

            std::wstring message = g_displayDataCache[displayIndex].m_displayName +
                L" �����ű���������Ϊ " + std::to_wstring(newDPI) + L"%";
            // MessageBoxW(NULL, message.c_str(), L"DPI���óɹ�", MB_ICONINFORMATION);

            // ˢ������
            RefreshDisplayData();
        }
        else {
            MessageBoxW(NULL, L"����DPIʧ��", L"����", MB_ICONERROR);
        }
    }
}

// ���ڹ���
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_TRAYICON:
        if (lParam == WM_RBUTTONUP) {
            POINT pt;
            GetCursorPos(&pt);

            HMENU hMenu = CreateTrayMenu();
            SetForegroundWindow(hWnd);

            int result = TrackPopupMenu(hMenu, TPM_RETURNCMD | TPM_RIGHTBUTTON,
                pt.x, pt.y, 0, hWnd, NULL);

            if (result == ID_TRAY_EXIT) {
                PostQuitMessage(0);
            }
            else if (result == ID_TRAY_REFRESH) {
                RefreshDisplayData();
                // MessageBoxW(NULL, L"��ʾ���б���ˢ��", L"��Ϣ", MB_ICONINFORMATION);
            }
            else if (result >= ID_DPI_BASE) {
                HandleDPIChange(result);
            }

            DestroyMenu(hMenu);
        }
        break;

    case WM_DESTROY:
        Shell_NotifyIcon(NIM_DELETE, &g_NotifyIconData);
        // ��������ͼ����Դ
        if (g_hTrayIcon) {
            DestroyIcon(g_hTrayIcon);
            g_hTrayIcon = NULL;
        }
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// ��������ͼ��
bool CreateTrayIcon() {
    g_NotifyIconData.cbSize = sizeof(NOTIFYICONDATA);
    g_NotifyIconData.hWnd = g_hWnd;
    g_NotifyIconData.uID = 1;
    g_NotifyIconData.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    g_NotifyIconData.uCallbackMessage = WM_TRAYICON;

    // ���Դ�Base64���ݴ���ͼ��
    g_hTrayIcon = CreateIconFromBase64(TRAY_ICON_BASE64);

    if (g_hTrayIcon != NULL) {
        g_NotifyIconData.hIcon = g_hTrayIcon;
    }
    else {
        // ���Base64ͼ�괴��ʧ�ܣ�ʹ��Ĭ��ͼ��
        g_NotifyIconData.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    }

    wcscpy_s(g_NotifyIconData.szTip, L"DPI���ŵ�������");

    return Shell_NotifyIcon(NIM_ADD, &g_NotifyIconData) != FALSE;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    g_hInstance = hInstance;

    // ע�ᴰ����
    WNDCLASS wc = {};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = L"SetDPITrayApp";
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION); // ʹ��Ĭ��ͼ��

    if (!RegisterClass(&wc)) {
        MessageBoxW(NULL, L"ע�ᴰ����ʧ��", L"����", MB_ICONERROR);
        return 1;
    }

    // �������ش���
    g_hWnd = CreateWindow(L"SetDPITrayApp", L"SetDPI Tray", WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
        NULL, NULL, hInstance, NULL);

    if (!g_hWnd) {
        MessageBoxW(NULL, L"��������ʧ��", L"����", MB_ICONERROR);
        return 1;
    }

    // ��������ͼ��
    if (!CreateTrayIcon()) {
        MessageBoxW(NULL, L"��������ͼ��ʧ��", L"����", MB_ICONERROR);
        return 1;
    }

    // ��ʼ����ʾ������
    RefreshDisplayData();

    // ��Ϣѭ��
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return (int)msg.wParam;
}
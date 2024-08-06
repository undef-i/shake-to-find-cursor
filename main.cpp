#include <windows.h>
#include "mouse_handler.hpp"
#include "cursor_utils.hpp"
#include <chrono>

HWND h_min_size_edit;
HWND h_max_size_edit;
HWND h_min_speed_edit;
HWND h_max_speed_edit;
HWND h_jitter_cnt_edit;
HWND h_reset_delay_edit;

int base_size = 32;
int max_size = 256;
double min_speed = 8000.0;
double max_speed = 20000.0;
int jitter_cnt_thresh = 3;
std::chrono::seconds reset_delay(2);

POINT last_pos;
int cur_size = base_size;
std::vector<POINT> path;
bool jitter_detected = false;
int jitter_cnt = 0;
auto last_jitter_time = std::chrono::steady_clock::now();
auto last_move_time = std::chrono::steady_clock::now();

const int WINDOW_WIDTH = 300;
const int WINDOW_HEIGHT = 300;

LRESULT CALLBACK wnd_proc(HWND hwnd, UINT msg, WPARAM w_param, LPARAM l_param) {
    switch (msg) {
    case WM_CREATE:
        CreateWindowW(L"static", L"Min Size:", WS_VISIBLE | WS_CHILD, 10, 10, 80, 20, hwnd, (HMENU)1, NULL, NULL);
        h_min_size_edit = CreateWindowW(L"edit", L"32", WS_VISIBLE | WS_CHILD | WS_BORDER, 100, 10, 100, 20, hwnd, (HMENU)2, NULL, NULL);

        CreateWindowW(L"static", L"Max Size:", WS_VISIBLE | WS_CHILD, 10, 40, 80, 20, hwnd, (HMENU)3, NULL, NULL);
        h_max_size_edit = CreateWindowW(L"edit", L"256", WS_VISIBLE | WS_CHILD | WS_BORDER, 100, 40, 100, 20, hwnd, (HMENU)4, NULL, NULL);

        CreateWindowW(L"static", L"Min Speed:", WS_VISIBLE | WS_CHILD, 10, 70, 80, 20, hwnd, (HMENU)5, NULL, NULL);
        h_min_speed_edit = CreateWindowW(L"edit", L"8000", WS_VISIBLE | WS_CHILD | WS_BORDER, 100, 70, 100, 20, hwnd, (HMENU)6, NULL, NULL);

        CreateWindowW(L"static", L"Max Speed:", WS_VISIBLE | WS_CHILD, 10, 100, 80, 20, hwnd, (HMENU)7, NULL, NULL);
        h_max_speed_edit = CreateWindowW(L"edit", L"20000", WS_VISIBLE | WS_CHILD | WS_BORDER, 100, 100, 100, 20, hwnd, (HMENU)8, NULL, NULL);

        CreateWindowW(L"static", L"Jitter Count:", WS_VISIBLE | WS_CHILD, 10, 130, 100, 20, hwnd, (HMENU)9, NULL, NULL);
        h_jitter_cnt_edit = CreateWindowW(L"edit", L"3", WS_VISIBLE | WS_CHILD | WS_BORDER, 120, 130, 80, 20, hwnd, (HMENU)10, NULL, NULL);

        CreateWindowW(L"static", L"Reset Delay:", WS_VISIBLE | WS_CHILD, 10, 160, 100, 20, hwnd, (HMENU)11, NULL, NULL);
        h_reset_delay_edit = CreateWindowW(L"edit", L"2", WS_VISIBLE | WS_CHILD | WS_BORDER, 120, 160, 80, 20, hwnd, (HMENU)12, NULL, NULL);

        CreateWindowW(L"button", L"Reset", WS_VISIBLE | WS_CHILD, 10, 200, 180, 30, hwnd, (HMENU)13, NULL, NULL);

        SetTimer(hwnd, 1, 10, NULL);
        break;

    case WM_COMMAND:
        if (LOWORD(w_param) == 13) {
            restore_default_cursors();
        }
        break;

    case WM_TIMER:
    {
        wchar_t buffer[256];

        GetWindowTextW(h_min_size_edit, buffer, 256);
        base_size = _wtoi(buffer);

        GetWindowTextW(h_max_size_edit, buffer, 256);
        max_size = _wtoi(buffer);

        GetWindowTextW(h_min_speed_edit, buffer, 256);
        min_speed = _wtof(buffer);

        GetWindowTextW(h_max_speed_edit, buffer, 256);
        max_speed = _wtof(buffer);

        GetWindowTextW(h_jitter_cnt_edit, buffer, 256);
        jitter_cnt_thresh = _wtoi(buffer);

        GetWindowTextW(h_reset_delay_edit, buffer, 256);
        reset_delay = std::chrono::seconds(_wtoi(buffer));

        mouse_move_handler(last_pos, cur_size, base_size, max_size, min_speed, max_speed, jitter_cnt_thresh, reset_delay, path, jitter_detected, jitter_cnt, last_jitter_time, last_move_time);
    }
    break;

    case WM_GETMINMAXINFO:
    {
        MINMAXINFO* min_max_info = (MINMAXINFO*)l_param;
        min_max_info->ptMinTrackSize.x = WINDOW_WIDTH;
        min_max_info->ptMinTrackSize.y = WINDOW_HEIGHT;
        min_max_info->ptMaxTrackSize.x = WINDOW_WIDTH;
        min_max_info->ptMaxTrackSize.y = WINDOW_HEIGHT;
    }
    break;

    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);

        RECT rect;
        GetClientRect(hwnd, &rect);
        HBRUSH hBrush = CreateSolidBrush(RGB(240, 240, 240));
        FillRect(hdc, &rect, hBrush);
        DeleteObject(hBrush);

        EndPaint(hwnd, &ps);
    }
    break;

    case WM_DESTROY:
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProcW(hwnd, msg, w_param, l_param);
    }
    return 0;
}

int WINAPI wWinMain(HINSTANCE h_instance, HINSTANCE h_prev_instance, PWSTR p_cmd_line, int n_cmd_show) {
    const wchar_t CLASS_NAME[] = L"SFCWindowClass";

    WNDCLASSW wc = {};
    wc.lpfnWndProc = wnd_proc;
    wc.hInstance = h_instance;
    wc.lpszClassName = CLASS_NAME;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);

    RegisterClassW(&wc);

    HWND hwnd = CreateWindowExW(
        WS_EX_CLIENTEDGE,
        CLASS_NAME,
        L"Shake to Find Cursor",
        WS_POPUP | WS_CAPTION | WS_SYSMENU,
        CW_USEDEFAULT, CW_USEDEFAULT, WINDOW_WIDTH, WINDOW_HEIGHT,
        NULL,
        NULL,
        h_instance,
        NULL
    );

    if (hwnd == NULL) {
        return 0;
    }

    ShowWindow(hwnd, n_cmd_show);
    UpdateWindow(hwnd);

    MSG msg = {};
    while (GetMessageW(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    return 0;
}

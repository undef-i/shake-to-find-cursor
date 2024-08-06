#include "cursor_utils.hpp"

std::map<std::wstring, DWORD> cur_type_map = {
    {L"Arrow", 32512},
    {L"IBeam", 32513},
    {L"Wait", 32514},
    {L"Cross", 32515},  
    {L"UpArrow", 32516},
    {L"SizeWE", 32640},
    {L"SizeNS", 32641},
    {L"SizeNWSE", 32642},
    {L"SizeNESW", 32643},
    {L"SizeAll", 32644},
    {L"No", 32645},
    {L"Hand", 32649},
    {L"AppStarting", 32650},
    {L"Help", 32651},
    {L"Pen", 32631},
    {L"Icon", 32629},
};

std::wstring expand_env_vars(const std::wstring& path) {
    DWORD buffer_size = GetEnvironmentVariableW(L"SystemRoot", NULL, 0) + path.size() + 1;
    std::wstring expanded_path(buffer_size, L'\0');

    if (ExpandEnvironmentStringsW(path.c_str(), &expanded_path[0], buffer_size) == 0) {
        return L"";
    }

    expanded_path.resize(wcslen(expanded_path.c_str()));
    return expanded_path;
}

std::wstring get_cur_path_from_reg(const std::wstring& cur_name) {
    HKEY h_key;
    LONG result = RegOpenKeyExW(HKEY_CURRENT_USER, L"Control Panel\\Cursors", 0, KEY_READ, &h_key);
    if (result != ERROR_SUCCESS) {
        return L"";
    }

    WCHAR value_data[MAX_PATH];
    DWORD value_size = sizeof(value_data);
    result = RegQueryValueExW(h_key, cur_name.c_str(), NULL, NULL, (LPBYTE)value_data, &value_size);
    RegCloseKey(h_key);

    if (result != ERROR_SUCCESS) {
        return L"";
    }

    return expand_env_vars(value_data);
}

void update_cur_size(int size) {
    for (const auto& [cur_name, cur_id] : cur_type_map) {
        std::wstring cur_path = get_cur_path_from_reg(cur_name);
        if (cur_path.empty()) {
            continue;
        }

        HCURSOR h_cur = (HCURSOR)LoadImageW(NULL, cur_path.c_str(), IMAGE_CURSOR, size, size, LR_LOADFROMFILE);
        if (h_cur == NULL) {
            continue;
        }

        if (!SetSystemCursor(h_cur, cur_id)) {
            continue;
        }
    }
}

void restore_default_cursors() {
    for (const auto& [cur_name, cur_id] : cur_type_map) {
        SystemParametersInfoW(SPI_SETCURSORS, 0, NULL, 0);
    }
}

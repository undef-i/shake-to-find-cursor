#ifndef CURSOR_UTILS_HPP
#define CURSOR_UTILS_HPP

#include <windows.h>
#include <map>
#include <string>

extern std::map<std::wstring, DWORD> cur_type_map;

std::wstring expand_env_vars(const std::wstring& path);
std::wstring get_cur_path_from_reg(const std::wstring& cur_name);
void update_cur_size(int size);
void restore_default_cursors();

#endif

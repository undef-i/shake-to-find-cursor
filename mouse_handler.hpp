#pragma once
#include <windows.h>
#include <vector>
#include <chrono>

double calc_dist(POINT p1, POINT p2);
double calc_cos_angle(POINT p1, POINT p2, POINT p3);
int smooth_trans(int cur_size, int tgt_size, double factor);
void mouse_move_handler(POINT& last_pos, int& cur_size, int base_size, int max_size, double min_speed, double max_speed, int jitter_cnt_thresh, std::chrono::seconds reset_delay, std::vector<POINT>& path, bool& jitter_detected, int& jitter_cnt, std::chrono::steady_clock::time_point& last_jitter_time, std::chrono::steady_clock::time_point& last_move_time);

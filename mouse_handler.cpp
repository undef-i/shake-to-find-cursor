#include "mouse_handler.hpp"
#include "cursor_utils.hpp"
#include <cmath>

double calc_dist(POINT p1, POINT p2) {
    return sqrt(pow(p2.x - p1.x, 2) + pow(p2.y - p1.y, 2));
}

double calc_cos_angle(POINT p1, POINT p2, POINT p3) {
    double a = calc_dist(p2, p3);
    double b = calc_dist(p1, p3);
    double c = calc_dist(p1, p2);

    double cos_angle = (pow(b, 2) + pow(c, 2) - pow(a, 2)) / (2 * b * c);
    return cos_angle;
}

int smooth_trans(int cur_size, int tgt_size, double factor) {
    if (cur_size < tgt_size) {
        return static_cast<int>(cur_size + (tgt_size - cur_size) * factor);
    }
    else if (cur_size > tgt_size) {
        return static_cast<int>(cur_size - (cur_size - tgt_size) * factor);
    }
    return cur_size;
}

void mouse_move_handler(POINT& last_pos, int& cur_size, int base_size, int max_size, double min_speed, double max_speed, int jitter_cnt_thresh, std::chrono::seconds reset_delay, std::vector<POINT>& path, bool& jitter_detected, int& jitter_cnt, std::chrono::steady_clock::time_point& last_jitter_time, std::chrono::steady_clock::time_point& last_move_time) {
    POINT cur_pos;
    GetCursorPos(&cur_pos);
    auto cur_time = std::chrono::steady_clock::now();
    double dist = calc_dist(last_pos, cur_pos);
    std::chrono::duration<double> elapsed = cur_time - last_move_time;
    last_move_time = cur_time;

    path.push_back(cur_pos);
    if (path.size() > 3) {
        path.erase(path.begin());
    }

    if (path.size() == 3) {
        double cos_angle = calc_cos_angle(path[0], path[1], path[2]);
        if (cos_angle <= 0.0) {
            jitter_cnt++;
            last_jitter_time = cur_time;
            if (jitter_cnt >= jitter_cnt_thresh) {
                jitter_detected = true;
            }
        }
    }

    if (jitter_detected) {
        double speed = dist / elapsed.count();

        int tgt_size;
        if (speed < min_speed) {
            tgt_size = base_size;
        }
        else if (speed > max_speed) {
            tgt_size = max_size;
        }
        else {
            double speed_factor = (speed - min_speed) / (max_speed - min_speed);
            tgt_size = base_size + static_cast<int>((max_size - base_size) * speed_factor);
        }

        cur_size = smooth_trans(cur_size, tgt_size, 0.2);
        cur_size = (cur_size < base_size) ? base_size : ((cur_size > max_size) ? max_size : cur_size);

        update_cur_size(cur_size);
    }

    if (cur_time - last_jitter_time > reset_delay) {
        jitter_cnt = 0;
        jitter_detected = false;
    }

    last_pos = cur_pos;
}

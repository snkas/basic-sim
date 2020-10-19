/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2020 ETH Zurich
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Simon
 */

#ifndef LOG_UPDATE_HELPER_H
#define LOG_UPDATE_HELPER_H

#include <map>
#include <vector>
#include <iostream>
#include <fstream>
#include <string>
#include <ctime>
#include <iostream>
#include <fstream>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <chrono>
#include <stdexcept>

class LogUpdateHelper {

public:
    LogUpdateHelper();
    LogUpdateHelper(bool save_in_memory, bool save_to_file, std::string save_filename, std::string file_line_prefix);
    void Update(int64_t time, int64_t value);
    const std::vector<std::tuple<int64_t, int64_t, int64_t>>& Finalize(int64_t time);

private:
    std::vector<std::tuple<int64_t, int64_t, int64_t>> m_log_time_value;
    int64_t m_last_update_value;
    int64_t m_interval_alpha_value;
    int64_t m_last_interval_check_time;
    int64_t m_interval_alpha_left_time;
    int64_t m_interval_beta_left_time;

    bool m_save_in_memory;
    bool m_save_to_file;
    std::string m_save_filename;
    std::string m_file_line_prefix;
    std::fstream m_save_file_stream;
};

#endif // LOG_UPDATE_HELPER_H

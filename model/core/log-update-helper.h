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

#include <vector>
#include <tuple>
#include <fstream>
#include <string>
#include <stdexcept>

template <class V>
class LogUpdateHelper {

public:
    LogUpdateHelper();
    LogUpdateHelper(bool save_in_memory, bool save_to_file, std::string save_filename, std::string file_line_prefix);
    void Update(int64_t time, V value);
    const std::vector<std::tuple<int64_t, int64_t, V>>& Finalize(int64_t time);

private:
    std::vector<std::tuple<int64_t, int64_t, V>> m_log_time_value;
    V m_last_update_value;
    V m_interval_alpha_value;
    int64_t m_last_interval_check_time;
    int64_t m_interval_alpha_left_time;
    int64_t m_interval_beta_left_time;

    bool m_save_in_memory;
    bool m_save_to_file;
    std::string m_save_filename;
    std::string m_file_line_prefix;
    std::fstream m_save_file_stream;
};

template <class V>
LogUpdateHelper<V>::LogUpdateHelper(bool save_in_memory, bool save_to_file, std::string save_filename, std::string file_line_prefix) {
    m_last_update_value = -1;
    m_interval_alpha_value = -1;
    m_last_interval_check_time = -1;
    m_interval_alpha_left_time = -1;
    m_interval_beta_left_time = -1;
    if (!save_in_memory && !save_to_file) {
        throw std::invalid_argument("Log updater helper needs at least either to save to memory or to file.");
    }
    m_save_in_memory = save_in_memory;
    m_save_to_file = save_to_file;
    m_save_filename = save_filename;
    m_file_line_prefix = file_line_prefix;
    if (m_save_to_file) {
        m_save_file_stream.open(m_save_filename, std::fstream::out);
    }
}

template <class V>
LogUpdateHelper<V>::LogUpdateHelper() : LogUpdateHelper(true, false, "", "") {
    // Left empty intentionally
}

template <class V>
void LogUpdateHelper<V>::Update(int64_t time, V value) {

    // Time must be positive
    if (time < 0) {
        throw std::invalid_argument("Negative time is not permitted.");
    }

    // No logging in the past
    if (time < m_last_interval_check_time) {
        throw std::invalid_argument("Cannot log a value for a time in the past.");
    }

    // Check if there is a new interval once per time moment
    if (time > m_last_interval_check_time) {

        // If no first interval is yet started, start interval alpha
        if (m_interval_alpha_left_time == -1) {
            m_interval_alpha_left_time = time;
        } else {

            // If no second interval is yet started, start interval beta
            if (m_interval_beta_left_time == -1) {
                m_interval_beta_left_time = time;
                m_interval_alpha_value = m_last_update_value;

                // If both interval alpha and beta has been started, the value of interval beta
                // is determined, as such interval alpha can be pushed to the log if it is different
            } else {

                // The value of interval beta has now been determined
                V interval_beta_value = m_last_update_value;

                // If interval alpha and beta have the same value, merge
                if (m_interval_alpha_value == interval_beta_value) {
                    m_interval_beta_left_time = time;

                } else { // If not, save

                    // Save the alpha interval
                    if (m_save_in_memory) {
                        m_log_time_value.push_back(std::make_tuple(m_interval_alpha_left_time, m_interval_beta_left_time, m_interval_alpha_value));
                    }
                    if (m_save_to_file) {
                        m_save_file_stream << m_file_line_prefix << m_interval_alpha_left_time << "," << m_interval_alpha_value << std::endl;
                    }

                    // And make beta interval become alpha
                    m_interval_alpha_left_time = m_interval_beta_left_time;
                    m_interval_alpha_value = interval_beta_value;
                    m_interval_beta_left_time = time;

                }

            }

        }

        // Check once per time moment for new intervals
        m_last_interval_check_time = time;

    }

    // Continuously update it
    m_last_update_value = value;

}

template <class V>
const std::vector<std::tuple<int64_t, int64_t, V>>& LogUpdateHelper<V>::Finalize(int64_t time) {

    // Time must be positive
    if (time < 0) {
        throw std::invalid_argument("Negative time is not permitted.");
    }

    // No logging in the past
    if (time < m_last_interval_check_time) {
        throw std::invalid_argument("Cannot finalize at a time in the past.");
    }

    // Only have for one time moment, as such only alpha
    if (m_interval_alpha_left_time != -1 && m_interval_beta_left_time == -1) {
        if (m_interval_alpha_left_time != time) {
            if (m_save_in_memory) {
                m_log_time_value.push_back(std::make_tuple(m_interval_alpha_left_time, time, m_last_update_value));
            }
            if (m_save_to_file) {
                m_save_file_stream << m_file_line_prefix << m_interval_alpha_left_time << "," << m_last_update_value << std::endl;
                m_save_file_stream << m_file_line_prefix << time << "," << m_last_update_value << std::endl;
            }
        }

        // If there is a beta interval, save the (guaranteed non-empty) alpha interval and the beta interval
    } else if (m_interval_alpha_left_time != -1 && m_interval_beta_left_time != -1) {

        // If the last two intervals can be merged
        if (m_interval_alpha_value == m_last_update_value) {
            if (m_save_in_memory) {
                m_log_time_value.push_back(std::make_tuple(m_interval_alpha_left_time, time, m_interval_alpha_value));
            }
            if (m_save_to_file) {
                m_save_file_stream << m_file_line_prefix << m_interval_alpha_left_time << "," << m_interval_alpha_value << std::endl;
                m_save_file_stream << m_file_line_prefix << time << "," << m_interval_alpha_value << std::endl;
            }

            // If the last two intervals have different values, save separately
        } else {
            if (m_save_in_memory) {
                m_log_time_value.push_back(std::make_tuple(m_interval_alpha_left_time, m_interval_beta_left_time, m_interval_alpha_value));
            }
            if (m_save_to_file) {
                m_save_file_stream << m_file_line_prefix << m_interval_alpha_left_time << "," << m_interval_alpha_value << std::endl;
            }
            if (m_interval_beta_left_time != time) {
                if (m_save_in_memory) {
                    m_log_time_value.push_back(std::make_tuple(m_interval_beta_left_time, time, m_last_update_value));
                }
                if (m_save_to_file) {
                    m_save_file_stream << m_file_line_prefix << m_interval_beta_left_time << "," << m_last_update_value << std::endl;
                    m_save_file_stream << m_file_line_prefix << time << "," << m_last_update_value << std::endl;
                }
            } else {
                if (m_save_to_file) {
                    m_save_file_stream << m_file_line_prefix << m_interval_beta_left_time << "," << m_interval_alpha_value << std::endl;
                }
            }
        }

    }

    // Close to save file stream if it was opened
    if (m_save_to_file) {
        m_save_file_stream.close();
    }

    // Return final resulting log, which will be empty if save in memory is not enabled
    return m_log_time_value;
}

#endif // LOG_UPDATE_HELPER_H

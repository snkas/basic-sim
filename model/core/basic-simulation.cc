/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "basic-simulation.h"

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (BasicSimulation);
TypeId BasicSimulation::GetTypeId (void)
{
    static TypeId tid = TypeId ("ns3::BasicSimulation")
            .SetParent<Object> ()
            .SetGroupName("BasicSim")
    ;
    return tid;
}

BasicSimulation::BasicSimulation(std::string run_dir) {
    m_run_dir = run_dir;
    RegisterTimestamp("Start");
    ConfigureRunDirectory();
    WriteFinished(false);
    ReadConfig();
    ConfigureSimulation();
}

int64_t BasicSimulation::NowNsSinceEpoch() {
    return std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}

void BasicSimulation::RegisterTimestamp(std::string label) {
    m_timestamps.push_back(std::make_pair(label, NowNsSinceEpoch()));
}

void BasicSimulation::ConfigureRunDirectory() {
    std::cout << "CONFIGURE RUN DIRECTORY" << std::endl;

    // Run directory
    if (!dir_exists(m_run_dir)) {
        throw std::runtime_error(format_string("Run directory \"%s\" does not exist.", m_run_dir.c_str()));
    } else {
        printf("  > Run directory: %s\n", m_run_dir.c_str());
    }

    // Logs in run directory
    m_logs_dir = m_run_dir + "/logs_ns3";
    if (dir_exists(m_logs_dir)) {
        printf("  > Emptying existing logs directory\n");
        remove_file_if_exists(m_logs_dir + "/finished.txt");
        remove_file_if_exists(m_logs_dir + "/timing_results.txt");
    } else {
        mkdir_if_not_exists(m_logs_dir);
    }
    printf("  > Logs directory: %s\n", m_logs_dir.c_str());
    printf("\n");

    RegisterTimestamp("Configure run directory");
}

void BasicSimulation::WriteFinished(bool finished) {
    std::ofstream fileFinishedEnd(m_logs_dir + "/finished.txt");
    fileFinishedEnd << (finished ? "Yes" : "No") << std::endl;
    fileFinishedEnd.close();
}

void BasicSimulation::ReadConfig() {

    // Read the config
    m_config = read_config(m_run_dir + "/config_ns3.properties");

    // Print full config
    printf("CONFIGURATION\n-----\nKEY                                       VALUE\n");
    std::map<std::string, std::string>::iterator it;
    for ( it = m_config.begin(); it != m_config.end(); it++ ) {
        printf("%-40s  %s\n", it->first.c_str(), it->second.c_str());
    }
    printf("\n");

    // End time
    m_simulation_end_time_ns = parse_positive_int64(GetConfigParamOrFail("simulation_end_time_ns"));

    // Seed
    m_simulation_seed = parse_positive_int64(GetConfigParamOrFail("simulation_seed"));

}

void BasicSimulation::ConfigureSimulation() {
    std::cout << "CONFIGURE SIMULATION" << std::endl;

    // Set primary seed
    ns3::RngSeedManager::SetSeed(m_simulation_seed);
    std::cout << "  > Seed: " << m_simulation_seed << std::endl;

    // Set end time
    Simulator::Stop(NanoSeconds(m_simulation_end_time_ns));
    printf("  > Duration: %.2f s (%" PRId64 " ns)\n", m_simulation_end_time_ns / 1e9, m_simulation_end_time_ns);

    std::cout << std::endl;
    RegisterTimestamp("Configure simulator");
}

void BasicSimulation::ShowSimulationProgress() {
    int64_t now = NowNsSinceEpoch();
    if (now - m_last_log_time_ns_since_epoch > m_progress_interval_ns) {
        printf(
                "%5.2f%% - Simulation Time = %.2f s ::: Wallclock Time = %.2f s\n",
                (Simulator::Now().GetSeconds() / (m_simulation_end_time_ns / 1e9)) * 100.0,
                Simulator::Now().GetSeconds(),
                (now - m_sim_start_time_ns_since_epoch) / 1e9
        );
        if (m_counter_progress_updates < 8 || m_counter_progress_updates % 5 == 0) { // The first 8 and every 5 progress updates we show estimate
            int remaining_s = (int) ((m_simulation_end_time_ns / 1e9 - Simulator::Now().GetSeconds()) / (Simulator::Now().GetSeconds() / ((now - m_sim_start_time_ns_since_epoch) / 1e9)));
            int seconds = remaining_s % 60;
            int minutes = ((remaining_s - seconds) / 60) % 60;
            int hours = (remaining_s - seconds - minutes * 60) / 3600;
            std::string remainder;
            if (hours > 0) {
                remainder = format_string("Estimated wallclock time remaining: %d hours %d minutes", hours, minutes);
            } else if (minutes > 0) {
                remainder = format_string("Estimated wallclock time remaining: %d minutes %d seconds", minutes, seconds);
            } else {
                remainder = format_string("Estimated wallclock time remaining: %d seconds", seconds);
            }
            std::cout << remainder << std::endl;
        }
        m_last_log_time_ns_since_epoch = now;
        if (m_counter_progress_updates < 4) {
            m_progress_interval_ns = 10000000000; // The first five are every 10s
        } else if (m_counter_progress_updates < 19) {
            m_progress_interval_ns = 20000000000; // The next 15 are every 20s
        } else if (m_counter_progress_updates < 99) {
            m_progress_interval_ns = 60000000000; // The next 80 are every 60s
        } else {
            m_progress_interval_ns = 360000000000; // After that, it is every 360s = 5 minutes
        }
        m_counter_progress_updates++;
    }
    Simulator::Schedule(Seconds(m_simulation_event_interval_s), &BasicSimulation::ShowSimulationProgress, this);
}

void BasicSimulation::ConfirmAllConfigParamKeysRequested() {
    for (const std::pair<std::string, std::string>& key_val : m_config) {
        if (m_configRequestedKeys.find(key_val.first) == m_configRequestedKeys.end()) {
            throw std::runtime_error(format_string("Config key \'%s\' has not been requested (unused config keys are not allowed)", key_val.first.c_str()));
        }
    }
}

void BasicSimulation::Run() {
    std::cout << "SIMULATION" << std::endl;

    // Before it starts to run, we need to have processed all the config
    ConfirmAllConfigParamKeysRequested();

    // Schedule progress printing
    m_sim_start_time_ns_since_epoch = NowNsSinceEpoch();
    m_last_log_time_ns_since_epoch = m_sim_start_time_ns_since_epoch;
    Simulator::Schedule(Seconds(m_simulation_event_interval_s), &BasicSimulation::ShowSimulationProgress, this);

    // Run
    printf("Running the simulation for %.2f simulation seconds...\n", (m_simulation_end_time_ns / 1e9));
    Simulator::Run();
    printf("Finished simulation.\n");

    // Print final duration
    printf(
            "Simulation of %.1f seconds took in wallclock time %.1f seconds.\n\n",
            m_simulation_end_time_ns / 1e9,
            (NowNsSinceEpoch() - m_sim_start_time_ns_since_epoch) / 1e9
    );

    RegisterTimestamp("Run simulation");
}

void BasicSimulation::CleanUpSimulation() {
    std::cout << "CLEAN-UP" << std::endl;
    Simulator::Destroy();
    std::cout << "  > Simulator is destroyed" << std::endl;
    std::cout << std::endl;
    RegisterTimestamp("Destroy simulator");
}

void BasicSimulation::StoreTimingResults() {
    std::cout << "TIMING RESULTS" << std::endl;
    std::cout << "------" << std::endl;

    // Write to both file and out
    std::ofstream fileTimingResults(m_logs_dir + "/timing_results.txt");
    int64_t t_prev = -1;
    for (std::pair <std::string, int64_t> &ts : m_timestamps) {
        if (t_prev == -1) {
            t_prev = ts.second;
        } else {
            std::string line = format_string(
                    "[%7.1f - %7.1f] (%.1f s) :: %s",
                    (t_prev - m_timestamps[0].second) / 1e9,
                    (ts.second - m_timestamps[0].second) / 1e9,
                    (ts.second - t_prev) / 1e9,
                    ts.first.c_str()
            );
            std::cout << line << std::endl;
            fileTimingResults << line << std::endl;
            t_prev = ts.second;
        }
    }
    fileTimingResults.close();

    std::cout << std::endl;
}

void BasicSimulation::Finalize() {
    CleanUpSimulation();
    StoreTimingResults();
    WriteFinished(true);
}

int64_t BasicSimulation::GetSimulationEndTimeNs() {
    return m_simulation_end_time_ns;
}

std::string BasicSimulation::GetConfigParamOrFail(std::string key) {
    m_configRequestedKeys.insert(key);
    return get_param_or_fail(key, m_config);
}

std::string BasicSimulation::GetConfigParamOrDefault(std::string key, std::string default_value) {
    m_configRequestedKeys.insert(key);
    return get_param_or_default(key, default_value, m_config);
}

std::string BasicSimulation::GetLogsDir() {
    return m_logs_dir;
}

std::string BasicSimulation::GetRunDir() {
    return m_run_dir;
}

}

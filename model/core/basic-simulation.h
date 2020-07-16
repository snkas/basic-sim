/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#ifndef BASIC_SIMULATION_H
#define BASIC_SIMULATION_H

#include <map>
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

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/random-variable-stream.h"
#include "ns3/command-line.h"
#include "ns3/traffic-control-helper.h"

#include "ns3/exp-util.h"

namespace ns3 {

class BasicSimulation : public Object
{

public:

    // Primary
    static TypeId GetTypeId (void);
    BasicSimulation(std::string run_dir);
    void Run();
    void Finalize();

    // Timestamps to track performance
    void RegisterTimestamp(std::string label);

    // Getters
    int64_t GetSimulationEndTimeNs();
    std::string GetConfigParamOrFail(std::string key);
    std::string GetConfigParamOrDefault(std::string key, std::string default_value);
    std::string GetLogsDir();
    std::string GetRunDir();

private:

    // Internal setup
    void ConfigureRunDirectory();
    void WriteFinished(bool finished);
    void ReadConfig();
    void ConfigureSimulation();
    void ShowSimulationProgress();
    void RunSimulation();
    void CleanUpSimulation();
    void ConfirmAllConfigParamKeysRequested();
    void StoreTimingResults();

    // Timestamp to identify which parts take long
    int64_t NowNsSinceEpoch();
    std::vector<std::pair<std::string, int64_t>> m_timestamps; // List of all important events happening in the pipeline

    // Run directory
    std::string m_run_dir;
    std::string m_logs_dir;

    // Config variables
    std::map<std::string, std::string> m_config;
    std::set<std::string> m_configRequestedKeys;
    int64_t m_simulation_seed;
    int64_t m_simulation_end_time_ns;

    // Progress show variables
    int64_t m_sim_start_time_ns_since_epoch;
    int64_t m_last_log_time_ns_since_epoch;
    int m_counter_progress_updates = 0;
    double m_progress_interval_ns = 10000000000; // First one after 10s
    double m_simulation_event_interval_s = 0.00001;

};

}

#endif /* BASIC_SIMULATION_H */

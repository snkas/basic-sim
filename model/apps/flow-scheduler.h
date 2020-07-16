/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
#ifndef FLOW_SCHEDULER_H
#define FLOW_SCHEDULER_H

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

#include "ns3/basic-simulation.h"
#include "ns3/exp-util.h"
#include "ns3/topology.h"

#include "ns3/schedule-reader.h"
#include "ns3/flow-send-helper.h"
#include "ns3/flow-send-application.h"
#include "ns3/flow-sink-helper.h"
#include "ns3/flow-sink.h"

namespace ns3 {

class FlowScheduler
{

public:
    FlowScheduler(Ptr<BasicSimulation> basicSimulation, Ptr<Topology> topology);
    void Schedule();
    void WriteResults();

protected:
    void StartNextFlow(int i);
    Ptr<BasicSimulation> m_basicSimulation;
    int64_t m_simulation_end_time_ns;
    Ptr<Topology> m_topology = nullptr;
    std::vector<schedule_entry_t> m_schedule;
    NodeContainer m_nodes;
    std::vector<ApplicationContainer> m_apps;
    std::set<int64_t> m_enableFlowLoggingToFileForFlowIds;

};

}

#endif /* FLOW_SCHEDULER_H */

/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#ifndef TEST_CASE_WITH_LOG_VALIDATORS_H
#define TEST_CASE_WITH_LOG_VALIDATORS_H

#include "ns3/basic-sim-module.h"
#include "test-helpers.h"

using namespace ns3;

class TestCaseWithLogValidators : public TestCase
{
public:
    TestCaseWithLogValidators (std::string s);

    void prepare_clean_run_dir(std::string run_dir);

    void validate_finished(std::string run_dir);

    void validate_link_net_device_queue_logs(
            std::string run_dir,
            std::vector<std::pair<int64_t, int64_t>> dir_a_b_list,
            std::map<std::pair<int64_t, int64_t>, std::vector<std::tuple<int64_t, int64_t, int64_t>>>& link_net_device_queue_pkt,
            std::map<std::pair<int64_t, int64_t>, std::vector<std::tuple<int64_t, int64_t, int64_t>>>& link_net_device_queue_byte
    );

    void validate_link_net_device_utilization_logs(
            std::string run_dir,
            std::vector<std::pair<int64_t, int64_t>> dir_a_b_list,
            int64_t duration_ns,
            int64_t link_net_device_utilization_tracking_interval_ns,
            std::map<std::pair<int64_t, int64_t>, std::vector<std::tuple<int64_t, int64_t, int64_t>>>& link_net_device_utilization,
            std::map<std::pair<int64_t, int64_t>, double>& link_overall_utilization_as_fraction
    );

    void validate_link_interface_tc_qdisc_queue_logs(
            std::string run_dir,
            std::vector<std::pair<int64_t, int64_t>> dir_a_b_list,
            std::map<std::pair<int64_t, int64_t>, std::vector<std::tuple<int64_t, int64_t, int64_t>>>& link_interface_tc_qdisc_queue_pkt,
            std::map<std::pair<int64_t, int64_t>, std::vector<std::tuple<int64_t, int64_t, int64_t>>>& link_interface_tc_qdisc_queue_byte
    );

    void validate_tcp_flow_logs(
            int64_t simulation_end_time_ns, 
            std::string run_dir, 
            std::vector<TcpFlowScheduleEntry> tcp_flow_schedule,
            std::set<int64_t> tcp_flow_ids_with_logging,
            std::vector<int64_t>& end_time_ns_list, 
            std::vector<int64_t>& sent_byte_list, 
            std::vector<std::string>& finished_list
    );

    void validate_udp_burst_logs(
            int64_t simulation_end_time_ns,
            std::string run_dir,
            std::vector<UdpBurstInfo> udp_burst_schedule,
            std::set<int64_t> udp_burst_ids_with_logging,
            std::vector<double>& list_outgoing_rate_megabit_per_s,
            std::vector<double>& list_incoming_rate_megabit_per_s
    );

    void validate_udp_ping_logs(
            int64_t simulation_end_time_ns,
            std::string run_dir,
            std::vector<UdpPingInfo> udp_ping_schedule,
            std::vector<std::vector<int64_t>>& list_latency_there_ns,
            std::vector<std::vector<int64_t>>& list_latency_back_ns,
            std::vector<std::vector<int64_t>>& list_rtt_ns
    );
    
};

#endif //TEST_CASE_WITH_LOG_VALIDATORS_H

/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

////////////////////////////////////////////////////////////////////////////////////////

class PtopReceiveErrorModelTestCase : public TestCaseWithLogValidators
{
public:
    PtopReceiveErrorModelTestCase (std::string s) : TestCaseWithLogValidators (s) {};
    std::string test_run_dir = ".tmp-test-ptop-receive-error-model";

    void prepare_ptop_receive_error_model_test_config() {
        std::ofstream config_file(test_run_dir + "/config_ns3.properties");
        config_file << "simulation_end_time_ns=10000000000" << std::endl;
        config_file << "simulation_seed=123456789" << std::endl;
        config_file << "topology_ptop_filename=\"topology.properties\"" << std::endl;
        config_file.close();
    }

    void cleanup_ptop_receive_error_model_test() {
        remove_file_if_exists(test_run_dir + "/config_ns3.properties");
        remove_file_if_exists(test_run_dir + "/topology.properties");
        remove_file_if_exists(test_run_dir + "/logs_ns3/finished.txt");
        remove_file_if_exists(test_run_dir + "/logs_ns3/timing_results.txt");
        remove_file_if_exists(test_run_dir + "/logs_ns3/timing_results.csv");
        remove_dir_if_exists(test_run_dir + "/logs_ns3");
        remove_dir_if_exists(test_run_dir);
    }

};

////////////////////////////////////////////////////////////////////////////////////////

class PtopReceiveErrorModelValidTestCase : public PtopReceiveErrorModelTestCase
{
public:
    PtopReceiveErrorModelValidTestCase () : PtopReceiveErrorModelTestCase ("ptop-receive-error-model valid") {};

    void DoRun () {
        test_run_dir = ".tmp-test-ptop-receive-error-model-valid";
        prepare_clean_run_dir(test_run_dir);
        prepare_ptop_receive_error_model_test_config();
        
        std::ofstream topology_file;
        topology_file.open (test_run_dir + "/topology.properties");
        topology_file << "num_nodes=8" << std::endl;
        topology_file << "num_undirected_edges=7" << std::endl;
        topology_file << "switches=set(4)" << std::endl;
        topology_file << "switches_which_are_tors=set(4)" << std::endl;
        topology_file << "servers=set(0,1,2,3,5,6,7)" << std::endl;
        topology_file << "undirected_edges=set(0-4,1-4,2-4,3-4,4-5,4-6,4-7)" << std::endl;
        topology_file << "all_nodes_are_endpoints=false" << std::endl;
        topology_file << "link_channel_delay_ns=10000" << std::endl;
        topology_file << "link_net_device_data_rate_megabit_per_s=100.0" << std::endl;
        topology_file << "link_net_device_queue=drop_tail(100p)" << std::endl;
        topology_file << "link_net_device_receive_error_model=map(0->4: iid_uniform_random_pkt(0.0), 1->4: iid_uniform_random_pkt(0.1), 2->4: iid_uniform_random_pkt(0.2), 3->4: iid_uniform_random_pkt(0.3), 5->4:iid_uniform_random_pkt(0.5), 6->4:iid_uniform_random_pkt(0.6), 7->4:iid_uniform_random_pkt(0.7), 4->0: iid_uniform_random_pkt(0.4), 4->1: none, 4->2: iid_uniform_random_pkt(0.4), 4->3: none, 4->5: none, 4->6: iid_uniform_random_pkt(0.400000), 4->7: iid_uniform_random_pkt(1.0))" << std::endl;
        topology_file << "link_interface_traffic_control_qdisc=disabled" << std::endl;
        topology_file.close();
        
        Ptr<BasicSimulation> basicSimulation = CreateObject<BasicSimulation>(test_run_dir);
        Ptr<TopologyPtop> topology = CreateObject<TopologyPtop>(basicSimulation, Ipv4ArbiterRoutingHelper());

        // And now we are going to go test all the network devices installed and their channels in-between
        for (const std::pair<int64_t, int64_t>& edge : topology->GetUndirectedEdges()) {
            Ptr<PointToPointNetDevice> deviceAtoB = topology->GetSendingNetDeviceForLink(edge);
            Ptr<PointToPointNetDevice> deviceBtoA = topology->GetSendingNetDeviceForLink(std::make_pair(edge.second, edge.first));
            std::vector<std::pair<std::pair<int64_t, int64_t>, Ptr<PointToPointNetDevice>>> links_with_devices;
            links_with_devices.push_back(std::make_pair(edge, deviceAtoB));
            links_with_devices.push_back(std::make_pair(std::make_pair(edge.second, edge.first), deviceBtoA));
            for (std::pair<std::pair<int64_t, int64_t>, Ptr<PointToPointNetDevice>> link_and_device : links_with_devices) {

                // Under investigation
                std::pair<int64_t, int64_t> link = link_and_device.first;
                Ptr<PointToPointNetDevice> device = link_and_device.second;

                // Receive error model
                PointerValue ptr2;
                device->GetAttribute("ReceiveErrorModel", ptr2);
                Ptr<ErrorModel> errorModel = ptr2.Get<ErrorModel>();
                if (link.first == 7 && link.second == 4) {
                    Ptr<RateErrorModel> rateErrorModel = errorModel->GetObject<RateErrorModel>();
                    ASSERT_EQUAL(rateErrorModel->GetRate(), 1.0);
                    ASSERT_EQUAL(rateErrorModel->GetUnit(), RateErrorModel::ErrorUnit::ERROR_UNIT_PACKET);
                } else {
                    if (link.first % 2 == 0) {
                        Ptr <RateErrorModel> rateErrorModel = errorModel->GetObject<RateErrorModel>();
                        ASSERT_EQUAL_APPROX(rateErrorModel->GetRate(), link.second * 0.1, 0.000000001);
                        ASSERT_EQUAL(rateErrorModel->GetUnit(), RateErrorModel::ErrorUnit::ERROR_UNIT_PACKET);
                    } else {
                        ASSERT_EQUAL(errorModel, 0);
                    }
                }

                // Check if the node identifiers here and on the other side match up
                ASSERT_EQUAL(device->GetNode()->GetId(), link.first);
                int64_t node_id_one = device->GetChannel()->GetObject<PointToPointChannel>()->GetDevice(0)->GetNode()->GetId();
                int64_t node_id_two = device->GetChannel()->GetObject<PointToPointChannel>()->GetDevice(1)->GetNode()->GetId();
                ASSERT_EQUAL(device->GetChannel()->GetObject<PointToPointChannel>()->GetNDevices(), 2);
                ASSERT_TRUE((node_id_one == link.first && node_id_two == link.second) || (node_id_one == link.second && node_id_two == link.first));

            }

        }

        basicSimulation->Finalize();
        cleanup_ptop_receive_error_model_test();

    }
};

////////////////////////////////////////////////////////////////////////////////////////

class PtopReceiveErrorModelInvalidTestCase : public PtopReceiveErrorModelTestCase
{
public:
    PtopReceiveErrorModelInvalidTestCase () : PtopReceiveErrorModelTestCase ("ptop-receive-error-model invalid") {};
    void DoRun () {
        test_run_dir = ".tmp-test-ptop-receive-error-model-invalid";
        prepare_clean_run_dir(test_run_dir);
        prepare_ptop_receive_error_model_test_config();

        // Invalid receiving error model
        std::ofstream topology_file;
        Ptr<BasicSimulation> basicSimulation;
        basicSimulation = CreateObject<BasicSimulation>(test_run_dir);
        topology_file.open (test_run_dir + "/topology.properties");
        topology_file << "num_nodes=4" << std::endl;
        topology_file << "num_undirected_edges=3" << std::endl;
        topology_file << "switches=set(0,1,2,3)" << std::endl;
        topology_file << "switches_which_are_tors=set(0,3)" << std::endl;
        topology_file << "servers=set()" << std::endl;
        topology_file << "undirected_edges=set(0-1,1-2,2-3)" << std::endl;
        topology_file << "link_channel_delay_ns=10000" << std::endl;
        topology_file << "link_net_device_data_rate_megabit_per_s=100" << std::endl;
        topology_file << "link_net_device_queue=drop_tail(100p)" << std::endl;
        topology_file << "link_net_device_receive_error_model=an_error_model_which_does_not_exist(28420)" << std::endl;
        topology_file << "link_interface_traffic_control_qdisc=disabled" << std::endl;
        topology_file.close();
        ASSERT_EXCEPTION(CreateObject<TopologyPtop>(basicSimulation, Ipv4ArbiterRoutingHelper()));
        basicSimulation->Finalize();
        cleanup_ptop_receive_error_model_test();

        // Invalid receiving error model double value (above one)
        prepare_clean_run_dir(test_run_dir);
        prepare_ptop_receive_error_model_test_config();
        basicSimulation = CreateObject<BasicSimulation>(test_run_dir);
        topology_file.open (test_run_dir + "/topology.properties");
        topology_file << "num_nodes=4" << std::endl;
        topology_file << "num_undirected_edges=3" << std::endl;
        topology_file << "switches=set(0,1,2,3)" << std::endl;
        topology_file << "switches_which_are_tors=set(0,3)" << std::endl;
        topology_file << "servers=set()" << std::endl;
        topology_file << "undirected_edges=set(0-1,1-2,2-3)" << std::endl;
        topology_file << "link_channel_delay_ns=10000" << std::endl;
        topology_file << "link_net_device_data_rate_megabit_per_s=100" << std::endl;
        topology_file << "link_net_device_queue=drop_tail(100p)" << std::endl;
        topology_file << "link_net_device_receive_error_model=iid_uniform_random_pkt(1.0001)" << std::endl;
        topology_file << "link_interface_traffic_control_qdisc=disabled" << std::endl;
        topology_file.close();
        ASSERT_EXCEPTION(CreateObject<TopologyPtop>(basicSimulation, Ipv4ArbiterRoutingHelper()));
        basicSimulation->Finalize();
        cleanup_ptop_receive_error_model_test();

        // Invalid receiving error model double value (below zero)
        prepare_clean_run_dir(test_run_dir);
        prepare_ptop_receive_error_model_test_config();
        basicSimulation = CreateObject<BasicSimulation>(test_run_dir);
        topology_file.open (test_run_dir + "/topology.properties");
        topology_file << "num_nodes=4" << std::endl;
        topology_file << "num_undirected_edges=3" << std::endl;
        topology_file << "switches=set(0,1,2,3)" << std::endl;
        topology_file << "switches_which_are_tors=set(0,3)" << std::endl;
        topology_file << "servers=set()" << std::endl;
        topology_file << "undirected_edges=set(0-1,1-2,2-3)" << std::endl;
        topology_file << "link_channel_delay_ns=10000" << std::endl;
        topology_file << "link_net_device_data_rate_megabit_per_s=100" << std::endl;
        topology_file << "link_net_device_queue=drop_tail(100p)" << std::endl;
        topology_file << "link_net_device_receive_error_model=iid_uniform_random_pkt(-0.01)" << std::endl;
        topology_file << "link_interface_traffic_control_qdisc=disabled" << std::endl;
        topology_file.close();
        ASSERT_EXCEPTION(CreateObject<TopologyPtop>(basicSimulation, Ipv4ArbiterRoutingHelper()));
        basicSimulation->Finalize();
        cleanup_ptop_receive_error_model_test();

    }
};

////////////////////////////////////////////////////////////////////////////////////////

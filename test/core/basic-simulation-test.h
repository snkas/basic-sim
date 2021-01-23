/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

////////////////////////////////////////////////////////////////////////////////////////

class BasicSimulationNormalTestCase : public TestCaseWithLogValidators
{
public:
    BasicSimulationNormalTestCase () : TestCaseWithLogValidators ("basic-simulation normal") {};
    const std::string test_run_dir = ".tmp-test-basic-simulation-normal";
    
    void DoRun () {
        prepare_clean_run_dir(test_run_dir);

        // Prepare run directory
        std::ofstream config_file(test_run_dir + "/config_ns3.properties");
        config_file << "simulation_end_time_ns=10000000000" << std::endl;
        config_file << "simulation_seed=123456789" << std::endl;
        config_file.close();

        // Create and run
        Ptr<BasicSimulation> basicSimulation = CreateObject<BasicSimulation>(test_run_dir);
        basicSimulation->Run();
        basicSimulation->Finalize();

        // Test some of the values in there
        ASSERT_FALSE(basicSimulation->IsDistributedEnabled());
        ASSERT_EXCEPTION_MATCH_WHAT(
                basicSimulation->GetSystemId(),
                "Distributed mode is not enabled, as such the system id is zero always"
        );
        ASSERT_EXCEPTION_MATCH_WHAT(
                basicSimulation->GetSystemsCount(),
                "Distributed mode is not enabled, as such the systems count is one always"
        );
        ASSERT_EXCEPTION_MATCH_WHAT(
                basicSimulation->IsNodeAssignedToThisSystem(0),
                "Distributed mode is not enabled, as such this check should not need to be done"
        );
        ASSERT_EXCEPTION_MATCH_WHAT(
                basicSimulation->GetDistributedNodeSystemIdAssignment(),
                "Distributed mode is not enabled, as such the node assignment should not need to be retrieved"
        );
        ASSERT_EQUAL(basicSimulation->GetSimulationEndTimeNs(), 10000000000);
        ASSERT_EQUAL(basicSimulation->GetRunDir(), ".tmp-test-basic-simulation-normal");
        ASSERT_EQUAL(basicSimulation->GetLogsDir(), ".tmp-test-basic-simulation-normal/logs_ns3");

        // Verify finished
        validate_finished(test_run_dir);

        // Clean-up
        remove_file_if_exists(test_run_dir + "/config_ns3.properties");
        remove_file_if_exists(test_run_dir + "/logs_ns3/finished.txt");
        remove_file_if_exists(test_run_dir + "/logs_ns3/timing_results.txt");
        remove_file_if_exists(test_run_dir + "/logs_ns3/timing_results.csv");
        remove_dir_if_exists(test_run_dir + "/logs_ns3");
        remove_dir_if_exists(test_run_dir);

    }
};

////////////////////////////////////////////////////////////////////////////////////////

class BasicSimulationUnusedKeyTestCase : public TestCaseWithLogValidators
{
public:
    BasicSimulationUnusedKeyTestCase () : TestCaseWithLogValidators ("basic-simulation unused-key") {};
    const std::string test_run_dir = ".tmp-test-basic-simulation-unused-key";

    void DoRun () {
        prepare_clean_run_dir(test_run_dir);

        // Prepare run directory
        std::ofstream config_file(test_run_dir + "/config_ns3.properties");
        config_file << "simulation_end_time_ns=10000000000" << std::endl;
        config_file << "    \t\n\r" << std::endl;
        config_file << "another_key=abcdef" << std::endl;
        config_file << "# This is a comment" << std::endl;
        config_file << "simulation_seed=123456789" << std::endl;
        config_file.close();

        // Create and run
        Ptr<BasicSimulation> basicSimulation = CreateObject<BasicSimulation>(test_run_dir);
        ASSERT_EXCEPTION_MATCH_WHAT(
                basicSimulation->Run(),
                "Config key 'another_key' has not been requested (unused config keys are not allowed)"
        );

        // Clean-up
        remove_file_if_exists(test_run_dir + "/config_ns3.properties");
        remove_file_if_exists(test_run_dir + "/logs_ns3/finished.txt");
        remove_dir_if_exists(test_run_dir + "/logs_ns3");
        remove_dir_if_exists(test_run_dir);

    }
};

////////////////////////////////////////////////////////////////////////////////////////

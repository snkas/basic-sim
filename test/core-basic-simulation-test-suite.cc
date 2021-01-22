/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "ns3/basic-sim-module.h"

using namespace ns3;

#include "test-helpers.h"
#include "test-case-with-log-validators.h"

#include "core/basic-simulation-test.h"


class BasicSimCoreBasicSimulationTestSuite : public TestSuite {
public:
    BasicSimCoreBasicSimulationTestSuite() : TestSuite("basic-sim-core-basic-simulation", UNIT) {

        // Basic simulation
        AddTestCase(new BasicSimulationNormalTestCase, TestCase::QUICK);
        AddTestCase(new BasicSimulationUnusedKeyTestCase, TestCase::QUICK);

    }
};
static BasicSimCoreBasicSimulationTestSuite basicSimCoreBasicSimulationTestSuite;

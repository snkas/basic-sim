/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "ns3/basic-simulation.h"
#include "ns3/test.h"
#include "exp-util-test.h"
#include "topology-ptop-test.h"
#include "arbiter-test.h"
#include "ptop-utilization-test.h"

using namespace ns3;

class BasicSimTestSuite : public TestSuite {
public:
    BasicSimTestSuite() : TestSuite("basic-sim-core", UNIT) {

        // Experiment utilities
        AddTestCase(new ExpUtilStringsTestCase, TestCase::QUICK);
        AddTestCase(new ExpUtilParsingTestCase, TestCase::QUICK);
        AddTestCase(new ExpUtilSetsTestCase, TestCase::QUICK);
        AddTestCase(new ExpUtilConfigurationReadingTestCase, TestCase::QUICK);
        AddTestCase(new ExpUtilUnitConversionTestCase, TestCase::QUICK);
        AddTestCase(new ExpUtilFileSystemTestCase, TestCase::QUICK);

        // Point-to-point topology
        AddTestCase(new TopologyPtopEmptyTestCase, TestCase::QUICK);
        AddTestCase(new TopologyPtopSingleTestCase, TestCase::QUICK);
        AddTestCase(new TopologyPtopTorTestCase, TestCase::QUICK);
        AddTestCase(new TopologyPtopLeafSpineTestCase, TestCase::QUICK);
        AddTestCase(new TopologyPtopRingTestCase, TestCase::QUICK);
        AddTestCase(new TopologyPtopInvalidTestCase, TestCase::QUICK);

        // Arbiter
        AddTestCase(new ArbiterIpResolutionTestCase, TestCase::QUICK);
        AddTestCase(new ArbiterEcmpHashTestCase, TestCase::QUICK);
        AddTestCase(new ArbiterEcmpStringReprTestCase, TestCase::QUICK);
        AddTestCase(new ArbiterBadImplTestCase, TestCase::QUICK);

        // Point-to-point utilization tracking
        AddTestCase(new PtopUtilizationSimpleTestCase, TestCase::QUICK);

    }
};
static BasicSimTestSuite basicSimTestSuite;

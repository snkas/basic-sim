/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "ns3/basic-simulation.h"
#include "ns3/test.h"
#include "exp-util-test.h"
#include "topology-ptop-test.h"
#include "arbiter-test.h"
#include "ptop-link-utilization-test.h"
#include "ptop-link-queue-test.h"
#include "tcp-optimizer-test.h"
#include "log-update-helper-test.h"

using namespace ns3;

class BasicSimTestSuite : public TestSuite {
public:
    BasicSimTestSuite() : TestSuite("basic-sim-core", Type::UNIT) {

        // Experiment utilities
        AddTestCase(new ExpUtilStringsTestCase, TestCase::Duration::QUICK);
        AddTestCase(new ExpUtilParsingTestCase, TestCase::Duration::QUICK);
        AddTestCase(new ExpUtilSetsTestCase, TestCase::Duration::QUICK);
        AddTestCase(new ExpUtilConfigurationReadingTestCase, TestCase::Duration::QUICK);
        AddTestCase(new ExpUtilUnitConversionTestCase, TestCase::Duration::QUICK);
        AddTestCase(new ExpUtilFileSystemTestCase, TestCase::Duration::QUICK);

        // Log update helper
        AddTestCase(new LogUpdateHelperValidTestCase, TestCase::Duration::QUICK);
        AddTestCase(new LogUpdateHelperInvalidTestCase, TestCase::Duration::QUICK);

        // Point-to-point topology
        AddTestCase(new TopologyPtopEmptyTestCase, TestCase::Duration::QUICK);
        AddTestCase(new TopologyPtopSingleTestCase, TestCase::Duration::QUICK);
        AddTestCase(new TopologyPtopTorTestCase, TestCase::Duration::QUICK);
        AddTestCase(new TopologyPtopLeafSpineTestCase, TestCase::Duration::QUICK);
        AddTestCase(new TopologyPtopRingTestCase, TestCase::Duration::QUICK);
        AddTestCase(new TopologyPtopInvalidTestCase, TestCase::Duration::QUICK);

        // Arbiter
        AddTestCase(new ArbiterIpResolutionTestCase, TestCase::Duration::QUICK);
        AddTestCase(new ArbiterEcmpHashTestCase, TestCase::Duration::QUICK);
        AddTestCase(new ArbiterEcmpStringReprTestCase, TestCase::Duration::QUICK);
        AddTestCase(new ArbiterBadImplTestCase, TestCase::Duration::QUICK);

        // Point-to-point link utilization tracking
        AddTestCase(new PtopLinkUtilizationSimpleTestCase, TestCase::Duration::QUICK);
        AddTestCase(new PtopLinkUtilizationNotEnabledTestCase, TestCase::Duration::QUICK);

        // Point-to-point link queue tracking
        AddTestCase(new PtopLinkQueueSimpleTestCase, TestCase::Duration::QUICK);
        AddTestCase(new PtopLinkQueueNotEnabledTestCase, TestCase::Duration::QUICK);

        // TCP optimizer
        AddTestCase(new TcpOptimizerBasicTestCase, TestCase::Duration::QUICK);
        AddTestCase(new TcpOptimizerWorstCaseRttTestCase, TestCase::Duration::QUICK);

    }
};
static BasicSimTestSuite basicSimTestSuite;

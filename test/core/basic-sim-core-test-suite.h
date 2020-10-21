/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "exp-util-test.h"
#include "log-update-helper-test.h"
#include "topology-ptop-test.h"
#include "ptop-link-interface-tc-qdisc-red-test.h"
#include "arbiter-test.h"
#include "ptop-link-net-device-utilization-test.h"
#include "ptop-link-net-device-queue-test.h"
#include "tcp-optimizer-test.h"

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

        // Log update helper
        AddTestCase(new LogUpdateHelperValidTestCase, TestCase::QUICK);
        AddTestCase(new LogUpdateHelperValidWithFileTestCase, TestCase::QUICK);
        AddTestCase(new LogUpdateHelperInvalidTestCase, TestCase::QUICK);

        // Point-to-point topology
        AddTestCase(new TopologyPtopEmptyTestCase, TestCase::QUICK);
        AddTestCase(new TopologyPtopSingleTestCase, TestCase::QUICK);
        AddTestCase(new TopologyPtopTorTestCase, TestCase::QUICK);
        AddTestCase(new TopologyPtopLeafSpineTestCase, TestCase::QUICK);
        AddTestCase(new TopologyPtopRingTestCase, TestCase::QUICK);
        AddTestCase(new TopologyPtopInvalidTestCase, TestCase::QUICK);

        // Point-to-point traffic-control queueing discipline RED
        AddTestCase(new PtopLinkInterfaceTcQdiscRedValidTestCase, TestCase::QUICK);
        AddTestCase(new PtopLinkInterfaceTcQdiscRedInvalidTestCase, TestCase::QUICK);
        AddTestCase(new PtopLinkInterfaceTcQdiscRedEcnAndDropMarkingTestCase, TestCase::QUICK);

        // Arbiter
        AddTestCase(new ArbiterIpResolutionTestCase, TestCase::QUICK);
        AddTestCase(new ArbiterEcmpHashTestCase, TestCase::QUICK);
        AddTestCase(new ArbiterEcmpStringReprTestCase, TestCase::QUICK);
        AddTestCase(new ArbiterBadImplTestCase, TestCase::QUICK);
        AddTestCase(new ArbiterEcmpTooManyNodesTestCase, TestCase::QUICK);
        AddTestCase(new ArbiterEcmpSeparatedTestCase, TestCase::QUICK);

        // Point-to-point link net-device utilization tracking
        AddTestCase(new PtopLinkNetDeviceUtilizationSimpleTestCase, TestCase::QUICK);
        AddTestCase(new PtopLinkNetDeviceUtilizationSpecificLinksTestCase, TestCase::QUICK);
        AddTestCase(new PtopLinkNetDeviceUtilizationNotEnabledTestCase, TestCase::QUICK);

        // Point-to-point link net-device queue tracking
        AddTestCase(new PtopLinkNetDeviceQueueSimpleTestCase, TestCase::QUICK);
        AddTestCase(new PtopLinkNetDeviceQueueSpecificLinksTestCase, TestCase::QUICK);
        AddTestCase(new PtopLinkNetDeviceQueueNotEnabledTestCase, TestCase::QUICK);

        // TCP optimizer
        AddTestCase(new TcpOptimizerBasicTestCase, TestCase::QUICK);
        AddTestCase(new TcpOptimizerWorstCaseRttTestCase, TestCase::QUICK);

    }
};
static BasicSimTestSuite basicSimTestSuite;

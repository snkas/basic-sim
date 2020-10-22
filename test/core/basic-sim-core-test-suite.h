/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "basic-simulation-test.h"
#include "exp-util-test.h"
#include "log-update-helper-test.h"
#include "tcp-optimizer-test.h"
#include "topology-ptop-test.h"
#include "arbiter-test.h"

#include "ptop-queue-test.h"
#include "ptop-receive-error-model-test.h"
#include "ptop-tc-qdisc-test.h"

#include "ptop-tracking-link-net-device-utilization-test.h"
#include "ptop-tracking-link-net-device-queue-test.h"
#include "ptop-tracking-link-interface-tc-qdisc-queue-test.h"


class BasicSimTestSuite : public TestSuite {
public:
    BasicSimTestSuite() : TestSuite("basic-sim-core", UNIT) {

        // Basic simulation
        AddTestCase(new BasicSimulationNormalTestCase, TestCase::QUICK);
        AddTestCase(new BasicSimulationUnusedKeyTestCase, TestCase::QUICK);

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

        // TCP optimizer
        AddTestCase(new TcpOptimizerBasicTestCase, TestCase::QUICK);
        AddTestCase(new TcpOptimizerWorstCaseRttTestCase, TestCase::QUICK);

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
        AddTestCase(new ArbiterEcmpTooManyNodesTestCase, TestCase::QUICK);
        AddTestCase(new ArbiterEcmpSeparatedTestCase, TestCase::QUICK);

        // Point-to-point queue
        AddTestCase(new PtopQueueValidTestCase, TestCase::QUICK);
        AddTestCase(new PtopQueueInvalidTestCase, TestCase::QUICK);

        // Point-to-point receive error model
        AddTestCase(new PtopReceiveErrorModelValidTestCase, TestCase::QUICK);
        AddTestCase(new PtopReceiveErrorModelInvalidTestCase, TestCase::QUICK);

        // Point-to-point traffic-control qdisc
        AddTestCase(new PtopTcQdiscFqCodelValidTestCase, TestCase::QUICK);
        AddTestCase(new PtopTcQdiscRedValidTestCase, TestCase::QUICK);
        AddTestCase(new PtopTcQdiscInvalidTestCase, TestCase::QUICK);
        AddTestCase(new PtopTcQdiscRedEcnAndDropMarkingTestCase, TestCase::QUICK);

        // Point-to-point link net-device utilization tracking
        AddTestCase(new PtopTrackingLinkNetDeviceUtilizationSimpleTestCase, TestCase::QUICK);
        AddTestCase(new PtopTrackingLinkNetDeviceUtilizationSpecificLinksTestCase, TestCase::QUICK);
        AddTestCase(new PtopTrackingLinkNetDeviceUtilizationNotEnabledTestCase, TestCase::QUICK);

        // Point-to-point link net-device queue tracking
        AddTestCase(new PtopTrackingLinkNetDeviceQueueSimpleTestCase, TestCase::QUICK);
        AddTestCase(new PtopTrackingLinkNetDeviceQueueSpecificLinksTestCase, TestCase::QUICK);
        AddTestCase(new PtopTrackingLinkNetDeviceQueueNotEnabledTestCase, TestCase::QUICK);

        // Point-to-point link interface traffic-control qdisc queue tracking
        AddTestCase(new PtopTrackingLinkInterfaceTcQdiscQueueSimpleTestCase, TestCase::QUICK);
        AddTestCase(new PtopTrackingLinkInterfaceTcQdiscQueueSpecificLinksTestCase, TestCase::QUICK);
        AddTestCase(new PtopTrackingLinkInterfaceTcQdiscQueueNotEnabledTestCase, TestCase::QUICK);
        AddTestCase(new PtopTrackingLinkInterfaceTcQdiscQueueNoQdiscTestCase, TestCase::QUICK);

    }
};
static BasicSimTestSuite basicSimTestSuite;

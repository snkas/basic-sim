/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "ns3/basic-sim-module.h"

using namespace ns3;

#include "test-helpers.h"
#include "test-case-with-log-validators.h"

#include "core/ptop-tracking-link-net-device-utilization-test.h"
#include "core/ptop-tracking-link-net-device-queue-test.h"
#include "core/ptop-tracking-link-interface-tc-qdisc-queue-test.h"


class BasicSimCorePtopTrackingTestSuite : public TestSuite {
public:
    BasicSimCorePtopTrackingTestSuite() : TestSuite("basic-sim-core-ptop-tracking", UNIT) {

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
static BasicSimCorePtopTrackingTestSuite basicSimCorePtopTrackingTestSuite;

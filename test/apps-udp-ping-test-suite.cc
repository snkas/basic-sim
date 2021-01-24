/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "ns3/basic-sim-module.h"

using namespace ns3;

#include "test-helpers.h"
#include "test-case-with-log-validators.h"

#include "apps/udp-ping-schedule-reader-test.h"
#include "apps/udp-ping-simple-test.h"
#include "apps/udp-ping-end-to-end-test.h"
#include "apps/udp-ping-pingmesh-test.h"

class BasicSimAppsUdpPingTestSuite : public TestSuite {
public:
    BasicSimAppsUdpPingTestSuite() : TestSuite("basic-sim-apps-udp-ping", UNIT) {

        // UDP ping schedule reader
        AddTestCase(new UdpPingScheduleReaderNormalTestCase, TestCase::QUICK);
        AddTestCase(new UdpPingScheduleReaderInvalidTestCase, TestCase::QUICK);

        // UDP ping simple
        AddTestCase(new UdpPingSimpleHeaderTestCase, TestCase::QUICK);
        AddTestCase(new UdpPingSimpleDoubleServerBindTestCase, TestCase::QUICK);
        AddTestCase(new UdpPingSimpleDoubleClientBindTestCase, TestCase::QUICK);

        // UDP ping end-to-end
        AddTestCase(new UdpPingEndToEndOneToOneManyTestCase, TestCase::QUICK);
        AddTestCase(new UdpPingEndToEndMultiPathTestCase, TestCase::QUICK);

        // UDP ping pingmesh
        AddTestCase(new UdpPingPingmeshNineAllTestCase, TestCase::QUICK);
        AddTestCase(new UdpPingPingmeshFivePairsTestCase, TestCase::QUICK);
        AddTestCase(new UdpPingPingmeshCompetitionTcpTestCase, TestCase::QUICK);
        AddTestCase(new UdpPingPingmeshOnePairNoRecTestCase, TestCase::QUICK);
        AddTestCase(new UdpPingPingmeshInvalidPairATestCase, TestCase::QUICK);
        AddTestCase(new UdpPingPingmeshInvalidPairBTestCase, TestCase::QUICK);
        AddTestCase(new UdpPingPingmeshNotEnabledTestCase, TestCase::QUICK);

    }
};
static BasicSimAppsUdpPingTestSuite basicSimAppsUdpPingTestSuite;

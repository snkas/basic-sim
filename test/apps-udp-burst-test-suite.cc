/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "ns3/basic-sim-module.h"

using namespace ns3;

#include "test-helpers.h"
#include "test-case-with-log-validators.h"

#include "apps/udp-burst-schedule-reader-test.h"
#include "apps/udp-burst-simple-test.h"
#include "apps/udp-burst-end-to-end-test.h"

class BasicSimAppsUdpBurstTestSuite : public TestSuite {
public:
    BasicSimAppsUdpBurstTestSuite() : TestSuite("basic-sim-apps-udp-burst", UNIT) {

        // UDP burst schedule reader
        AddTestCase(new UdpBurstScheduleReaderNormalTestCase, TestCase::QUICK);
        AddTestCase(new UdpBurstScheduleReaderInvalidTestCase, TestCase::QUICK);

        // UDP burst simple
        AddTestCase(new UdpBurstSimpleHeaderTestCase, TestCase::QUICK);
        AddTestCase(new UdpBurstSimpleDoubleServerBindTestCase, TestCase::QUICK);
        AddTestCase(new UdpBurstSimpleDoubleClientBindTestCase, TestCase::QUICK);

        // UDP burst end-to-end
        AddTestCase(new UdpBurstEndToEndOneToOneEqualStartTestCase, TestCase::QUICK);
        AddTestCase(new UdpBurstEndToEndSingleOverflowTestCase, TestCase::QUICK);
        AddTestCase(new UdpBurstEndToEndDoubleEnoughTestCase, TestCase::QUICK);
        AddTestCase(new UdpBurstEndToEndDoubleOverflowTestCase, TestCase::QUICK);
        AddTestCase(new UdpBurstEndToEndLossTestCase, TestCase::QUICK);
        AddTestCase(new UdpBurstEndToEndNotEnabledTestCase, TestCase::QUICK);
        AddTestCase(new UdpBurstEndToEndInvalidLoggingIdTestCase, TestCase::QUICK);
        AddTestCase(new UdpBurstEndToEndLoggingSpecificTestCase, TestCase::QUICK);
        AddTestCase(new UdpBurstEndToEndLoggingAllTestCase, TestCase::QUICK);
        AddTestCase(new UdpBurstEndToEndMultiPathTestCase, TestCase::QUICK);

    }
};
static BasicSimAppsUdpBurstTestSuite basicSimAppsUdpBurstTestSuite;

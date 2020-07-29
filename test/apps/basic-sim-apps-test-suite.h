/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "ns3/basic-simulation.h"
#include "ns3/test.h"
#include "tcp-flow-schedule-reader-test.h"
#include "udp-burst-schedule-reader-test.h"
#include "end-to-end-flows-test.h"
#include "end-to-end-pingmesh-test.h"
#include "end-to-end-manual-test.h"

using namespace ns3;

class BasicAppsTestSuite : public TestSuite {
public:
    BasicAppsTestSuite() : TestSuite("basic-sim-apps", UNIT) {

        // TCP flow schedule reader
        AddTestCase(new TcpFlowScheduleReaderNormalTestCase, TestCase::QUICK);
        AddTestCase(new TcpFlowScheduleReaderInvalidTestCase, TestCase::QUICK);

        // UDP burst schedule reader
        AddTestCase(new UdpBurstScheduleReaderNormalTestCase, TestCase::QUICK);
        AddTestCase(new UdpBurstScheduleReaderInvalidTestCase, TestCase::QUICK);


        AddTestCase(new EndToEndFlowsOneToOneEqualStartTestCase, TestCase::QUICK);
        AddTestCase(new EndToEndFlowsOneToOneSimpleStartTestCase, TestCase::QUICK);
        AddTestCase(new EndToEndFlowsOneToOneApartStartTestCase, TestCase::QUICK);
        AddTestCase(new EndToEndFlowsEcmpSimpleTestCase, TestCase::QUICK);
        AddTestCase(new EndToEndFlowsEcmpRemainTestCase, TestCase::QUICK);
        AddTestCase(new EndToEndFlowsNonExistentRunDirTestCase, TestCase::QUICK);
        AddTestCase(new EndToEndFlowsOneDropOneNotTestCase, TestCase::QUICK);
        AddTestCase(new EndToEndPingmeshNineAllTestCase, TestCase::QUICK);
        AddTestCase(new EndToEndPingmeshNinePairsTestCase, TestCase::QUICK);
        AddTestCase(new EndToEndManualTestCase, TestCase::QUICK);
    }
};
static BasicAppsTestSuite basicAppsTestSuite;

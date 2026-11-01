/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "ns3/basic-simulation.h"
#include "ns3/test.h"
#include "tcp-flow-schedule-reader-test.h"
#include "udp-burst-schedule-reader-test.h"
#include "tcp-flow-end-to-end-test.h"
#include "pingmesh-end-to-end-test.h"
#include "manual-end-to-end-test.h"
#include "udp-burst-end-to-end-test.h"

using namespace ns3;

class BasicAppsTestSuite : public TestSuite {
public:
    BasicAppsTestSuite() : TestSuite("basic-sim-apps", Type::UNIT) {

        // TCP flow schedule reader
        AddTestCase(new TcpFlowScheduleReaderNormalTestCase, TestCase::Duration::QUICK);
        AddTestCase(new TcpFlowScheduleReaderInvalidTestCase, TestCase::Duration::QUICK);

        // UDP burst schedule reader
        AddTestCase(new UdpBurstScheduleReaderNormalTestCase, TestCase::Duration::QUICK);
        AddTestCase(new UdpBurstScheduleReaderInvalidTestCase, TestCase::Duration::QUICK);

        // Manual end-to-end, which means the application helpers are used
        // directly instead of the schedulers reading from files
        AddTestCase(new ManualEndToEndTestCase, TestCase::Duration::QUICK);

        // TCP flows end-to-end
        AddTestCase(new TcpFlowEndToEndOneToOneEqualStartTestCase, TestCase::Duration::QUICK);
        AddTestCase(new TcpFlowEndToEndOneToOneSimpleStartTestCase, TestCase::Duration::QUICK);
        AddTestCase(new TcpFlowEndToEndOneToOneApartStartTestCase, TestCase::Duration::QUICK);
        AddTestCase(new TcpFlowEndToEndEcmpSimpleTestCase, TestCase::Duration::QUICK);
        AddTestCase(new TcpFlowEndToEndEcmpRemainTestCase, TestCase::Duration::QUICK);
        AddTestCase(new TcpFlowEndToEndNonExistentRunDirTestCase, TestCase::Duration::QUICK);
        AddTestCase(new TcpFlowEndToEndOneDropOneNotTestCase, TestCase::Duration::QUICK);
        AddTestCase(new TcpFlowEndToEndConnFailTestCase, TestCase::Duration::QUICK);
        AddTestCase(new TcpFlowEndToEndPrematureCloseTestCase, TestCase::Duration::QUICK);
        AddTestCase(new TcpFlowEndToEndNotEnabledTestCase, TestCase::Duration::QUICK);
        AddTestCase(new TcpFlowEndToEndInvalidLoggingIdTestCase, TestCase::Duration::QUICK);

        // Pingmesh end-to-end
        AddTestCase(new PingmeshEndToEndNineAllTestCase, TestCase::Duration::QUICK);
        AddTestCase(new PingmeshEndToEndFivePairsTestCase, TestCase::Duration::QUICK);
        AddTestCase(new PingmeshEndToEndCompetitionTcpTestCase, TestCase::Duration::QUICK);
        AddTestCase(new PingmeshEndToEndNotEnabledTestCase, TestCase::Duration::QUICK);

        // UDP burst end-to-end
        AddTestCase(new UdpBurstEndToEndOneToOneEqualStartTestCase, TestCase::Duration::QUICK);
        AddTestCase(new UdpBurstEndToEndSingleOverflowTestCase, TestCase::Duration::QUICK);
        AddTestCase(new UdpBurstEndToEndDoubleEnoughTestCase, TestCase::Duration::QUICK);
        AddTestCase(new UdpBurstEndToEndDoubleOverflowTestCase, TestCase::Duration::QUICK);
        AddTestCase(new UdpBurstEndToEndNotEnabledTestCase, TestCase::Duration::QUICK);
        AddTestCase(new UdpBurstEndToEndInvalidLoggingIdTestCase, TestCase::Duration::QUICK);

    }
};
static BasicAppsTestSuite basicAppsTestSuite;

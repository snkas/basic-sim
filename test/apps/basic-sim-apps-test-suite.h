/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "initial-helpers-test.h"
#include "manual-end-to-end-test.h"
#include "tcp-flow-schedule-reader-test.h"
#include "tcp-flow-end-to-end-test.h"
#include "udp-burst-schedule-reader-test.h"
#include "udp-burst-end-to-end-test.h"
#include "udp-ping-schedule-reader-test.h"
#include "udp-ping-end-to-end-test.h"
#include "udp-ping-pingmesh-test.h"

class BasicAppsTestSuite : public TestSuite {
public:
    BasicAppsTestSuite() : TestSuite("basic-sim-apps", UNIT) {

        // Initial helpers test case
        AddTestCase(new InitialHelpersCorrectTestCase, TestCase::QUICK);
        AddTestCase(new InitialHelpersMismatchesTestCase, TestCase::QUICK);

        // Manual end-to-end, which means the application helpers are used
        // directly instead of the schedulers reading from files
        AddTestCase(new ManualEndToEndTestCase, TestCase::QUICK);
        AddTestCase(new ManualTriggerStopExceptionsTestCase, TestCase::QUICK);

        // TCP flow schedule reader
        AddTestCase(new TcpFlowScheduleReaderNormalTestCase, TestCase::QUICK);
        AddTestCase(new TcpFlowScheduleReaderInvalidTestCase, TestCase::QUICK);

        // TCP flows end-to-end
        AddTestCase(new TcpFlowEndToEndOneToOneEqualStartTestCase, TestCase::QUICK);
        AddTestCase(new TcpFlowEndToEndOneToOneSimpleStartTestCase, TestCase::QUICK);
        AddTestCase(new TcpFlowEndToEndOneToOneApartStartTestCase, TestCase::QUICK);
        AddTestCase(new TcpFlowEndToEndEcmpSimpleTestCase, TestCase::QUICK);
        AddTestCase(new TcpFlowEndToEndEcmpRemainTestCase, TestCase::QUICK);
        AddTestCase(new TcpFlowEndToEndLoggingSpecificTestCase, TestCase::QUICK);
        AddTestCase(new TcpFlowEndToEndLoggingAllTestCase, TestCase::QUICK);
        AddTestCase(new TcpFlowEndToEndNonExistentRunDirTestCase, TestCase::QUICK);
        AddTestCase(new TcpFlowEndToEndOneDropOneNotTestCase, TestCase::QUICK);
        AddTestCase(new TcpFlowEndToEndConnFailTestCase, TestCase::QUICK);
        AddTestCase(new TcpFlowEndToEndPrematureCloseTestCase, TestCase::QUICK);
        AddTestCase(new TcpFlowEndToEndBadCloseTestCase, TestCase::QUICK);
        AddTestCase(new TcpFlowEndToEndNotEnabledTestCase, TestCase::QUICK);
        AddTestCase(new TcpFlowEndToEndInvalidLoggingIdTestCase, TestCase::QUICK);
        AddTestCase(new TcpFlowEndToEndMultiPathTestCase, TestCase::QUICK);

        // UDP burst schedule reader
        AddTestCase(new UdpBurstScheduleReaderNormalTestCase, TestCase::QUICK);
        AddTestCase(new UdpBurstScheduleReaderInvalidTestCase, TestCase::QUICK);

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

        // UDP ping schedule reader
        AddTestCase(new UdpPingScheduleReaderNormalTestCase, TestCase::QUICK);
        AddTestCase(new UdpPingScheduleReaderInvalidTestCase, TestCase::QUICK);

        // UDP ping end-to-end
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
static BasicAppsTestSuite basicAppsTestSuite;

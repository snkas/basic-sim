/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "ns3/basic-sim-module.h"

using namespace ns3;

#include "test-helpers.h"
#include "test-case-with-log-validators.h"

#include "apps/tcp-flow-schedule-reader-test.h"
#include "apps/tcp-flow-simple-test.h"
#include "apps/tcp-flow-end-to-end-test.h"

class BasicSimAppsTcpFlowTestSuite : public TestSuite {
public:
    BasicSimAppsTcpFlowTestSuite() : TestSuite("basic-sim-apps-tcp-flow", UNIT) {

        // TCP flow schedule reader
        AddTestCase(new TcpFlowScheduleReaderNormalTestCase, TestCase::QUICK);
        AddTestCase(new TcpFlowScheduleReaderInvalidTestCase, TestCase::QUICK);

        // TCP flow simple
        AddTestCase(new TcpFlowSimpleDoubleServerBindTestCase, TestCase::QUICK);
        AddTestCase(new TcpFlowSimpleDoubleClientBindTestCase, TestCase::QUICK);

        // TCP flow end-to-end
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
        AddTestCase(new TcpFlowEndToEndOneToOneEcnTestCase, TestCase::QUICK);
        AddTestCase(new TcpFlowEndToEndOneToOneTwoServersTestCase, TestCase::QUICK);

    }
};
static BasicSimAppsTcpFlowTestSuite basicSimAppsTcpFlowTestSuite;

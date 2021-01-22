/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "ns3/basic-sim-module.h"

using namespace ns3;

#include "test-helpers.h"
#include "test-case-with-log-validators.h"

#include "apps/manual-end-to-end-test.h"

class BasicSimAppsManualTestSuite : public TestSuite {
public:
    BasicSimAppsManualTestSuite() : TestSuite("basic-sim-apps-manual", UNIT) {

        // Manual end-to-end, which means the application helpers are used
        // directly instead of the schedulers reading from files
        AddTestCase(new ManualEndToEndTestCase, TestCase::QUICK);
        AddTestCase(new ManualTriggerStopExceptionsTestCase, TestCase::QUICK);

    }
};
static BasicSimAppsManualTestSuite basicSimAppsManualTestSuite;

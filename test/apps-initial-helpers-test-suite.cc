/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "ns3/basic-sim-module.h"

using namespace ns3;

#include "test-helpers.h"
#include "test-case-with-log-validators.h"

#include "apps/initial-helpers-test.h"

class BasicSimAppsInitialHelpersTestSuite : public TestSuite {
public:
    BasicSimAppsInitialHelpersTestSuite() : TestSuite("basic-sim-apps-initial-helpers", UNIT) {

        // Initial helpers test case
        AddTestCase(new InitialHelpersCorrectTestCase, TestCase::QUICK);
        AddTestCase(new InitialHelpersMismatchesTestCase, TestCase::QUICK);

    }
};
static BasicSimAppsInitialHelpersTestSuite basicSimAppsInitialHelpersTestSuite;

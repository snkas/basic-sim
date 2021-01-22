/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "ns3/basic-sim-module.h"

using namespace ns3;

#include "test-helpers.h"
#include "test-case-with-log-validators.h"

#include "core/log-update-helper-test.h"


class BasicSimCoreLogUpdateHelperTestSuite : public TestSuite {
public:
    BasicSimCoreLogUpdateHelperTestSuite() : TestSuite("basic-sim-core-log-update-helper", UNIT) {

        // Log update helper
        AddTestCase(new LogUpdateHelperValidTestCase, TestCase::QUICK);
        AddTestCase(new LogUpdateHelperValidWithFileTestCase, TestCase::QUICK);
        AddTestCase(new LogUpdateHelperInvalidTestCase, TestCase::QUICK);
        AddTestCase(new LogUpdateHelperValidStringTestCase, TestCase::QUICK);
        AddTestCase(new LogUpdateHelperValidStringWithFileTestCase, TestCase::QUICK);

    }
};
static BasicSimCoreLogUpdateHelperTestSuite basicSimCoreLogUpdateHelperTestSuite;

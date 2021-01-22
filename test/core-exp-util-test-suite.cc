/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "ns3/basic-sim-module.h"

using namespace ns3;

#include "test-helpers.h"
#include "test-case-with-log-validators.h"

#include "core/exp-util-test.h"


class BasicSimCoreExpUtilTestSuite : public TestSuite {
public:
    BasicSimCoreExpUtilTestSuite() : TestSuite("basic-sim-core-exp-util", UNIT) {

        // Experiment utilities
        AddTestCase(new ExpUtilStringsTestCase, TestCase::QUICK);
        AddTestCase(new ExpUtilParsingTestCase, TestCase::QUICK);
        AddTestCase(new ExpUtilSetsTestCase, TestCase::QUICK);
        AddTestCase(new ExpUtilConfigurationReadingTestCase, TestCase::QUICK);
        AddTestCase(new ExpUtilUnitConversionTestCase, TestCase::QUICK);
        AddTestCase(new ExpUtilFileSystemTestCase, TestCase::QUICK);

    }
};
static BasicSimCoreExpUtilTestSuite basicSimCoreExpUtilTestSuite;

/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "ns3/basic-sim-module.h"

using namespace ns3;

#include "test-helpers.h"
#include "test-case-with-log-validators.h"

#include "apps/tcp-config-helper-test.h"


class BasicSimCoreTcpConfigHelperTestSuite : public TestSuite {
public:
    BasicSimCoreTcpConfigHelperTestSuite() : TestSuite("basic-sim-apps-tcp-config-helper", UNIT) {

        // TCP config helper
        AddTestCase(new TcpConfigHelperDefaultTestCase, TestCase::QUICK);
        AddTestCase(new TcpConfigHelperBasicTestCase, TestCase::QUICK);
        AddTestCase(new TcpConfigHelperCustomTestCase, TestCase::QUICK);
        AddTestCase(new TcpConfigHelperInvalidTestCase, TestCase::QUICK);

    }
};
static BasicSimCoreTcpConfigHelperTestSuite basicSimCoreTcpConfigHelperTestSuite;

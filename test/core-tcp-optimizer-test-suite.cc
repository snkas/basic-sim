/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "ns3/basic-sim-module.h"

using namespace ns3;

#include "test-helpers.h"
#include "test-case-with-log-validators.h"

#include "core/tcp-optimizer-test.h"


class BasicSimCoreTcpOptimizerTestSuite : public TestSuite {
public:
    BasicSimCoreTcpOptimizerTestSuite() : TestSuite("basic-sim-core-tcp-optimizer", UNIT) {

        // TCP optimizer
        AddTestCase(new TcpOptimizerBasicTestCase, TestCase::QUICK);
        AddTestCase(new TcpOptimizerWorstCaseRttTestCase, TestCase::QUICK);

    }
};
static BasicSimCoreTcpOptimizerTestSuite basicSimCoreTcpOptimizerTestSuite;

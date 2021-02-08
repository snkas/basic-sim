/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "ns3/basic-sim-module.h"

using namespace ns3;

#include "test-helpers.h"
#include "test-case-with-log-validators.h"

#include "core/topology-ptop-test.h"
#include "core/ptop-queue-test.h"
#include "core/ptop-receive-error-model-test.h"
#include "core/ptop-tc-qdisc-test.h"

class BasicSimCorePtopTestSuite : public TestSuite {
public:
    BasicSimCorePtopTestSuite() : TestSuite("basic-sim-core-ptop", UNIT) {

        // Point-to-point topology
        AddTestCase(new TopologyPtopEmptyTestCase, TestCase::QUICK);
        AddTestCase(new TopologyPtopSingleTestCase, TestCase::QUICK);
        AddTestCase(new TopologyPtopTorTestCase, TestCase::QUICK);
        AddTestCase(new TopologyPtopLeafSpineTestCase, TestCase::QUICK);
        AddTestCase(new TopologyPtopRingTestCase, TestCase::QUICK);
        AddTestCase(new TopologyPtopInvalidTestCase, TestCase::QUICK);

        // Point-to-point queue
        AddTestCase(new PtopQueueValidTestCase, TestCase::QUICK);
        AddTestCase(new PtopQueueInvalidTestCase, TestCase::QUICK);

        // Point-to-point receive error model
        AddTestCase(new PtopReceiveErrorModelValidTestCase, TestCase::QUICK);
        AddTestCase(new PtopReceiveErrorModelInvalidTestCase, TestCase::QUICK);

        // Point-to-point traffic-control qdisc
        AddTestCase(new PtopTcQdiscFqCodelValidTestCase, TestCase::QUICK);
        AddTestCase(new PtopTcQdiscRedValidTestCase, TestCase::QUICK);
        AddTestCase(new PtopTcQdiscInvalidTestCase, TestCase::QUICK);
        AddTestCase(new PtopTcQdiscRedDropMarkingTestCase, TestCase::QUICK);
        AddTestCase(new PtopTcQdiscRedEcnMarkingTestCase, TestCase::QUICK);
        AddTestCase(new PtopTcQdiscPfifoFastAbsoluteTestCase, TestCase::QUICK);

    }
};
static BasicSimCorePtopTestSuite basicSimCorePtopTestSuite;

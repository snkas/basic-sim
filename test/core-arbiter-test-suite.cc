/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "ns3/basic-sim-module.h"

using namespace ns3;

#include "test-helpers.h"
#include "test-case-with-log-validators.h"

#include "core/arbiter-test.h"


class BasicSimCoreArbiterTestSuite : public TestSuite {
public:
    BasicSimCoreArbiterTestSuite() : TestSuite("basic-sim-core-arbiter", UNIT) {

        // Arbiter
        AddTestCase(new ArbiterIpResolutionTestCase, TestCase::QUICK);
        AddTestCase(new ArbiterResultTestCase, TestCase::QUICK);
        AddTestCase(new ArbiterPtopOneTestCase, TestCase::QUICK);
        AddTestCase(new Ipv4ArbiterRoutingExceptionsTestCase, TestCase::QUICK);
        AddTestCase(new ArbiterEcmpHashTestCase, TestCase::QUICK);
        AddTestCase(new ArbiterEcmpStringReprTestCase, TestCase::QUICK);
        AddTestCase(new ArbiterBadImplTestCase, TestCase::QUICK);
        AddTestCase(new ArbiterEcmpTooManyNodesTestCase, TestCase::QUICK);
        AddTestCase(new ArbiterEcmpSeparatedTestCase, TestCase::QUICK);
        AddTestCase(new Ipv4ArbiterRoutingNoRouteTestCase, TestCase::QUICK);

    }
};
static BasicSimCoreArbiterTestSuite basicSimCoreArbiterTestSuite;

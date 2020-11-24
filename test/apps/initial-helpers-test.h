/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "ns3/initial-helpers.h"

////////////////////////////////////////////////////////////////////////////////////////

class ClassWithAnAttribute : public Object {

public:
    static TypeId GetTypeId(void);
    ClassWithAnAttribute();
    virtual ~ClassWithAnAttribute();
    uint64_t m_attA;
    bool m_attB;
    double m_attC;
    Time m_attD;

};

NS_OBJECT_ENSURE_REGISTERED (ClassWithAnAttribute);
TypeId ClassWithAnAttribute::GetTypeId (void)
{
    static TypeId tid = TypeId ("ns3::ClassWithAnAttribute")
            .SetParent<Object> ()
            .SetGroupName("BasicSim")
            .AddConstructor<ClassWithAnAttribute>()
            .AddAttribute("AttA", "Attribute A as unsigned integer.",
                          UintegerValue(100000),
                          MakeUintegerAccessor(&ClassWithAnAttribute::m_attA),
                          MakeUintegerChecker<uint32_t>(1))
            .AddAttribute("AttB", "Attribute B as boolean.",
                          BooleanValue(true),
                          MakeBooleanAccessor(&ClassWithAnAttribute::m_attB),
                          MakeBooleanChecker())
            .AddAttribute("AttC", "Attribute C as double.",
                          DoubleValue(352.3),
                          MakeDoubleAccessor(&ClassWithAnAttribute::m_attC),
                          MakeDoubleChecker<double>(0))
            .AddAttribute("AttD", "Attribute D as time value.",
                          TimeValue(NanoSeconds(123456)),
                          MakeTimeAccessor(&ClassWithAnAttribute::m_attD),
                          MakeTimeChecker())
    ;
    return tid;
}

ClassWithAnAttribute::ClassWithAnAttribute() {

}

ClassWithAnAttribute::~ClassWithAnAttribute() {

}

////////////////////////////////////////////////////////////////////////////////////////

class InitialHelpersCorrectTestCase : public TestCase {
public:
    InitialHelpersCorrectTestCase() : TestCase("initial-helpers correct") {};

    void DoRun() {

        // Get the initial values
        ASSERT_EQUAL(GetInitialUintValue("ns3::ClassWithAnAttribute", "AttA"), 100000);
        ASSERT_EQUAL(GetInitialBooleanValue("ns3::ClassWithAnAttribute", "AttB"), true);
        ASSERT_EQUAL(GetInitialDoubleValue("ns3::ClassWithAnAttribute", "AttC"), 352.3);
        ASSERT_EQUAL(GetInitialTimeValue("ns3::ClassWithAnAttribute", "AttD").Get(), NanoSeconds(123456));

        // Check it for an instantiated object
        Ptr<ClassWithAnAttribute> a = CreateObject<ClassWithAnAttribute>();
        ASSERT_EQUAL(a->m_attA, 100000);
        ASSERT_EQUAL(a->m_attB, true);
        ASSERT_EQUAL(a->m_attC, 352.3);
        ASSERT_EQUAL(a->m_attD, NanoSeconds(123456));

        // Set them to something different
        Config::SetDefault("ns3::ClassWithAnAttribute::AttA", UintegerValue(13444));
        Config::SetDefault("ns3::ClassWithAnAttribute::AttB", BooleanValue(false));
        Config::SetDefault("ns3::ClassWithAnAttribute::AttC", DoubleValue(4626.334));
        Config::SetDefault("ns3::ClassWithAnAttribute::AttD", TimeValue(NanoSeconds(1003352)));

        // Check it
        ASSERT_EQUAL(GetInitialUintValue("ns3::ClassWithAnAttribute", "AttA"), 13444);
        ASSERT_EQUAL(GetInitialBooleanValue("ns3::ClassWithAnAttribute", "AttB"), false);
        ASSERT_EQUAL(GetInitialDoubleValue("ns3::ClassWithAnAttribute", "AttC"), 4626.334);
        ASSERT_EQUAL(GetInitialTimeValue("ns3::ClassWithAnAttribute", "AttD").Get(), NanoSeconds(1003352));

        // Check it for an instantiated object
        Ptr<ClassWithAnAttribute> b = CreateObject<ClassWithAnAttribute>();
        ASSERT_EQUAL(b->m_attA, 13444);
        ASSERT_EQUAL(b->m_attB, false);
        ASSERT_EQUAL(b->m_attC, 4626.334);
        ASSERT_EQUAL(b->m_attD, NanoSeconds(1003352));

        // Now reset
        Config::Reset();

        // Get the reset initial values
        ASSERT_EQUAL(GetInitialUintValue("ns3::ClassWithAnAttribute", "AttA"), 100000);
        ASSERT_EQUAL(GetInitialBooleanValue("ns3::ClassWithAnAttribute", "AttB"), true);
        ASSERT_EQUAL(GetInitialDoubleValue("ns3::ClassWithAnAttribute", "AttC"), 352.3);
        ASSERT_EQUAL(GetInitialTimeValue("ns3::ClassWithAnAttribute", "AttD").Get(), NanoSeconds(123456));

        // Check it for an instantiated object
        Ptr<ClassWithAnAttribute> c = CreateObject<ClassWithAnAttribute>();
        ASSERT_EQUAL(c->m_attA, 100000);
        ASSERT_EQUAL(c->m_attB, true);
        ASSERT_EQUAL(c->m_attC, 352.3);
        ASSERT_EQUAL(c->m_attD, NanoSeconds(123456));

    }
};

////////////////////////////////////////////////////////////////////////////////////////

class InitialHelpersMismatchesTestCase : public TestCase {
public:
    InitialHelpersMismatchesTestCase() : TestCase("initial-helpers mismatches") {};

    void DoRun() {
        ASSERT_EXCEPTION_MATCH_WHAT(
                GetInitialUintValue("ns3::ClassWhichDoesNotExist", "AttA"),
                "Type not found: ns3::ClassWhichDoesNotExist"
                );
        ASSERT_EXCEPTION_MATCH_WHAT(
                GetInitialUintValue("ns3::ClassWithAnAttribute", "AttE"),
                "Type ns3::ClassWithAnAttribute does not have an attribute named AttE"
                );
        ASSERT_EXCEPTION_MATCH_WHAT(
                GetInitialUintValue("ns3::ClassWithAnAttribute", "AttD"),
                "Value provided for ns3::ClassWithAnAttribute::AttD is not an unsigned integer"
                );
        ASSERT_EXCEPTION_MATCH_WHAT(
                GetInitialBooleanValue("ns3::ClassWithAnAttribute", "AttC"),
                "Value provided for ns3::ClassWithAnAttribute::AttC is not a boolean"
                );
        ASSERT_EXCEPTION_MATCH_WHAT(
                GetInitialDoubleValue("ns3::ClassWithAnAttribute", "AttB"),
                "Value provided for ns3::ClassWithAnAttribute::AttB is not a double"
                );
        ASSERT_EXCEPTION_MATCH_WHAT(
                GetInitialTimeValue("ns3::ClassWithAnAttribute", "AttA"),
                "Value provided for ns3::ClassWithAnAttribute::AttA is not a time type"
                );
    }
};

////////////////////////////////////////////////////////////////////////////////////////

# -*- Mode: python; py-indent-offset: 4; indent-tabs-mode: nil; coding: utf-8; -*-

def build(bld):

    # Register 'basic-sim' module with dependencies
    module = bld.create_ns3_module('basic-sim', ['core', 'internet', 'applications', 'point-to-point', 'traffic-control'])

    # Source files
    module.source = [
        'model/core/basic-simulation.cc',
        'model/core/exp-util.cc',
        'model/core/tcp-optimizer.cc',
        'model/core/topology-ptop.cc',
        'model/core/arbiter.cc',
        'model/core/arbiter-ptop.cc',
        'model/core/arbiter-ecmp.cc',
        'model/core/ipv4-arbiter-routing.cc',
        'model/core/ptop-utilization-tracker.cc',

        'helper/core/arbiter-ecmp-helper.cc',
        'helper/core/ipv4-arbiter-routing-helper.cc',
        'helper/core/ptop-utilization-tracker-helper.cc',

        'model/apps/flow-send-application.cc',
        'model/apps/flow-sink.cc',
        'model/apps/schedule-reader.cc',
        'model/apps/flow-scheduler.cc',
        'model/apps/pingmesh-scheduler.cc',
        'model/apps/udp-rtt-client.cc',
        'model/apps/udp-rtt-server.cc',

        'helper/apps/flow-send-helper.cc',
        'helper/apps/flow-sink-helper.cc',
        'helper/apps/udp-rtt-helper.cc',
        ]

    # Header files
    headers = bld(features='ns3header')
    headers.module = 'basic-sim'
    headers.source = [
        'model/core/basic-simulation.h',
        'model/core/exp-util.h',
        'model/core/tcp-optimizer.h',
        'model/core/topology.h',
        'model/core/topology-ptop.h',
        'model/core/arbiter.h',
        'model/core/arbiter-ptop.h',
        'model/core/arbiter-ecmp.h',
        'model/core/ipv4-arbiter-routing.h',
        'model/core/ptop-utilization-tracker.h',

        'helper/core/arbiter-ecmp-helper.h',
        'helper/core/ipv4-arbiter-routing-helper.h',
        'helper/core/ptop-utilization-tracker-helper.h',

        'model/apps/flow-send-application.h',
        'model/apps/flow-sink.h',
        'model/apps/schedule-reader.h',
        'model/apps/flow-scheduler.h',
        'model/apps/pingmesh-scheduler.h',
        'model/apps/udp-rtt-client.h',
        'model/apps/udp-rtt-server.h',

        'helper/apps/flow-send-helper.h',
        'helper/apps/flow-sink-helper.h',
        'helper/apps/udp-rtt-helper.h',
        ]

    # Tests
    module_test = bld.create_ns3_module_test_library('basic-sim')
    module_test.source = [
        'test/basic-sim-test-suite.cc',
        ]

    # Examples
    if bld.env.ENABLE_EXAMPLES:
        bld.recurse('examples')

    # For now, no Python bindings are generated
    # bld.ns3_python_bindings()

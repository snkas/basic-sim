# -*- Mode: python; py-indent-offset: 4; indent-tabs-mode: nil; coding: utf-8; -*-

def build(bld):

    # Register 'basic-sim' module with dependencies
    module = bld.create_ns3_module('basic-sim', ['core', 'internet', 'applications', 'point-to-point', 'traffic-control'])

    # Source files
    module.source = [

        'model/core/exp-util.cc',
        'model/core/basic-simulation.cc',
        'model/core/topology-ptop.cc',
        'model/core/topology-ptop-queue-selector-default.cc',
        'model/core/topology-ptop-receive-error-model-selector-default.cc',
        'model/core/topology-ptop-tc-qdisc-selector-default.cc',
        'model/core/arbiter.cc',
        'model/core/arbiter-ptop.cc',
        'model/core/arbiter-ecmp.cc',
        'model/core/ipv4-arbiter-routing.cc',

        'model/core/net-device-utilization-tracker.cc',
        'model/core/queue-tracker.cc',
        'model/core/qdisc-queue-tracker.cc',

        'helper/core/arbiter-ecmp-helper.cc',
        'helper/core/ipv4-arbiter-routing-helper.cc',
        'helper/core/ptop-link-net-device-utilization-tracking.cc',
        'helper/core/ptop-link-net-device-queue-tracking.cc',
        'helper/core/ptop-link-interface-tc-qdisc-queue-tracking.cc',
        'helper/core/initial-helpers.cc',
        'helper/core/point-to-point-ab-helper.cc',

        'model/apps/socket-generator.cc',
        'model/apps/ip-tos-generator.cc',

        'model/apps/tcp-flow-client.cc',
        'model/apps/tcp-flow-server.cc',
        'model/apps/udp-burst-header.cc',
        'model/apps/udp-burst-server.cc',
        'model/apps/udp-burst-client.cc',
        'model/apps/udp-ping-header.cc',
        'model/apps/udp-ping-server.cc',
        'model/apps/udp-ping-client.cc',

        'helper/apps/client-remote-port-selector.cc',
        'helper/apps/tcp-flow-helper.cc',
        'helper/apps/tcp-flow-schedule-reader.cc',
        'helper/apps/tcp-flow-scheduler.cc',
        'helper/apps/udp-burst-helper.cc',
        'helper/apps/udp-burst-schedule-reader.cc',
        'helper/apps/udp-burst-scheduler.cc',
        'helper/apps/udp-ping-helper.cc',
        'helper/apps/udp-ping-schedule-reader.cc',
        'helper/apps/udp-ping-scheduler.cc',
        'helper/apps/tcp-config-helper.cc',

        ]

    # Header files
    headers = bld(features='ns3header')
    headers.module = 'basic-sim'
    headers.source = [

        'model/core/log-update-helper.h',
        'model/core/exp-util.h',
        'model/core/basic-simulation.h',
        'model/core/topology.h',
        'model/core/topology-ptop.h',
        'model/core/topology-ptop-queue-selector-default.h',
        'model/core/topology-ptop-receive-error-model-selector-default.h',
        'model/core/topology-ptop-tc-qdisc-selector-default.h',
        'model/core/arbiter.h',
        'model/core/arbiter-ptop.h',
        'model/core/arbiter-ecmp.h',
        'model/core/ipv4-arbiter-routing.h',

        'model/core/net-device-utilization-tracker.h',
        'model/core/queue-tracker.h',
        'model/core/qdisc-queue-tracker.h',

        'helper/core/arbiter-ecmp-helper.h',
        'helper/core/ipv4-arbiter-routing-helper.h',
        'helper/core/ptop-link-net-device-utilization-tracking.h',
        'helper/core/ptop-link-net-device-queue-tracking.h',
        'helper/core/ptop-link-interface-tc-qdisc-queue-tracking.h',
        'helper/core/initial-helpers.h',
        'helper/core/point-to-point-ab-helper.h',

        'model/apps/socket-generator.h',
        'model/apps/ip-tos-generator.h',

        'model/apps/tcp-flow-client.h',
        'model/apps/tcp-flow-server.h',
        'model/apps/udp-burst-header.h',
        'model/apps/udp-burst-server.h',
        'model/apps/udp-burst-client.h',
        'model/apps/udp-ping-header.h',
        'model/apps/udp-ping-server.h',
        'model/apps/udp-ping-client.h',

        'helper/apps/client-remote-port-selector.h',
        'helper/apps/tcp-flow-helper.h',
        'helper/apps/tcp-flow-schedule-reader.h',
        'helper/apps/tcp-flow-scheduler.h',
        'helper/apps/udp-burst-helper.h',
        'helper/apps/udp-burst-schedule-reader.h',
        'helper/apps/udp-burst-scheduler.h',
        'helper/apps/udp-ping-helper.h',
        'helper/apps/udp-ping-schedule-reader.h',
        'helper/apps/udp-ping-scheduler.h',
        'helper/apps/tcp-config-helper.h',

        ]

    # Tests
    module_test = bld.create_ns3_module_test_library('basic-sim')
    module_test.source = [

        'test/test-helpers.cc',
        'test/test-case-with-log-validators.cc',

        'test/core-basic-simulation-test-suite.cc',
        'test/core-exp-util-test-suite.cc',
        'test/core-log-update-helper-test-suite.cc',
        'test/core-ptop-test-suite.cc',
        'test/core-ptop-tracking-test-suite.cc',
        'test/core-arbiter-test-suite.cc',

        'test/apps-initial-helpers-test-suite.cc',
        'test/apps-manual-test-suite.cc',
        'test/apps-tcp-flow-test-suite.cc',
        'test/apps-udp-burst-test-suite.cc',
        'test/apps-udp-ping-test-suite.cc',
        'test/apps-tcp-config-helper-test-suite.cc',

        ]

    # Main
    bld.recurse('main')

    # Examples
    if bld.env.ENABLE_EXAMPLES:
        bld.recurse('examples')

    # For now, no Python bindings are generated
    # bld.ns3_python_bindings()

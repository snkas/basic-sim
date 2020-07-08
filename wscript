# -*- Mode: python; py-indent-offset: 4; indent-tabs-mode: nil; coding: utf-8; -*-

# def options(opt):
#     pass

# def configure(conf):
#     conf.check_nonfatal(header_name='stdint.h', define_name='HAVE_STDINT_H')

def build(bld):
    module = bld.create_ns3_module('basic-sim', ['core', 'internet', 'applications', 'point-to-point', 'traffic-control'])
    module.source = [
        'model/basic-simulation.cc',
        'model/exp-util.cc',
        'model/tcp-optimizer.cc',
        'model/topology-ptop.cc',
        'model/arbiter.cc',
        'model/arbiter-ptop.cc',
        'model/arbiter-ecmp.cc',
        'helper/arbiter-ecmp-helper.cc',
        'model/ipv4-arbiter-routing.cc',
        'helper/ipv4-arbiter-routing-helper.cc',
        ]

    module_test = bld.create_ns3_module_test_library('basic-sim')
    module_test.source = [
        'test/basic-sim-test-suite.cc'
        ]

    headers = bld(features='ns3header')
    headers.module = 'basic-sim'
    headers.source = [
        'model/basic-simulation.h',
        'model/exp-util.h',
        'model/tcp-optimizer.h',
        'model/topology.h',
        'model/topology-ptop.h',
        'model/arbiter.h',
        'model/arbiter-ptop.h',
        'model/arbiter-ecmp.h',
        'helper/arbiter-ecmp-helper.h',
        'model/ipv4-arbiter-routing.h',
        'helper/ipv4-arbiter-routing-helper.h',
        ]

    if bld.env.ENABLE_EXAMPLES:
        bld.recurse('examples')

    # bld.ns3_python_bindings()


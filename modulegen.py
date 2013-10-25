#! /usr/bin/env python

import sys

import pybindgen
from pybindgen import ReturnValue, Parameter, Module, Function, FileCodeSink

def my_module_gen(out_file):

    mod = Module('_DMDomain')
    mod.add_include('"DMDomain.h"')

    mod.add_function('ValidateDomainString', None, [
         Parameter.new('char*', 'DomainStr', transfer_ownership=False),
         Parameter.new('char', 'DomainType'),
         Parameter.new('char', 'DomainLen')])

    mod.add_function('DMDomainMatch', 'char', [
         Parameter.new('char*', 'ControlStr', transfer_ownership=False),
         Parameter.new('char*', 'MatchStr', transfer_ownership=False),
         Parameter.new('char', 'ControlLen')])

    mod.generate(FileCodeSink(out_file) )

if __name__ == '__main__':
    my_module_gen(sys.stdout)

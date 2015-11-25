#!/usr/bin/env python
# encoding: utf-8

import subprocess
import argparse
import string
import random
import codecs
import sys
import re
import os


TEST_LINE = re.compile(".*//~\s*(?P<type>\w*)\s*(?P<code>\d{4}).*")
COMPILER_ERROR = re.compile(
    "\s*ERROR\s+(?P<line>\d+)\,(?P<column>\d+)\s+(?P<code>\d{4}).*")


class TestInfo():

    def __init__(self, folder, compiler, filename):
        self.folder = folder
        self.filename = filename
        self.compiler = compiler
        self.options = []
        self.errors = []

    def add_error(self, line, code):
        self.errors.append({'line': line, 'code': code, 'marked': False})

    def mark_error(self, line, code):
        for error in self.errors:
            if error['line'] == line and error['code'] == code:
                error['marked'] = True
                return

    def has_unmarked_erros(self):
        return bool(len([e for e in self.errors if not e['marked']]))

    def get_run_command(self):
        return "{} {}/{}".format(self.compiler, self.folder, self.filename)

    def __str__(self):
        return "TestInfo {}/{} errors:\n{}".format(
            self.folder, self.filename, "\n".join(str(d) for d in self.errors))


def dig_file(info):
    lc = 0
    with (open(os.path.join(info.folder, info.filename), 'r')) as f:
        for line in f:
            lc = lc + 1
            m = TEST_LINE.match(line)
            if m:
                info.add_error(str(lc), m.group('code'))


def test_parse(env):
    path = '../parse-fail'
    for f in (f for f in os.listdir(path) if os.path.isfile(os.path.join(path, f))):
        info = TestInfo(path, env['compiler'], f)
        dig_file(info)

        output = None
        try:
            print "Running test {:.<80}".format(f),
            output = subprocess.check_output(
                info.get_run_command(), shell=True)
        except subprocess.CalledProcessError as e:
            output = e.output

        if not output:
            print 'Fail'
            continue

        for line in output.splitlines():
            m = COMPILER_ERROR.match(line)
            if not m:
                continue
            line = m.group('line')
            code = m.group('code')
            info.mark_error(line, code)

        if info.has_unmarked_erros():
            print 'Fail\n{}'.format(info)
            print 'Compiler output:\n{}'.format(output)
        else:
            print 'OK'

#
# Actions
#


def test(options):
    env = {'compiler': options.compiler}
    for target in options.targets:
        if target == 'parse':
            test_parse(env)
    pass

#
# Menu
#

parser = argparse.ArgumentParser(prog='htr', description='Helium test runner')
subparsers = parser.add_subparsers(title='Actions', dest='action')

#
# Run
parser_test = subparsers.add_parser('test', help='')
parser_test.add_argument('compiler', metavar='COMPILER', help='')
parser_test.add_argument('-t', '--targets', metavar='TARGET',  nargs='+',
                         default=['parse'],
                         choices=['parse', 'compile', 'run'],
                         help='')
parser_test.set_defaults(func=test)

options = parser.parse_args()
options.func(options)


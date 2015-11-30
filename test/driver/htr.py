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


FLAG_LINE = re.compile(".*(//|/\*|/\*\*|\*)\s*compile-flags:\s*(?P<flags>.*)")
TEST_LINE = re.compile(".*//~\s*(?P<type>\w*)\s*(?P<code>\d{4,}).*")
COMPILER_ERROR = re.compile(
    "\s*ERROR\s+(?P<line>\d+)\,(?P<column>\d+)\s+(?P<code>\d{4,}).*")
RUNTIME_ERROR = re.compile(
    "\s*EXIT\s+(?P<line>\d+)\,(?P<column>\d+)\s+(?P<code>\d{4,}).*")


class TestInfo():

    def __init__(self, compiler, folder, filename):
        self.compiler = compiler
        self.folder = folder
        self.filename = filename
        self.flags = []
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

    def add_flags(self, option):
        self.flags.append(option)

    def get_compile_command(self):
        return "{} {}/{} {}".format(
            self.compiler,
            self.folder,
            self.filename,
            " ".join(self.flags))

    def __str__(self):
        return "TestInfo {}/{}\nerrors:\n{}\ncompile-flags:\n{}".format(
            self.folder,
            self.filename,
            "\n".join(str(d) for d in self.errors),
            " ".join(self.flags))


class SPIMRunTestInfo(TestInfo):

    def __init__(self, compiler, spim, folder, filename):
        TestInfo.__init__(self, compiler, folder, filename)
        self.spim = spim

    def get_run_command(self):
        return "{} -f {}/{}".format(
            self.spim,
            self.folder,
            self.filename)


def dig_file(info):
    lc = 0
    with (open(os.path.join(info.folder, info.filename), 'r')) as f:
        for line in f:
            lc = lc + 1

            f = FLAG_LINE.match(line)
            if f:
                info.add_flags(f.group('flags'))

            t = TEST_LINE.match(line)
            if t:
                info.add_error(str(lc), t.group('code'))


def test_parse_fail(env):
    test_folder = os.path.join(env.test_folder, 'parse-fail')
    for f in (f for f in os.listdir(test_folder) if os.path.isfile(os.path.join(test_folder, f))):
        info = TestInfo(env.compiler, test_folder, f)
        dig_file(info)

        output = None
        try:
            print "Running {:.<80}".format(os.path.join(test_folder, f)),
            output = subprocess.check_output(
                info.get_compile_command(), shell=True)
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
            print 'Compile command:\n{}'.format(info.get_compile_command())
            print 'Compiler output:\n{}'.format(output)
        else:
            print 'OK'


def test_run_fail(env):
    test_folder = os.path.join(env.test_folder, 'run-fail')
    temp_folder = os.path.join(env.temp_folder, 'run-fail')

    if not os.path.exists(temp_folder):
        os.makedirs(temp_folder)

    for f in os.listdir(test_folder):

        fullname = os.path.join(test_folder, f)

        if not os.path.isfile(fullname):
            continue

        outfile = "{}/{}".format(temp_folder, f.replace(".he", ".s"))

        info = SPIMRunTestInfo(env.compiler, "spim", test_folder, f)
        info.add_flags("-o{}".format(outfile))
        dig_file(info)

        output = None
        print "Running {:.<80}".format(fullname),
        try:
            output = subprocess.check_output(
                info.get_compile_command(), shell=True)
        except subprocess.CalledProcessError as e:
            output = e.output

        if not output:
            print 'Fail'
            continue

        output = None
        error = None
        try:
            output = subprocess.check_output(
                "spim -f {}".format(outfile), shell=True)
        except subprocess.CalledProcessError as e:
            error = e

        # We test for failing here
        if not error:
            print "Fail"
            continue

        # Making sure every error mentioned in the snippet is met
        for line in error.output.splitlines():
            m = RUNTIME_ERROR.match(line)
            if not m:
                continue
            line = m.group('line')
            code = m.group('code')
            info.mark_error(line, code)

        if info.has_unmarked_erros():
            print 'Fail\n{}'.format(info)
            print 'Compile command:\n{}'.format(info.get_compile_command())
            print 'Compiler output:\n{}'.format(output)
            print 'Run ouput:\n{}'.format(error.output)
        else:
            print 'OK'

#
# Actions
#


def test(options):
    for target in options.targets:
        if target == 'parse-fail':
            test_parse_fail(options)
        elif target == 'run-fail':
            test_run_fail(options)

#
# Menu
#

parser = argparse.ArgumentParser(prog='htr', description='Helium test runner')
subparsers = parser.add_subparsers(title='Actions', dest='action')

#
# Test
parser_test = subparsers.add_parser('test', help='')
parser_test.add_argument('test_folder', metavar='TEST_FOLDER', help='')
parser_test.add_argument('temp_folder', metavar='TEMP_FOLDER', help='')
parser_test.add_argument('compiler', metavar='COMPILER', help='')
parser_test.add_argument('-t', '--targets', metavar='TARGET',  nargs='+',
                         default=['parse-fail'],
                         choices=['parse-fail', 'compile-fail',
                                  'run-pass', 'run-fail'],
                         help='')
parser_test.set_defaults(func=test)

options = parser.parse_args()
options.func(options)


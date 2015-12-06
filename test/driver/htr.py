#!/usr/bin/env python
# encoding: utf-8

from enum import Enum
import subprocess
import argparse
import string
import random
import codecs
import sys
import re
import os


FLAG_LINE = re.compile(".*(//|/\*|/\*\*|\*)\s*compile-flags:\s*(?P<flags>.*)")
CODE_LINE = re.compile(".*(//|/\*|/\*\*|\*)\s*return-code:\s*(?P<code>\d+).*")
TEST_LINE = re.compile(".*//~\s*(?P<type>\w*)\s*(?P<code>\d{4,}).*")
TEST_PREV_LINE = re.compile(".*//(?P<prev>\^+)\s*(?P<type>\w*)\s*(?P<code>\d{4,}).*")
COMPILER_ERROR = re.compile(
    "\s*ERROR\s+(?P<line>\d+)\,(?P<column>\d+)\s+(?P<code>\d{4,}).*")
RUNTIME_ERROR = re.compile(
    "\s*EXIT\s+(?P<line>\d+)\,(?P<column>\d+)\s+(?P<code>\d{4,}).*")


class Runner():

    def __init__(self, command, re, is_reversed):
        self.command = command
        self.re = re
        self.is_reversed = is_reversed

    def run(self):
        self.code = 0
        self.output = ""
        try:
            self.output = subprocess.check_output(
                self.command, stderr=subprocess.STDOUT, shell=True)
        except subprocess.CalledProcessError as e:
            self.code = e.returncode
            self.output = e.output

    def is_ok(self):
        result = self.code == 0
        if self.is_reversed:
            result = not result
        return result

    def get_output(self):
        return self.output


class Test():

    is_fail = False
    expected_code = None

    def __init__(self, test_folder, temp_folder, filename):
        self.temp_folder = temp_folder
        self.test_folder = test_folder
        self.filename = filename
        self.fullname = os.path.join(test_folder, filename)
        self.compile_flags = []
        self.execute_flags = []
        self.expectations = []
        self.runners = []

    def dig(self):
        lc = 0
        with (open(self.fullname, 'r')) as f:
            for line in f:
                lc = lc + 1

                t = CODE_LINE.match(line)
                if t:
                    self.expected_code = int(t.group('code'))

                f = FLAG_LINE.match(line)
                if f:
                    # TODO split
                    self.add_compile_flag(f.group('flags'))

                t = TEST_LINE.match(line)
                if t:
                    self.add_expectation(lc, t.group('code'))

                t = TEST_PREV_LINE.match(line)
                if t:
                    self.add_expectation(lc - len(t.group('prev')), t.group('code'))

    def add_expectation(self, line, code):
        self.expectations.append(
            {'line': int(line), 'code': int(code), 'marked': False})

    def mark_expectation(self, line, code):
        for e in self.expectations:
            if e['line'] == int(line) and e['code'] == int(code):
                e['marked'] = True
                return True
        return False

    def has_unfulfilled(self):
        return bool(len([e for e in self.expectations if not e['marked']]))

    def add_compile_flag(self, option):
        self.compile_flags.append(option)

    def get_compile_flags(self):
        return "{} {}".format(
            self.fullname,
            " ".join(self.compile_flags))

    def add_runner(self, runner):
        self.runners.append(runner)

    def run(self):
        print "Running {:.<80}".format(self.fullname),

        if (self.is_fail
                and len(self.expectations) == 0
                and self.expected_code == None):
            print 'Mal'
            return

        for r in self.runners:
            r.run()
            if not r.is_ok():
                print 'Fail'
                print 'Not OK'
                print 'Output:\n{}'.format(r.output)
                print self
                return

            # Check the output for errors
            for line in r.output.splitlines():
                m = r.re.match(line)
                if not m:
                    continue
                lnum = m.group('line')
                code = m.group('code')
                if not self.mark_expectation(lnum, code):
                    print "Fail"
                    print "Unexpected error: {}".format(line)
                    print self

        if self.has_unfulfilled():
            print 'Fail'
            print 'Unfulfilled expectations\n'
            print self
            return

        # Check the return code
        if self.expected_code != None and self.expected_code != r.code:
            print 'Fail\n{}\nReason: {}\nOutput:\n{}'.format(
                self,
                "wrong exit code; expected {}, got {}".format(
                    self.expected_code,
                    r.code),
                r.output)
            return

        print "OK"

    def __str__(self):
        return "Test {}\ncompile-flags: {}\nexpectations:\n{}".format(
            self.fullname,
            self.compile_flags,
            "\n".join(str(d) for d in self.expectations),
            " ".join(self.compile_flags))

#
# Actions


def test(options):
    for target in options.targets:

        test_folder = os.path.join(options.test_folder, target)
        temp_folder = os.path.join(options.temp_folder, target)

        if not os.path.exists(temp_folder):
            os.makedirs(temp_folder)

        for f in os.listdir(test_folder):

            source_fullname = os.path.join(test_folder, f)
            output_fullname = os.path.join(temp_folder, f.replace(".he", ".s"))

            test = Test(test_folder, temp_folder, f)
            test.add_compile_flag("-o{}".format(output_fullname))
            test.dig()

            if target == 'parse-fail':
                test.is_fail = True
                test.add_compile_flag("-Zparse-only")
                test.add_runner(Runner("{} {}".format(
                    options.compiler,
                    test.get_compile_flags()),
                    COMPILER_ERROR,
                    False))

            elif target == 'compile-fail':
                test.is_fail = True
                test.add_runner(
                    Runner("{} {}".format(
                        options.compiler,
                        test.get_compile_flags()),
                        COMPILER_ERROR,
                        False))

            elif target == 'run-fail':
                test.is_fail = True
                test.add_runner(
                    Runner("{} {}".format(
                        options.compiler,
                        test.get_compile_flags()),
                        COMPILER_ERROR,
                        False))
                test.add_runner(
                    Runner("spim -f {}".format(output_fullname),
                           RUNTIME_ERROR,
                           True))

            elif target == 'run-pass':
                test.add_runner(
                    Runner("{} {}".format(
                        options.compiler,
                        test.get_compile_flags()),
                        COMPILER_ERROR,
                        False))
                test.add_runner(
                    Runner("spim -f {}".format(output_fullname),
                           RUNTIME_ERROR,
                           False))

            test.run()

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


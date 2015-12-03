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
TEST_LINE = re.compile(".*//~\s*(?P<type>\w*)\s*(?P<code>\d{4,}).*")
COMPILER_ERROR = re.compile(
    "\s*ERROR\s+(?P<line>\d+)\,(?P<column>\d+)\s+(?P<code>\d{4,}).*")
RUNTIME_ERROR = re.compile(
    "\s*EXIT\s+(?P<line>\d+)\,(?P<column>\d+)\s+(?P<code>\d{4,}).*")


class Runner():

    def __init__(self, command, is_reversed):
        self.command = command
        self.is_reversed = is_reversed

    def run(self):
        self.code = 0
        self.output = ""
        try:
            output = subprocess.check_output(
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

                f = FLAG_LINE.match(line)
                if f:
                    # TODO split
                    self.add_compile_flag(f.group('flags'))

                t = TEST_LINE.match(line)
                if t:
                    self.add_expectation(str(lc), t.group('code'))

    def add_expectation(self, line, code):
        self.expectations.append({'line': line, 'code': code, 'marked': False})

    def mark_expectation(self, line, code):
        for e in self.expectations:
            if e['line'] == line and e['code'] == code:
                e['fulfilled'] = True
                return

    def has_unfulfilled(self):
        return bool(len([e for e in self.expectations if not e['fulfilled']]))

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

        for r in self.runners:
            r.run()
            if not r.is_ok():
                print 'Fail\n{}\nOutput:\n{}'.format(
                    self,
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
#


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
                test.add_runner(Runner("{} {}".format(
                    options.compiler,
                    test.get_compile_flags()),
                    False))

            elif target == 'run-fail':
                test.add_runner(
                    Runner("{} {}".format(
                        options.compiler,
                        test.get_compile_flags()),
                        False))
                test.add_runner(
                    Runner("spim -f {}".format(output_fullname), True))

            elif target == 'run-pass':
                test.add_runner(
                    Runner("{} {}".format(
                        options.compiler,
                        test.get_compile_flags()),
                        False))
                test.add_runner(
                    Runner("spim -f {}".format(output_fullname), False))

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


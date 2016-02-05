# Helium
Helium is a toy system programming language and compiler.
Work in progress...

## Wiki
Is [here](https://github.com/m4yers/helium/wiki)

## Build

Things to have:
 - [CMake](https://cmake.org/) to build
 - [CMocka](https://cmocka.org/) to test
 - GNU Argp library for compiler argument parsing. It is not standard for Mac distributions so you
 can get a standalone version [here](https://github.com/jahrome/argp-standalone).
 - Flex and Bison to generate lexer and parser

```bash
./create.sh
./build.sh
./run_tests.sh
```

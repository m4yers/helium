# TODO
- [x] re-do the stack frame lowering
- [x] sort things with static links
    - [x] restore fp -> sp and stack -> fp
    - [x] test it
- [x] new front-end
- [x] BIG CLEAN UP
    - [x] module-based debug
    - [x] proper front-end error message system
- [x] new test system
    - [x] internals
    - [x] syntax
    - [x] semant
    - [x] runtime
    - [ ] codegen? -> postponed till optimization
- [ ] tests for existing functionality
- [ ] clear definion of what is:
    - [ ] undefined
    - [ ] malformed
    - [ ] invalid
    - [ ] unknown
- [ ] tests for list.h/.c
- [ ] ir printer
- [ ] language features
    - [ ] asm
    - [ ] pointers
    - [ ] strings
    - [ ] boolean
    - [ ] various loops
    - [ ] tuples
    - [ ] unions
    - [ ] packages
    - [ ] macro
    - [ ] static storage
    - [ ] floats?
- [ ] clean-up error codes
- [ ] minimal stdlib
    - [ ] io
    - [ ] mem
    - [ ] heap
    - [ ] math
- [ ] optimizations
    - [ ] drop dummy moves
    - [ ] drop dummy jumps
    - [ ] SSA
    - [ ] Loops
    - [ ] stack frame space ~ free registers, e.g. $fp and $ra and leaf/non-leaf stack layout
    - [ ] remove unnecessary storing static-link to stack if not used or used immediately
- [ ] BIG CLEAN UP
    - [ ] make sure the compiler can be built straight from the repo
    - [ ] Memory usage/freeing
    - [ ] Subsystems split akin LLVM
- [ ] C runtime interface
- [ ] functions' specifications -> tempesting
- [ ] lexer/parser/semant error recovery

 ...

- [ ] x86
- [ ] ARM?

 ...

- [ ] self-hoisting

# QUESTIONS
- classes, to be or not to be?

# THOUGTS
- $fp set to $sp - wordsize? Now it points to $a0
- make blocks yield a valid result, e.g:

  let c = {
      let a = 100;
      let b = 500;
      ret a + b;
  }

- make if,while,for as expression that yields a valid result, e.g:

  let f = if (blah) { true; } else { false; };

  let f = while(b > 0) { b--; }





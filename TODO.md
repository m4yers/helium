# TODO
- [x] re-do the stack frame lowering
- [x] sort things with static links
    - [x] restore fp -> sp and stack -> fp
    - [x] test it
- [x] new front-end
- [ ] BIG CLEAN UP
    - [x] module-based debug
    - [ ] proper front-end error system
- [ ] new test system
    - [ ] syntax
    - [ ] semant
    - [ ] codegen?
    - [ ] runtime
- [ ] lang
    - [ ] pointers
    - [ ] strings
    - [ ] boolean
    - [ ] various loops
    - [ ] tuples
    - [ ] unions
    - [ ] packages
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
    - [ ] Memory usage/freeing
    - [ ] Subsystems split akin LLVM
- [ ] C runtime interface
- [ ] functions' specifications -> tempesting

- [ ] ...

- [ ] self-hoisting?

# THOUGTS
- $fp set to $sp - wordsize? Now it points to $a0

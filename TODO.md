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
- [x] tests for existing functionality
- [ ] solve typedef vs actual type hell
- [ ] better semant error system
- [ ] clear definition on what is:
    - [ ] undefined
    - [ ] malformed
    - [ ] invalid
    - [ ] unknown
- [x] tests for list.h/.c
- [ ] ir printer
- [ ] language features
    - [x] pointers
    - [x] type cast
    - [x] strings
    - [x] asm base
    - [x] asm extended
    - [ ] aliasing
    - [ ] proper macro
    - [ ] boolean
    - [ ] ranges
    - [ ] various loops
    - [ ] tuples
    - [ ] unions
    - [ ] packages
    - [ ] static storage
    - [ ] proper int ranges
    - [ ] array and record function results
    - [ ] floats?
- [ ] better type inference using "meta types tree"
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
- [ ] templates
- [ ] function overloading
- [ ] lexer/parser/semant error recovery

 ...

- [ ] x86
- [ ] ARM?

 ...

- [ ] self-hoisting

# BUGS
- [x] equal literals same location, does it work?
- [x] add asm features context-wise validation
- [x] function empty body
- [x] diffrence between stm and exp and function edge
- [ ] different types of multiple rets, including local type inference(LTI)
- [ ] function and types preproc pass to map names to definitions
- [ ] integer range validation
- [ ] arrays and records on function boundries
- [ ] str vs &str, what happens to str?
- [ ] assert! is somehow broken
- [ ] default values for array

# QUESTIONS
- classes, to be or not to be?

# THOUGTS
- $fp set to $sp - wordsize? Now it points to $a0
- make blocks yield a valid result, e.g:

  let c = {
      let a = 100;
      let b = 500;
      a + b;
  }

- make if,while,for as expression that yields a valid result, e.g:

  let f = if (blah) { true; } else { false; };

  let f = while(b > 0) { b--; }

- every traverse level need to know exactly where it belongs to: left side or right or none side of
assignment, this will tell whether blocks used as statements or as expressions. Also this helps in
general do deside whether a value must be fetched e.g.:

    a = b = b + 1;

here, b must be fetched again after assignment, but a should not. Though, clever asm optimizer can
follow def/use graph and delte all the shit.

References:

    let a = { x = 0, y = 10 };
    let ref b = &a;

    b.x = 10;

';'
Follow RUST approach: if the last exp in function is not followed by the ';' it is interpreted as
ret exp;

Error handling:
i need to skip whole statement if an error is encountered but continue parse definitions, since they
affect next statements

Array pointers:

    let a = [1,2,3];
    let b = &a;
    let c = *b;

    let d = b:0;

** remove difference between var and exp, though i need keep in mind lhs and rhs differences

Liternals -> remove old stuff and do convertion in semant

    asm (mips,volatile;x;func())
    {
        addi `d0, $zero, `s0
    }

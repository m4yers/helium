# Grammar
# Types
## Boolean
## Integer
## Real
## Array
A fixed-size stack-based array, denoted [T; N], for the element type, T, and the non-negative compile time constant size, N. The are three ways to create a variable of array type, the first is to declare array var type explicitly, the second is to use auto type inferring from an array expression, and the last is to use typedef of array type.

Examples:
---
```
// explicit declaration
let a: [int;3];
a[0] = 10;
a[1] = 11;
a[2] = 12;

// type inferring
let b = [1, 2, 3];

// array type def
def Arr = [int;3]
let c: Arr;
```
Note, however, that the first and the second variables in the example share the same array type of 3 ints, but the third one defined its another type which is based on array of 3 ints but is not strictly equal to it.
## Record
Records is the way to define custom types. As with the arrays there are three ways to declare a variable of record type.

Examples:
---
```
// explicit declaration
let a: { x:int, y: int };
a.x = 10;
a.y = 11;

// type inferring
let b = { x = 10, y = 11 };

// record typedef
def Point = { x:int, y:int }
let c: Point;
```
Again, note that first two variables share the same anonymous record type of two int fields, but the third one defined its own custom type Point which is compatible but not strictly equal with the anonymous type of the first two variables.    
## Union
## Tuple
## Pointer
## Macro
## Typedefs

# Packaging
Every program has at minimum a single package called main with a function called main, this is the
entry point of the program. Package can span several source files, called _unit_. A unit may
contain basically anything, by default _all_ symbols inside unit are visible inside unit only. In
order to make symbols visible outside the package they must be exported

## Importing
```
import <package>              # imports package's namespace into current scope
import <package>.<symbol>     # brings a specific symbol into current namespace
import <package>.*            # brings all symbols into current namespace
```

## Standard packages

### io
#### io.print
#### io.println
#### io.fprint

### mem
#### mem.copy
#### mem.move

### heap
#### heap.create - create allocator
#### heap.destroy - destroy allocator
#### heap.alloc
#### heap.realloc
#### heap.free
#### heap.status

### math

# Minimal program
```
package main

fn main
{
    ret 0;
}
```

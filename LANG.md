# Grammar
# Types
## Boolean
## Integer
## Real
## Record
## Union
## Array
## Tuple
## Pointer

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

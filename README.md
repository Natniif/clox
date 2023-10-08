# Clox

This repository is for educatinoal purposes and is my implementation of the clox language described in the [crafting interpreters](http://craftinginterpreters.com/contents.html) book. 

Each branch represents each chapter in the book so you can see how the book builds up the interpreter. 

Syntax is similar to C and is intended to work as an object oriented language (with classes implementation coming soon!) 

```C
message = "Hello world!";
isTrue = true;

if (message && isTrue) {
  print message; // "Hello World!" 
}
```

## Usage

```
make

// run as script file
main [file]

// repl mode
main
```

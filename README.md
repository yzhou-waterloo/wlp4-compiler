# wlp4-compiler
A simple compiler made for the WLP4 language a simplified, C-like language designed for educational purposes. This project was developed as part of the CS 241: Foundations of Sequential Programs course.


Overview

The compiler takes WLP4 source code and produces MIPS assembly code.
It implements the full compilation pipeline:

Lexical Analysis 

Parsing (SLR(1))

Semantic Analysis (type checking, type annotation)

Code Generation (MIPS assembly)


Source Language: Wlp4
Target Language: MIPS Assembly
Key Features:

Supports int and int* data types

Control flow: if, else, while

Functions with parameters and return values

Static type checking


Requirements

Language: C++17

Build Tools: make

Dependencies:


GCC or Clang

Build Instructions
# From project root
make

This will produce an executable named wlp4compiler

Usage
./wlp4compiler <input_file> [options]


Authors

Yang Zhou

Course: CS 241 — Foundations of Sequential Programs

University of Waterloo

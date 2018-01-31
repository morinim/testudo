# TESTUDO

## Overview

Testudo is a simple chess engine written in ISO C++14.

It's distributed under the [Mozilla Public License v2.0][2] (see the accompanying [LICENSE][3] file for more details).

## Design goals

- Simplicity / Clearness
- Correctness
- Simple interfacing with [Vita][1]

## Purpose

- To establish a flexible test-bed for Artificial Intelligence / Machine Learning techniques
- Easy understanding of a not-so-trivial implementation

## Features

- 10x12 mailbox board representation (piece type and colour encoding) 
- Alpha-beta with aspiration search
- Quiescence search
- Evaluation based on material, piece square tables
- [CECP v2][4] support

## Build requirements

A modern C++ compiler and `cmake` (https://cmake.org/).

Just type:

```shell
cd testudo/src
mkdir -p build
cd build
cmake ..
make
```

All the output files will be stored in subdirectories of `build/` (out of source builds).

To suggest a specific compiler you can write:

```shell
CXX=clang++ cmake ..
```

[1]: https://github.com/morinim/vita
[2]: https://www.mozilla.org/MPL/2.0/
[3]: https://github.com/morinim/testudo/blob/master/LICENSE
[4]: https://www.gnu.org/software/xboard/engine-intf.html
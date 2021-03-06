<!--
SPDX-FileCopyrightText: 2020 callgraph-tool authors. All rights reserved

SPDX-License-Identifier: Apache-2.0
-->

# Callgraph

This repository is a collection of tools for constructing and analyzing callgraphs. The tools have been developed mainly to target Linux Kernel, but they should work with any target C/C++ programs that can be compiled with clang. Main highlights include:
- Constructs a precise global callgraph.
- Builds on top of LLVM.
- Input is LLVM assembly (IR): generated callgraph reflects the compilation configurations. For instance, build configuration and compiler optimization levels impact the output callgraph.
- Resolves indirect call targets based on type-analysis code from the [crix](https://github.com/umnsec/crix) program.
- Scales well to large programs, does not require excessive resources. Finds direct and indirect function calls from an example Linux x86_64_defconfig kernel build in less than two minutes on a laptop with 8GB RAM and a SSD drive.
- Allows effectively querying and viewing function call chains from large C/C++ programs.

See some example uses at: [How to use the callgraph database](./doc/query_examples.md#how-to-use-the-callgraph-database). To get started on using callgraph with your own target C programs, check this [example](./doc/simple_c_example.md). For examples of how to use crix-callgraph with C++ targets, see: [Using callgraph with C++ targets](./doc/using_with_cpp.md).

Table of Contents
=================

* [Getting started](#getting-started)
* [Generating callgraph database](#generating-callgraph-database)
    * [Build the crix-callgraph tool](#build-the-crix-callgraph-tool)
    * [Generate bitcode files from the target program](#generate-bitcode-files-from-the-target-program)
    * [Run the crix-callgraph tool](#run-the-crix-callgraph-tool)
* [Visualizing callgraphs](#visualizing-callgraphs)


## Getting started
These setup instructions have been tested on Ubuntu 18.04 and 20.04.

Checkout callgraph:
```
git clone https://github.com/elisa-tech/workgroups.git

# $CG_DIR contains the path to callgraph-tool
CG_DIR=$(pwd)/workgroups/safety-architecture/tools/callgraph-tool
```

Install the following requirements:
```
sudo apt install python3 python3-pip build-essential cmake \
clang-10 python3-clang-10 llvm-10 llvm-10-dev clang-format-10 graphviz
```

In addition, the scripts rely on python packages specified in requirements.txt. You can install the required packages with:
```
cd $CG_DIR
pip3 install -r requirements.txt
```

You also need the target kernel source tree. For the sake of example, we use the stable tree:
```
cd ~   # wherever you prefer to clone the kernel tree to
git clone https://git.kernel.org/pub/scm/linux/kernel/git/stable/linux-stable.git
cd linux-stable
git checkout v5.9  # we'll use v5.9 as an example

# We assume $KERNEL contains the path to the target kernel tree root
KERNEL=$(pwd)
```
__Notice__: Linux kernel releases after v5.9 no longer compile with clang 10.0.0. If you encounter this problem you may want to [use crix-callgraph with custom LLVM version](./doc/using_custom_llvm.md).

## Generating callgraph database
To make use of the tool, you first need to generate callgraph database relevant for your kernel source tree and build configuration.

#### Build the crix-callgraph tool
We are going to use crix-callgraph to build the callgraph database. Crix-callgraph is based on callgraph code from the [crix](https://github.com/umnsec/crix) program. The tool was build on top of crix version: https://github.com/umnsec/crix/commit/13d3e5574aaaae07d93ae5d1fc5f46c9487992ba.

To compile crix-callgraph, run:
```
cd $CG_DIR

# Make sure correct version of clang is in $PATH
source $CG_DIR/env.sh

# Build crix-callgraph
make

# Now, you can find the executable in `build/lib/crix-callgraph`
```

#### Generate bitcode files from the target program
The target program should be compiled with the same version of LLVM you built the crix-callgraph with. Also note that crix-callgraph requires the target code compiled with debug information included (-g). For the kernel build, you can enable debug information by setting CONFIG_DEBUG_INFO=y in the build configuration.

For convenience, we use the [wllvm](https://github.com/travitch/whole-program-llvm) to generate the LLVM bitcode files from the target kernel. [wllvm](https://github.com/travitch/whole-program-llvm) is available as a pip package: `pip3 install wllvm`.

```
cd $KERNEL

# Set the environment variables for wllvm
source $CG_DIR/env.sh

# Clean
make clean && make mrproper
# Clean any prior generated bitcode files manually
find . -type f -name ".*.bc" -and ! -name "timeconst.bc" -delete

# Generate defconfig
make defconfig CC=wllvm

# Manually enable CONFIG_DEBUG_INFO by editing the .config
vim .config

# Set any missing configuration options to default without prompting
make olddefconfig CC=wllvm

# Build the kernel generating bitcode files
make CC=wllvm HOSTCC=wllvm -j$(nproc)

# List all generated bitcode files
find ~+ -type f -name "*.bc" -and ! -name "timeconst.bc" > bitcodefiles.txt
```

#### Run the crix-callgraph tool
```
cd $CG_DIR

# To generate a callgraph database using the .bc files listed in
# $KERNEL/bitcodefiles.txt as input, run crix-callgraph as follows
# (the @-notation allows specifying a list of input .bc files)
./build/lib/crix-callgraph @$KERNEL/bitcodefiles.txt

# Now, you can find the callgraph.csv database in `callgraph.csv`
```

Notice that we intentionally don't use the whole-program LLVM bitcode generated by wllvm. Instead, we use the list of individual pre-linked bitcode files as an input to crix-callgraph. On linking, llvm applies optimizations such as merging structurally identical struct types to avoid duplication. Such struct type merging would make identifying indirect call targets less accurate, therefore, we use the pre-linked bitcode files as input to crix-callgraph.

## Visualizing callgraphs
Once the database is generated, it can be used to visualize function callgraphs.

As an example, to query callgraph starting from function syscall close (`__x64_sys_close`) with max depth of four function calls, you would run:
```
cd $CG_DIR
./scripts/query_callgraph.py --csv callgraph.csv --function '__x64_sys_close' --depth 3
```
Which outputs the callgraph as an image:
<img src=doc/sys_close.png width="900">

Notice the compiler optimizations and function inlining impact the output generated by crix-callgraph. Therefore, depending on the compiler options you used on building the target program bitcode files, the visualized callgraphs might not be an exact representation of what one might expect based on the C-source files. For instructions on how to disable compiler optimizations and function inlining while building the kernel bitcode files, see: [Building kernel bitcode files with compiler optimizations disabled](./doc/query_examples.md#building-kernel-bitcode-files-with-compiler-optimizations-disabled). 

For more examples on querying and visualizing the generated callgraph data, as well as explanation of the notation used in the graphs, see: [How to visualize and query the callgraph data](./doc/query_examples.md#how-to-visualize-and-query-the-callgraph-data).

// SPDX-FileCopyrightText: 2020 callgraph-tool authors. All rights reserved
//
// SPDX-License-Identifier: LicenseRef-LLVM

#ifndef COMMON_H
#define COMMON_H

#include <llvm/Analysis/TargetLibraryInfo.h>
#include <regex>

using namespace llvm;
using namespace std;

#define OP llvm::errs()

#define DEBUG 0

#define DSTREAM stderr
#define LOG(str) LOG_FMT("%s\n", str)
#define LOG_OBJ(str, obj)                                                      \
  do {                                                                         \
    if (DEBUG) {                                                               \
      LOG_FMT("%s ", str);                                                     \
      obj->print(OP);                                                          \
      OP << "\n";                                                              \
    }                                                                          \
  } while (0)
#define LOG_FMT(fmt, ...)                                                      \
  do {                                                                         \
    if (DEBUG)                                                                 \
      fprintf(DSTREAM, "[%s:%d]: " fmt, __func__, __LINE__, __VA_ARGS__);      \
  } while (0)

#define CLR_NRM "\x1B[0m"
#define CLR_YEL "\x1B[1;33m"
#define WARN_FMT(fmt, ...)                                                     \
  do {                                                                         \
    fprintf(DSTREAM, "%s[Warning]%s " fmt, CLR_YEL, CLR_NRM, __VA_ARGS__);     \
  } while (0)

//
// Common functions
//

size_t funcHash(Function *F, bool withName = true);
size_t callHash(CallBase *CI);
size_t typeHash(Type *Ty);
size_t typeIdxHash(Type *Ty, int Idx = -1);
size_t hashIdxHash(size_t Hs, int Idx = -1);

#endif

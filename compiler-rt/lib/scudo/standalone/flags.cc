//===-- flags.cc ------------------------------------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "flags.h"
#include "common.h"
#include "flags_parser.h"
#include "interface.h"

namespace scudo {

Flags *getFlags() {
  static Flags F;
  return &F;
}

void Flags::setDefaults() {
#define SCUDO_FLAG(Type, Name, DefaultValue, Description) Name = DefaultValue;
#include "flags.inc"
#undef SCUDO_FLAG
}

void registerFlags(FlagParser *Parser, Flags *F) {
#define SCUDO_FLAG(Type, Name, DefaultValue, Description)                      \
  Parser->registerFlag(#Name, Description, FlagType::FT_##Type,                \
                       reinterpret_cast<void *>(&F->Name));
#include "flags.inc"
#undef SCUDO_FLAG
}

static const char *getCompileDefinitionScudoDefaultOptions() {
#ifdef SCUDO_DEFAULT_OPTIONS
  return STRINGIFY(SCUDO_DEFAULT_OPTIONS);
#else
  return "";
#endif
}

static const char *getScudoDefaultOptions() {
  return (&__scudo_default_options) ? __scudo_default_options() : "";
}

void initFlags() {
  Flags *F = getFlags();
  F->setDefaults();
  FlagParser Parser;
  registerFlags(&Parser, F);
  Parser.parseString(getCompileDefinitionScudoDefaultOptions());
  Parser.parseString(getScudoDefaultOptions());
  Parser.parseString(getEnv("SCUDO_OPTIONS"));
}

} // namespace scudo

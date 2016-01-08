//===--- IntersecTidyModule.cpp - clang-tidy --------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "../ClangTidy.h"
#include "../ClangTidyModule.h"
#include "../ClangTidyModuleRegistry.h"
#include "../readability/BracesAroundStatementsCheck.h"
#include "../readability/FunctionSizeCheck.h"
#include "../readability/NamespaceCommentCheck.h"
#include "../readability/RedundantSmartptrGetCheck.h"
#include "TscopeCheck.cpp"
#include "UnusedTscopesCheck.h"
#include "LoopAlloca.h"

using namespace clang::ast_matchers;

namespace clang {
namespace tidy {
namespace intersec {

class IntersecModule : public ClangTidyModule {
public:
  void addCheckFactories(ClangTidyCheckFactories &CheckFactories) override {
    CheckFactories.registerCheck<TscopeCheck>(
        "intersec-tscope");
    CheckFactories.registerCheck<UnusedTscopesCheck>(
        "intersec-unused-tscopes");
    CheckFactories.registerCheck<LoopAlloca>(
        "intersec-loop-alloca");
  }

  ClangTidyOptions getModuleOptions() override {
    ClangTidyOptions Options;
    return Options;
  }
};

// Register the IntersecTidyModule using this statically initialized variable.
static ClangTidyModuleRegistry::Add<IntersecModule> X("intersec-module",
                                                    "Adds Intersec lint checks.");

} // namespace intersec

// This anchor is used to force the linker to link in the generated object file
// and thus register the IntersecModule.
volatile int IntersecModuleAnchorSource = 0;

} // namespace tidy
} // namespace clang

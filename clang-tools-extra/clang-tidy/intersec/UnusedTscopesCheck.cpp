//===--- UnusedTscopesCheck.cpp - clang-tidy-------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "UnusedTscopesCheck.h"
#include "clang/AST/ASTContext.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"

using namespace clang::ast_matchers;

namespace clang {
namespace tidy {

void UnusedTscopesCheck::registerMatchers(MatchFinder *Finder) {
  Finder->addMatcher(compoundStmt(has(declStmt(has(varDecl(has(callExpr(
    callee(namedDecl(matchesName("mem_stack_push")))).bind("tscope")))))),
          unless(hasDescendant(callExpr(callee(namedDecl(
              matchesName("::z?_*t_"))))))), this);
}

void UnusedTscopesCheck::check(const MatchFinder::MatchResult &Result) {
  auto Tscope = Result.Nodes.getNodeAs<CallExpr>("tscope");

  diag(Tscope->getBeginLoc(), "t_scope is not used");
}

} // namespace tidy
} // namespace clang

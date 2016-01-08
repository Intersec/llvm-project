//===--- WrongTfunctionCheck.cpp - clang-tidy------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "WrongTfunctionCheck.h"
#include "clang/AST/ASTContext.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"

using namespace clang::ast_matchers;

namespace clang {
namespace tidy {

void WrongTfunctionCheck::registerMatchers(MatchFinder *Finder) {
    auto tfunctions = namedDecl(matchesName("::_*(z_)?t_"));
    auto excludes = anyOf(namedDecl(matchesName("::t_pool")), namedDecl(hasName("t_scope_cleanup")));
    auto matcher =
        functionDecl(tfunctions.bind("tfunc"), isDefinition(), unless(excludes),
                     unless(hasDescendant(callExpr(
                         callee(tfunctions),
                         unless(hasAncestor(compoundStmt(has(
                             declStmt(has(varDecl(has(callExpr(callee(namedDecl(
                                 matchesName("mem_stack_push"))))))))))))))));
    Finder->addMatcher(matcher, this);
}

void WrongTfunctionCheck::check(const MatchFinder::MatchResult &Result) {
    auto Tfunc = Result.Nodes.getNodeAs<FunctionDecl>("tfunc");
    diag(Tfunc->getLocation(), "t_ function which does not use the t_stack");
}

} // namespace tidy
} // namespace clang

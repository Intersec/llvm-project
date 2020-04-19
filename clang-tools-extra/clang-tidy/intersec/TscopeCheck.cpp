//===--- TscopeCheck.cpp - clang-tidy--------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "TscopeCheck.h"
#include "clang/AST/ASTContext.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "llvm/Support/Regex.h"

using namespace clang::ast_matchers;

namespace clang {
namespace tidy {

void TscopeCheck::registerMatchers(MatchFinder *Finder) {
  // creation of a new t stack
  auto tpush = callExpr(callee(namedDecl(matchesName("mem_stack_push"))));

  Finder->addMatcher(
    callExpr(expr().bind("callee"),
             // find call to a function needing a t_scope
             callee(namedDecl(matchesName("::_*z?r?_?t_"))),
             // inside a function definition
             hasAncestor(functionDecl(decl().bind("caller"),
                                      isDefinition())),
             // but do not match if a new t_scope was created
             unless(hasAncestor(compoundStmt(
               anyOf(
                 // direct call to mem_stack_push
                 has(tpush),
                 // "t_scope;" instruction
                 has(declStmt(has(varDecl(has(tpush)))))
               )
             )))
    ), this);
}

void TscopeCheck::check(const MatchFinder::MatchResult &Result) {
  auto called = Result.Nodes.getNodeAs<CallExpr>("callee");
  auto caller = Result.Nodes.getNodeAs<FunctionDecl>("caller");
  auto caller_name = caller->getName();

  // if the function is not inside a t_scope, return an error
  // Matches all variants:
  //  - with leading underscores
  //  - with "z_" prefix
  //  - with "r" or "r_" prefix, if both a r_pool and t_scope are used.
  if (!llvm::Regex("^_*z?r?_?t_").match(caller_name)
  &&  !caller_name.equals("ipool"))
  {
    diag(called->getBeginLoc(), "function '%0' is called by function '%1' "
        "but is missing a t_scope.")
      << called->getDirectCallee()->getName() << caller->getName();
  }
}

} // namespace tidy
} // namespace clang

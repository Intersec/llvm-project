//===--- LoopAlloca.cpp - clang-tidy-------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "LoopAlloca.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/ParentMap.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"

#include <iostream>

using namespace clang::ast_matchers;

namespace clang {
namespace tidy {
namespace intersec {

void LoopAlloca::registerMatchers(MatchFinder *Finder) {
    Finder->addMatcher(
        callExpr(
            callee(namedDecl(hasName("__builtin_alloca"))),
            hasAncestor(forStmt().bind("for"))
        ).bind("alloca_loop"), this
    );
    Finder->addMatcher(
        callExpr(
            callee(namedDecl(hasName("__builtin_alloca"))),
            hasAncestor(whileStmt().bind("while"))
        ).bind("alloca_loop"), this
    );
}

static
bool checkForParentBlock(const ParentMap &map,
                         ASTContext *ctx,
                         Stmt *node, const Stmt *forStmt,
                         const Stmt *whileStmt)
{
    if (!node) {
        return false;
    }
    if (node->getStmtClass() == Stmt::BlockExprClass) {
        return true;
    }

    return checkForParentBlock(map,
                               ctx,
                               map.getParent(node), forStmt, whileStmt);
}

void LoopAlloca::check(const MatchFinder::MatchResult &Result) {
    const auto *MatchedDecl = Result.Nodes.getNodeAs<CallExpr>("alloca_loop");
    const auto *whileStmt = Result.Nodes.getNodeAs<WhileStmt>("while");
    const auto *forStmt = Result.Nodes.getNodeAs<ForStmt>("for");
    const Stmt *parent;
    SourceManager &srcMgr = Result.Context->getSourceManager();
    std::string fileName;

    if (whileStmt) {
        fileName = srcMgr.getFilename(srcMgr.getSpellingLoc(whileStmt->getWhileLoc())).str();
        if (fileName.find("lib-common/z.h") < fileName.size()) {
            return;
        }
        parent = whileStmt;
    }
    if (forStmt) {
        fileName = srcMgr.getFilename(srcMgr.getSpellingLoc(forStmt->getForLoc())).str();
        if (fileName.find("lib-common/iop-macros.h") < fileName.size()) {
            return;
        }
        parent = forStmt;
    }
    ParentMap parentMap(
        const_cast<Stmt *>(parent)
    );
    if (checkForParentBlock(parentMap,
                            Result.Context,
                            const_cast<CallExpr *>(MatchedDecl), forStmt, whileStmt)) {
        return;
    }

    diag(MatchedDecl->getLocStart(), "variable allocated with 'alloca' should not be declared in a loop scope");
}

} // namespace intersec
} // namespace tidy
} // namespace clang

//===------------------------------------------------------------*- C++ -*-===//
//
// This file is distributed under BSD License. See LICENSE.txt for details.
//
//===----------------------------------------------------------------------===//
//
// Copyright (c) 2017 University of Kaiserslautern.
//
//===----------------------------------------------------------------------===//
// \file
// DecisionTreeNode implementation
//===----------------------------------------------------------------------===//

#include "DecisionTreeNode.hpp"
#include "../visitors/DecisionTreeNodeLocator.hpp"

namespace ESolver {

    DecisionTreeNode::DecisionTreeNode()
            : m_Expr(Expression::NullObject), m_ThenEvalPtr(nullptr),
              m_ElseEvalPtr(nullptr), m_ThenNode(nullptr), m_ElseNode(nullptr)
    {}

    DecisionTreeNode::DecisionTreeNode(Expression Expr,
                                       ConcreteEvaluator* ThenEval,
                                       ConcreteEvaluator* ElseEval)
            : m_Expr(Expr), m_ThenEvalPtr(ThenEval), m_ElseEvalPtr(ElseEval),
              m_ThenNode(nullptr), m_ElseNode(nullptr)
    {}

    const UserExpressionBase* DecisionTreeNode::GetExpr() const
    {
        return m_Expr.GetPtr();
    }

    const UserExpressionBase* DecisionTreeNode::GetConditionExpr() const
    {
        return m_Expr->GetChildren()[0].GetPtr();
    }

    const UserExpressionBase* DecisionTreeNode::GetThenExpr() const
    {
        return m_Expr->GetChildren()[1].GetPtr();
    }

    const UserExpressionBase* DecisionTreeNode::GetElseExpr() const
    {
        return m_Expr->GetChildren()[2].GetPtr();
    }

    void DecisionTreeNode::SetThenBranch(DecisionTreeNode* Node, Expression Expr)
    {
        m_ThenNode = Node;
        auto ExpPtr =
                const_cast<UserInterpretedFuncExpression*>(
                        UserExpressionBase::As<UserInterpretedFuncExpression>(m_Expr));
        ExpPtr->SetChildAt(1, Expr);
    }

    void DecisionTreeNode::SetElseBranch(DecisionTreeNode* Node, Expression Expr)
    {
        m_ElseNode = Node;
        auto ExpPtr =
                const_cast<UserInterpretedFuncExpression*>(
                        UserExpressionBase::As<UserInterpretedFuncExpression>(m_Expr));
        ExpPtr->SetChildAt(2, Expr);
    }

    DecisionTreeNode* DecisionTreeNode::GetThenNode() const
    {
        return m_ThenNode;
    }

    DecisionTreeNode* DecisionTreeNode::GetElseNode() const
    {
        return m_ElseNode;
    }

    ConcreteEvaluator* DecisionTreeNode::GetThenEval() const
    {
        return m_ThenEvalPtr;
    }

    ConcreteEvaluator* DecisionTreeNode::GetElseEval() const
    {
        return m_ElseEvalPtr;
    }

    void DecisionTreeNode::Accept(DecisionTreeVisitorBase* Visitor)
    {
        Visitor->VisitDecisionTreeNode(this);
    }
}

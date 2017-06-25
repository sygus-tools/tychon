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
// DecisionTreeNode
//===----------------------------------------------------------------------===//

#pragma once

#include "../common/ESolverForwardDecls.hpp"
#include "../expressions/UserExpression.hpp"
#include "../containers/ESSmartPtr.hpp"
#include <boost/functional/hash.hpp>

namespace ESolver {

    class DecisionTreeNode
    {
    private:
        Expression m_Expr;
        ConcreteEvaluator* m_ThenEvalPtr;
        ConcreteEvaluator* m_ElseEvalPtr;
        DecisionTreeNode* m_ThenNode;
        DecisionTreeNode* m_ElseNode;

    public:
        DecisionTreeNode();
        DecisionTreeNode(Expression Expr,
                         ConcreteEvaluator* ThenEval,
                         ConcreteEvaluator* ElseEval);
        virtual ~DecisionTreeNode() = default;
        DecisionTreeNode(const DecisionTreeNode& Other) = default;
        inline bool operator==(const DecisionTreeNode& Other) const
        {
            return (m_ThenEvalPtr == Other.GetThenEval())
                    && (m_ElseEvalPtr == Other.GetElseEval());
        }

        const UserExpressionBase* GetExpr() const;
        const UserExpressionBase* GetConditionExpr() const;
        void SetThenBranch(DecisionTreeNode* Node, Expression Expr);
        void SetElseBranch(DecisionTreeNode* Node, Expression Expr);
        const UserExpressionBase* GetThenExpr() const;
        const UserExpressionBase* GetElseExpr() const;
        DecisionTreeNode* GetThenNode() const;
        DecisionTreeNode* GetElseNode() const;
        ConcreteEvaluator* GetThenEval() const;
        ConcreteEvaluator* GetElseEval() const;
        void Accept(DecisionTreeVisitorBase* Visitor);
    };

    class DecisionTreeNodeLocation
    {
    public:
        DecisionTreeNode* Node;
        bool ValidCondition;

        inline ConcreteEvaluator* GetActiveEval() const
        {
            return (ValidCondition) ? Node->GetThenEval() : Node->GetElseEval();
        }

        inline uint64 Hash() const
        {
            uint64 Seed = 0;
            boost::hash_combine(Seed, ValidCondition);
            boost::hash_combine(Seed, Node->GetThenEval());
            boost::hash_combine(Seed, Node->GetElseEval());
            return Seed;
        }

        inline bool operator==(const DecisionTreeNodeLocation& Other) const
        {
            return (ValidCondition == Other.ValidCondition)
                    && (Node == Other.Node);
        }
    };

    class DecisionTreeNodeLocationHasher
    {
    public:
        inline uint64 operator()(const DecisionTreeNodeLocation& Location) const
        {
            return Location.Hash();
        }
    };

    class DecisionTreeNodeLocationEquals
    {
    public:
        inline bool operator()(const DecisionTreeNodeLocation& Location1,
                               const DecisionTreeNodeLocation& Location2) const
        {
            return (Location1 == Location2);
        }
    };
}

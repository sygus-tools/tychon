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
// Finds the suitable location node for inserting a new example
//===----------------------------------------------------------------------===//

#pragma once

#include "../common/ESolverForwardDecls.hpp"
#include "../visitors/ExpressionVisitorBase.hpp"
#include "../utils/UIDGenerator.hpp"
#include "../expressions/UserExpression.hpp"
#include "../containers/ESSmartPtr.hpp"
#include "../solverutils/EvalRule.hpp"
#include "../values/ConcreteValueBase.hpp"
#include "../solverutils/DecisionTreeNode.hpp"

namespace ESolver {

    class DecisionTreeVisitorBase
    {
    protected:
        const std::string m_Name;

    public:
        explicit DecisionTreeVisitorBase(std::string Name);
        virtual ~DecisionTreeVisitorBase() = default;
        virtual void VisitDecisionTreeNode(DecisionTreeNode* DTNode);
    };

    class DecisionTreeNodeLocator: public DecisionTreeVisitorBase
    {
    private:
        const ConcreteEvaluator* m_ConcEval;
        DecisionTreeNodeLocation* m_ResultLocation;
        ConcreteValueBase m_ConcValue;

    public:
        DecisionTreeNodeLocator();
        DecisionTreeNodeLocator(const ConcreteEvaluator* Eval,
                                DecisionTreeNodeLocation* ResultLocation);
        virtual ~DecisionTreeNodeLocator() = default;
        virtual void VisitDecisionTreeNode(DecisionTreeNode* DTNode) override;
        static void Do(DecisionTreeNode* TreeRoot,
                       const ConcreteEvaluator* Eval,
                       DecisionTreeNodeLocation& ResultLocation);
    };
}

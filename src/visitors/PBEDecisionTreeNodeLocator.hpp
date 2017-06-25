//===------------------------------------------------------------*- C++ -*-===//
//
// This file is distributed under MIT License. See LICENSE.txt for details.
//
//===----------------------------------------------------------------------===//
//
// Copyright (c) 2017 University of Kaiserslautern.
//
//===----------------------------------------------------------------------===//
// \file
// Finds the suitable node for inserting a new example
//===----------------------------------------------------------------------===//

#pragma once

#include "../common/ESolverForwardDecls.hpp"
#include "../visitors/ExpressionVisitorBase.hpp"
#include "../utils/UIDGenerator.hpp"
#include "../expressions/UserExpression.hpp"
#include "../containers/ESSmartPtr.hpp"
#include "../solverutils/EvalRule.hpp"
#include "../values/ConcreteValueBase.hpp"

namespace ESolver {

    class PBEDecisionTreeNodeLocator: public ExpressionVisitorBase
    {
    private:
        const OperatorBase* TreeNodeOp;
        const ConcreteEvaluator* ConcEvaluator;
        std::pair<Expression, uint8>& TheResultNode;
        ConcreteValueBase ConcreteValue;
    public:
        PBEDecisionTreeNodeLocator() = delete;
        PBEDecisionTreeNodeLocator(const OperatorBase* NodeOp,
                                  const ConcreteEvaluator* Eval,
                                  std::pair<Expression, uint8>& ResultNode);
        virtual ~PBEDecisionTreeNodeLocator() = default;
        virtual void VisitUserInterpretedFuncExpression(const UserInterpretedFuncExpression* Exp) override;
        static void Do(const Expression& TreeRoot,
                       const ConcreteEvaluator* Eval,
                       std::pair<Expression, uint8>& ResultNode);
    };
}



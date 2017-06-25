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
// Implementation of PBEDecisionTreeNodeLocator
//===----------------------------------------------------------------------===//

#include "PBEDecisionTreeNodeLocator.hpp"
#include "../solverutils/ConcreteEvaluator.hpp"

namespace ESolver {

    PBEDecisionTreeNodeLocator::PBEDecisionTreeNodeLocator
            (const OperatorBase* NodeOp,
             const ConcreteEvaluator* Eval,
             std::pair<Expression, uint8>& ResultNode)
            : ExpressionVisitorBase("PBEDecisionTreeNodeLocator"),
              TreeNodeOp(NodeOp),
              ConcEvaluator(Eval),
              TheResultNode(ResultNode)
    {}

    void PBEDecisionTreeNodeLocator::VisitUserInterpretedFuncExpression
            (const UserInterpretedFuncExpression* Exp)
    {
        ConcEvaluator->ConretelyEvaluate(Exp->GetChildren()[0], &ConcreteValue);
        if (ConcreteValue.GetValue() == 1) {
            auto& NextNode = Exp->GetChildren()[1];
            if (NextNode->GetOp()->GetID() == TreeNodeOp->GetID()) {
                NextNode->Accept(this);
            } else {
                // reached a terminal expression
                TheResultNode.first = Exp;
                TheResultNode.second = 1;
            }
        } else {
            auto& NextNode = Exp->GetChildren()[2];
            if (NextNode->GetOp()->GetID() == TreeNodeOp->GetID()) {
                NextNode->Accept(this);
            } else {
                // reached a terminal expression
                TheResultNode.first = Exp;
                TheResultNode.second = 2;
            }
        }
    }

    void PBEDecisionTreeNodeLocator::Do(const Expression& TreeRoot,
                                       const ConcreteEvaluator* Eval,
                                       std::pair<Expression, uint8>& ResultNode)
    {
        PBEDecisionTreeNodeLocator Positioner(TreeRoot->GetOp(), Eval, ResultNode);
        TreeRoot->Accept(&Positioner);
    }
}

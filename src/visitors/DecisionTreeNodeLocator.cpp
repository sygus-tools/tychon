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
// DecisionTreeNodeLocator implementation
//===----------------------------------------------------------------------===//

#include "DecisionTreeNodeLocator.hpp"
#include "../solverutils/ConcreteEvaluator.hpp"

namespace ESolver {

    DecisionTreeVisitorBase::DecisionTreeVisitorBase(std::string Name)
            : m_Name(Name)
    {}

    void DecisionTreeVisitorBase::VisitDecisionTreeNode(DecisionTreeNode* DTNode)
    {
        DTNode->Accept(this);
    }

    DecisionTreeNodeLocator::DecisionTreeNodeLocator()
            : DecisionTreeVisitorBase("InsertionLocator")
    {}

    DecisionTreeNodeLocator::DecisionTreeNodeLocator
            (const ConcreteEvaluator* Eval,
             DecisionTreeNodeLocation* ResultLocation)
            : DecisionTreeVisitorBase("InsertionLocator"),
              m_ConcEval(Eval),
              m_ResultLocation(ResultLocation)
    {}

    void DecisionTreeNodeLocator::VisitDecisionTreeNode(DecisionTreeNode* DTNode)
    {
        m_ConcEval->ConcretelyEvaluate(DTNode->GetConditionExpr(),
                                       &m_ConcValue);
        if (m_ConcValue.GetValue() == 1) {
            auto NextNode = DTNode->GetThenNode();
            if (NextNode != nullptr) {
                NextNode->Accept(this);
            } else {
                // reached a terminal expression
                m_ResultLocation->Node = DTNode;
                m_ResultLocation->ValidCondition = true;
            }
        } else {
            auto NextNode = DTNode->GetElseNode();
            if (NextNode != nullptr) {
                NextNode->Accept(this);
            } else {
                // reached a terminal expression
                m_ResultLocation->Node = DTNode;
                m_ResultLocation->ValidCondition = false;
            }
        }
    }

    void DecisionTreeNodeLocator::Do(DecisionTreeNode* TreeRoot,
                                     const ConcreteEvaluator* Eval,
                                     DecisionTreeNodeLocation& ResultLocation)
    {
        DecisionTreeNodeLocator Locator(Eval, &ResultLocation);
        TreeRoot->Accept(&Locator);
    }
}

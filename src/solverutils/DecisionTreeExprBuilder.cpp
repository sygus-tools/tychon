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
// DecisionTreeExprBuilder implementation
//===----------------------------------------------------------------------===//

#include "DecisionTreeExprBuilder.hpp"
#include "../solvers/ESolver.hpp"
#include "../visitors/DecisionTreeNodeLocator.hpp"
#include "../descriptions/Operators.hpp"
#include "../solverutils/ConcreteEvaluator.hpp"

namespace ESolver {

    DecisionTreeExprBuilder::DecisionTreeExprBuilder(ESolver* Solver)
            : m_QueueIdx(0), m_Solver(Solver)
    { }

    void DecisionTreeExprBuilder::Reset(uint32 ExampleNum)
    {
        m_QueueIdx = 0;
        m_TreeNodes.clear();
        m_EvalsQueue.clear();
        m_EvalsQueue.reserve(ExampleNum);
        m_EvalsQueueTemp.clear();
        m_SharedDecisionNodes.clear();
    }

    void DecisionTreeExprBuilder::AddUniqTermExprExample(ConcreteEvaluator* Eval)
    {
        m_EvalsQueue.push_back(Eval);
    }

    void DecisionTreeExprBuilder::AddDupTermExprExample(ConcreteEvaluator* Eval)
    {
        m_EvalsQueueTemp.push_back(Eval);
    }

    ConcreteEvaluator* DecisionTreeExprBuilder::GetQueueBack()
    {
        return m_EvalsQueue.back();
    }

    ConcreteEvaluator* DecisionTreeExprBuilder::GetQueueFront()
    {
        return m_EvalsQueue[m_QueueIdx];
    }

    const DecisionTreeNode& DecisionTreeExprBuilder::GetRootNode() const
    {
        return m_TreeNodes.front();
    }

    bool DecisionTreeExprBuilder::LocateNextEvalNode(DecisionTreeNodeLocation& NodeLocation,
                                                     DTBuilderCurEvals& Evals)
    {
        if (m_TreeNodes.empty()) {
            m_QueueIdx = 2;
            Evals.first = m_EvalsQueue[0];
            Evals.second = m_EvalsQueue[1];
            return true;
        }
        if (m_QueueIdx < m_EvalsQueue.size()) {
            Evals.first = m_EvalsQueue[m_QueueIdx];
            DecisionTreeNodeLocator::Do(&(*m_TreeNodes.begin()),
                                        Evals.first,
                                        NodeLocation);
            Evals.second = NodeLocation.GetActiveEval();
            m_QueueIdx++;
            return true;
        }
        Evals.first = nullptr;
        Evals.second = nullptr;
        return false;
    }

    void DecisionTreeExprBuilder::Initialize(const ESFixedTypeBase* Type)
    {
        m_QueueIdx = 0;
        vector<const ESFixedTypeBase*> FuncDomain(3, Type);
        m_CondExprOp = OperatorBase::As<InterpretedFuncOperator>
                (m_Solver->LookupOperator(m_ConditionOpName, FuncDomain));
        if (m_CondExprOp == nullptr) {
            throw InternalError(
                    (string) "Could not locate <if0> conditional grammar!" +
                            "\nAt: " + __FILE__ + ":" + to_string(__LINE__));
        }
        std::copy(m_EvalsQueueTemp.cbegin(),
                  m_EvalsQueueTemp.cend(),
                  std::back_inserter(m_EvalsQueue));
        m_EvalsQueueTemp.clear();
    }

    void DecisionTreeExprBuilder::Do()
    {
        // Nothing serious to do now
        Reset();
    }

    Expression DecisionTreeExprBuilder::GetTreeExpr() const
    {
        return m_RootExpr;
    }

    void DecisionTreeExprBuilder::InsertDecisionNode(DecisionTreeNodeLocation& NodeLocation,
                                                     DTBuilderCurEvals& Evals,
                                                     vector<Expression>& NodeExprs)
    {
        if (m_TreeNodes.empty()) {
            m_RootExpr = m_Solver->CreateNonmanagedExpression(m_CondExprOp, NodeExprs);
            m_TreeNodes.emplace_back(DecisionTreeNode(m_RootExpr, Evals.first, Evals.second));
            NodeLocation.Node = &(m_TreeNodes.front());
            NodeLocation.ValidCondition = true;
        } else {
            auto SharedNodes = m_SharedDecisionNodes.equal_range(NodeLocation);
            for (auto It = SharedNodes.first; It != SharedNodes.second; ++It) {
                m_EvalsQueue.push_back(It->second);
            }
            if (SharedNodes.first != SharedNodes.second) {
                m_SharedDecisionNodes.erase(NodeLocation);
            }

            // Place node in tree
            auto ChildExpr = m_Solver->CreateNonmanagedExpression(m_CondExprOp, NodeExprs);
            m_TreeNodes.emplace_back(DecisionTreeNode(ChildExpr, Evals.first, Evals.second));
            if (NodeLocation.ValidCondition) {
                NodeLocation.Node->SetThenBranch(&(m_TreeNodes.back()), ChildExpr);
            } else {
                NodeLocation.Node->SetElseBranch(&(m_TreeNodes.back()), ChildExpr);
            }
        }
    }

    void DecisionTreeExprBuilder::InsertSharedDecisionNode(
            DecisionTreeNodeLocation& NodeLocation,
            DTBuilderCurEvals& Evals)
    {
        ConcreteEvaluator* QueuedEval;
        if (Evals.first == NodeLocation.Node->GetThenEval()
                || Evals.first == NodeLocation.Node->GetThenEval() ) {
            QueuedEval = Evals.second;
        } else {
            QueuedEval = Evals.first;
        }
        m_SharedDecisionNodes.insert({NodeLocation, QueuedEval});
    }
}

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
// DecisionTreeExprBuilder
//===----------------------------------------------------------------------===//

#pragma once

#include "../solverutils/DecisionTreeNode.hpp"

namespace ESolver {

    using DTBuilderCurEvals = std::pair<ConcreteEvaluator*, ConcreteEvaluator*>;
    class DecisionTreeExprBuilder
    {
    private:
        uint32 m_QueueIdx;
        ESolver* m_Solver;
        Expression m_RootExpr;
        const InterpretedFuncOperator* m_ConditionExprOp;
        // XXX: WARNING competition hack here!
        const std::string m_ConditionOpName = "if0";
        std::list<DecisionTreeNode> m_TreeNodes;
        std::vector<ConcreteEvaluator*> m_EvalsQueue;
        std::vector<ConcreteEvaluator*> m_EvalsQueueTemp;
        std::unordered_multimap<DecisionTreeNodeLocation,
                                ConcreteEvaluator*,
                                DecisionTreeNodeLocationHasher,
                                DecisionTreeNodeLocationEquals>
                m_SharedDecisionNodes;

    public:
        DecisionTreeExprBuilder() = delete;
        explicit DecisionTreeExprBuilder(ESolver* Solver);
        virtual ~DecisionTreeExprBuilder() = default;

        ConcreteEvaluator* GetQueueBack();
        ConcreteEvaluator* GetQueueFront();
        const DecisionTreeNode& GetRootNode() const;
        Expression GetTreeExpr() const;
        void Reset(uint32 ExampleNum = 0);
        void AddUniqTermExprExample(ConcreteEvaluator* Eval);
        void AddDupTermExprExample(ConcreteEvaluator* Eval);

        void Initialize(const ESFixedTypeBase* Type);
        bool LocateNextEvalNode(DecisionTreeNodeLocation& NodeLocation,
                                DTBuilderCurEvals& CurEvalPtrs);
        void InsertDecisionNode(DecisionTreeNodeLocation& NodeLocation,
                                DTBuilderCurEvals& Evals,
                                vector<Expression>& NodeExprs);
        void InsertSharedDecisionNode(DecisionTreeNodeLocation& NodeLocation,
                                      DTBuilderCurEvals& Evals);

        void Do();
    };
}


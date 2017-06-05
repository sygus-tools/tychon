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
// Initializer for consequent expressions in PBE mode
//===----------------------------------------------------------------------===//

#pragma once

#include "../common/ESolverForwardDecls.hpp"
#include "../visitors/ExpressionVisitorBase.hpp"
#include "../utils/UIDGenerator.hpp"
#include "../expressions/UserExpression.hpp"
#include "../containers/ESSmartPtr.hpp"
#include "../solverutils/EvalRule.hpp"

namespace ESolver {

    class PBEConsequentsInitializer: public ExpressionVisitorBase
    {
    private:
        vector<Expression>& PBEConsequents;
        bool ReachedLeafNode;

    public:
        PBEConsequentsInitializer(vector<Expression>& PBEConsequentExprs);
        virtual ~PBEConsequentsInitializer() = default;

        virtual void VisitUserSynthFuncExpression(const UserSynthFuncExpression* Exp) override;
        virtual void VisitUserUQVarExpression(const UserUQVarExpression* Exp) override;
        virtual void VisitUserInterpretedFuncExpression(const UserInterpretedFuncExpression* Exp) override;
        virtual void VisitUserLetExpression(const UserLetExpression* Exp) override;

        virtual void VisitUserLetBoundVarExpression(const UserLetBoundVarExpression* Exp) override;
        virtual void VisitUserConstExpression(const UserConstExpression* Exp) override;
        virtual void VisitUserFormalParamExpression(const UserFormalParamExpression* Exp) override;
        virtual void VisitUserAuxVarExpression(const UserAuxVarExpression* Exp) override;
        static void Do(const Expression& RewrittenConstraints,
                       vector<Expression>& PBEConsequentExprs);
};
} /* End namespace */

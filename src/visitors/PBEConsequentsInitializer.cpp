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
// Implementation of initializer for consequent expressions in PBE mode
//===----------------------------------------------------------------------===//

#include "PBEConsequentsInitializer.hpp"
#include "../exceptions/ESException.hpp"

namespace ESolver {

    PBEConsequentsInitializer::PBEConsequentsInitializer(vector<Expression>& PBEConsequentExprs)
        : ExpressionVisitorBase("PBEConsequentsInit"),
          PBEConsequents(PBEConsequentExprs)
    {
    }

    void PBEConsequentsInitializer::Do(const Expression& RewrittenConstraints,
                                       vector<Expression>& PBEConsequentExprs)
    {
        PBEConsequentsInitializer Initializer(PBEConsequentExprs);
        RewrittenConstraints->GetChildren()[1]->Accept(&Initializer);
    }

    void
    PBEConsequentsInitializer::VisitUserInterpretedFuncExpression(const UserInterpretedFuncExpression* Exp)
    {
        const auto& Children = Exp->GetChildren();
        auto SubExprType1 = Exp->As<UserAuxVarExpression>(Children[0]);
        auto SubExprType2 = Exp->As<UserConstExpression>(Children[1]);
        if (SubExprType1 != nullptr && SubExprType2 != nullptr) {
            auto result = UniqueAuxVars.insert({Exp->ToString(), true});
            // keep only unique constraints
            if (result.second) {
                PBEConsequents.push_back(Exp);
            }
        } else {
            Children[0]->Accept(this);
            Children[1]->Accept(this);
        }
    }

    void
    PBEConsequentsInitializer::VisitUserSynthFuncExpression(const UserSynthFuncExpression* Exp)
    {
        throw InternalError((string)"Invalid constraint consequent in PBE mode detected" +
            "\nAt: " + __FILE__ + ":" + to_string(__LINE__));
    }

    void
    PBEConsequentsInitializer::VisitUserUQVarExpression(const UserUQVarExpression* Exp)
    {
        throw InternalError((string)"Invalid constraint consequent in PBE mode detected" +
            "\nAt: " + __FILE__ + ":" + to_string(__LINE__));
    }

    void
    PBEConsequentsInitializer::VisitUserLetExpression(const UserLetExpression* Exp)
    {
        throw InternalError((string)"Invalid constraint consequent in PBE mode detected" +
            "\nAt: " + __FILE__ + ":" + to_string(__LINE__));
    }

    void
    PBEConsequentsInitializer::VisitUserLetBoundVarExpression(const UserLetBoundVarExpression* Exp)
    {
        throw InternalError((string)"Invalid constraint consequent in PBE mode detected" +
            "\nAt: " + __FILE__ + ":" + to_string(__LINE__));
    }

    void
    PBEConsequentsInitializer::VisitUserConstExpression(const UserConstExpression* Exp)
    {
        throw InternalError((string)"Invalid constraint consequent in PBE mode detected" +
            "\nAt: " + __FILE__ + ":" + to_string(__LINE__));
    }


    void
    PBEConsequentsInitializer::VisitUserFormalParamExpression(const UserFormalParamExpression* Exp)
    {
        throw InternalError((string)"Invalid constraint consequent in PBE mode detected" +
            "\nAt: " + __FILE__ + ":" + to_string(__LINE__));
    }

    void
    PBEConsequentsInitializer::VisitUserAuxVarExpression(const UserAuxVarExpression* Exp)
    {
        throw InternalError((string)"Invalid constraint consequent in PBE mode detected" +
            "\nAt: " + __FILE__ + ":" + to_string(__LINE__));
    }
}

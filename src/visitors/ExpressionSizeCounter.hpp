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
// Calculates the size of an input expression
//===----------------------------------------------------------------------===//

#pragma once

#include "../visitors/ExpressionVisitorBase.hpp"
#include "../expressions/UserExpression.hpp"
#include "../containers/ESSmartPtr.hpp"

namespace ESolver {
    class ExpressionSizeCounter: public ExpressionVisitorBase
    {
    private:
        uint64 m_Size;
    public:
        ExpressionSizeCounter();
        virtual ~ExpressionSizeCounter() = default;
        static uint64 Do(const Expression& Expr);
        uint64 GetExprSize() const;
        virtual void VisitUserUQVarExpression(const UserUQVarExpression* Exp) override;
        virtual void VisitUserLetBoundVarExpression(const UserLetBoundVarExpression* Exp) override;
        virtual void VisitUserFormalParamExpression(const UserFormalParamExpression* Exp) override;
        virtual void VisitUserAuxVarExpression(const UserAuxVarExpression* Exp) override;
        virtual void VisitUserConstExpression(const UserConstExpression* Exp) override;
        virtual void VisitUserInterpretedFuncExpression(const UserInterpretedFuncExpression* Exp) override;
        virtual void VisitUserSynthFuncExpression(const UserSynthFuncExpression* Exp) override;
        virtual void VisitUserLetExpression(const UserLetExpression* Exp) override;
    };
}



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
// Implementation of ExpressionSizeCounter
//===----------------------------------------------------------------------===//

#include "ExpressionSizeCounter.hpp"

namespace ESolver {

    ExpressionSizeCounter::ExpressionSizeCounter()
            : ExpressionVisitorBase("ExpressionSizeCounter"), m_Size(0)
    { }

    uint64 ExpressionSizeCounter::GetExprSize() const
    {
        return m_Size;
    }

    uint64 ExpressionSizeCounter::Do(const Expression& Expr)
    {
        ExpressionSizeCounter Counter;
        Expr->Accept(&Counter);
        return Counter.GetExprSize();
    }

    void ExpressionSizeCounter::VisitUserUQVarExpression(const UserUQVarExpression* Exp)
    {
        m_Size++;
    }

    void ExpressionSizeCounter::VisitUserLetBoundVarExpression(const UserLetBoundVarExpression* Exp)
    {
        m_Size++;
    }

    void ExpressionSizeCounter::VisitUserFormalParamExpression(const UserFormalParamExpression* Exp)
    {
        m_Size++;
    }

    void ExpressionSizeCounter::VisitUserAuxVarExpression(const UserAuxVarExpression* Exp)
    {
        m_Size++;
    }

    void ExpressionSizeCounter::VisitUserConstExpression(const UserConstExpression* Exp)
    {
        m_Size++;
    }

    void ExpressionSizeCounter::VisitUserInterpretedFuncExpression(const UserInterpretedFuncExpression* Exp)
    {
        ExpressionVisitorBase::VisitUserInterpretedFuncExpression(Exp);
        m_Size++;
    }

    void ExpressionSizeCounter::VisitUserSynthFuncExpression(const UserSynthFuncExpression* Exp)
    {
        ExpressionVisitorBase::VisitUserSynthFuncExpression(Exp);
        m_Size++;
    }

    void ExpressionSizeCounter::VisitUserLetExpression(const UserLetExpression* Exp)
    {
        ExpressionVisitorBase::VisitUserLetExpression(Exp);
        m_Size++;
    }
}

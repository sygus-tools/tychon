// CEGSolver.cpp ---
//
// Filename: CEGSolver.cpp
// Author: Abhishek Udupa
// Created: Wed Jan 15 14:54:32 2014 (-0500)
//
//
// Copyright (c) 2013, Abhishek Udupa, University of Pennsylvania
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
// 1. Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
// 2. Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.
// 3. All advertising materials mentioning features or use of this software
//    must display the following acknowledgement:
//    This product includes software developed by The University of Pennsylvania
// 4. Neither the name of the University of Pennsylvania nor the
//    names of its contributors may be used to endorse or promote products
//    derived from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER ''AS IS'' AND ANY
// EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER BE LIABLE FOR ANY
// DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
// ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
//

// Code:


#include "CEGSolver.hpp"
#include "../solverutils/ConcreteEvaluator.hpp"
#include "../exceptions/ESException.hpp"
#include "../enumerators/CFGEnumerator.hpp"
#include "../descriptions/ESType.hpp"
#include "../descriptions/Operators.hpp"
#include "../z3interface/TheoremProver.hpp"
#include "../descriptions/Grammar.hpp"
#include "../descriptions/GrammarNodes.hpp"
#include "../utils/TimeValue.hpp"
#include "../solverutils/EvalRule.hpp"
#include "../visitors/ExpCheckers.hpp"
#include "../visitors/SpecRewriter.hpp"
#include "../visitors/Gatherers.hpp"
#include "../visitors/PBEConsequentsInitializer.hpp"


namespace ESolver {

    CEGSolver::CEGSolver(const ESolverOpts* Opts)
            : ESolver(Opts), ConcEval(nullptr), ExpEnumerator(nullptr),
              TheMode(CEGSolverMode::CEG),
              ThePhase(PBEPhase::EnumerateTermExprs)
    {
        // Nothing here
    }

    CEGSolver::~CEGSolver()
    {
        if (ConcEval != nullptr) {
            delete ConcEval;
        }
        if (ExpEnumerator != nullptr) {
            delete ExpEnumerator;
        }
    }

    inline bool CEGSolver::CheckSymbolicValidity(GenExpressionBase const* const* Exps)
    {
        vector<SMTExpr> Assumptions;
        auto FinConstraint =
                RewrittenConstraint->ToSMT(TP, Exps, BaseExprs, Assumptions);
        auto Antecedent = TP->CreateAndExpr(Assumptions);
        FinConstraint = TP->CreateImpliesExpr(Antecedent, FinConstraint);

        if (Opts.StatsLevel >= 3) {
            TheLogger.Log2("Validity Query:").Log2("\n");
            TheLogger.Log2(FinConstraint.ToString()).Log2("\n");
        }
        auto TPRes = TP->CheckValidity(FinConstraint);

        switch (TPRes) {
            case SOLVE_VALID:
                return true;
            case SOLVE_INVALID: {
                if (Opts.StatsLevel >= 4) {
                    TheLogger.Log4("Validity failed\nModel:\n");
                    SMTModel Model;
                    TP->GetConcreteModel(RelevantVars, Model, this);
                    for (auto ValuePair : Model) {
                        TheLogger.Log4(ValuePair.first).Log4(" : ").Log4(
                                ValuePair.second.ToString()).Log4(
                                "\n");
                    }
                }
                return false;
            }
            default:
                throw Z3Exception(
                        (string) "Error: Z3 returned an UNKNOWN result.\n" +
                                "Make sure all theories are decidable.");
        }
    }

    inline bool CEGSolver::CheckSymbolicValidity(const GenExpressionBase* Exp)
    {
        GenExpressionBase const* Arr[1];
        Arr[0] = Exp;
        return CheckSymbolicValidity(Arr);
    }

    CallbackStatus CEGSolver::SubExpressionCallBack(const GenExpressionBase* Exp,
                                                    const ESFixedTypeBase* Type,
                                                    uint32 ExpansionTypeID)
    {
        // Check if the subexpression is distinguishable
        uint32 StatusRet = 0;

        if (Opts.StatsLevel >= 4) {
            TheLogger.Log4("Checking Subexpression ").Log4(Exp->ToString()).Log4(
                    "... ");
        }

        CheckResourceLimits();

        auto Distinguishable =
                ConcEval->CheckSubExpression(const_cast<GenExpressionBase*>(Exp),
                                             Type, ExpansionTypeID, StatusRet);
        if (Distinguishable) {
            ++NumDistExpressions;
            if (Opts.StatsLevel >= 4) {
                if (StatusRet & CONCRETE_EVAL_PART) {
                    TheLogger.Log4("Dist (Partial).").Log4("\n");
                } else {
                    TheLogger.Log4("Dist.").Log4("\n");
                }
            }
            return NONE_STATUS;
        } else {
            if (Opts.StatsLevel >= 4) {
                TheLogger.Log4("Indist.").Log4("\n");
            }
            return DELETE_EXPRESSION;
        }
    }

    // Callbacks
    CallbackStatus CEGSolver::ExpressionCallBack(const GenExpressionBase* Exp,
                                                 const ESFixedTypeBase* Type,
                                                 uint32 ExpansionTypeID,
                                                 uint32 EnumeratorIndex)
    {

        if (TheMode == CEGSolverMode::PBE) {
            return ExpressionCallBackPBE(Exp,
                                         Type,
                                         ExpansionTypeID,
                                         EnumeratorIndex);
        }

        uint32 StatusRet = 0;

        NumExpressionsTried++;
        if (Opts.StatsLevel >= 4) {
            TheLogger.Log4(Exp->ToString()).Log4("... ");
        }

        CheckResourceLimits();

        auto ConcValid = ConcEval->CheckConcreteValidity(Exp,
                                                         Type,
                                                         ExpansionTypeID,
                                                         StatusRet);
        if (!ConcValid && (StatusRet & CONCRETE_EVAL_DIST) == 0) {
            if (Opts.StatsLevel >= 4) {
                TheLogger.Log4("Invalid, Indist.").Log4("\n");
            }
            return DELETE_EXPRESSION;
        } else if (!ConcValid) {
            NumDistExpressions++;
            if (Opts.StatsLevel >= 4) {
                if ((StatusRet & CONCRETE_EVAL_PART) != 0) {
                    TheLogger.Log4("Invalid, Dist (Partial).").Log4("\n");
                } else {
                    TheLogger.Log4("Invalid, Dist.").Log4("\n");
                }
            }
            return NONE_STATUS;
        }

        NumDistExpressions++;
        if (Opts.StatsLevel >= 4) {
            TheLogger.Log4("Valid.").Log4("\n");
        }

        // ConcValid, check for symbolic validity
        bool SymbValid = CheckSymbolicValidity(Exp);
        if (SymbValid) {
            // We're done
            this->Complete = true;
            Solutions.push_back(vector<pair<const SynthFuncOperator*,
                                            Expression>>());
            Solutions.back().push_back(pair<const SynthFuncOperator*,
                                            Expression>(SynthFuncs[0],
                                                        GenExpressionBase::ToUserExpression(
                                                                Exp,
                                                                this)));
            return STOP_ENUMERATION;
        } else {

            // Get the counter example and add it as a point
            SMTModel TheSMTModel;
            SMTConcreteValueModel ConcSMTModel;
            TP->GetConcreteModel(RelevantVars, TheSMTModel, ConcSMTModel, this);
            ConcEval->AddPoint(ConcSMTModel);
            ConcreteEvaluator::ResetSigStore(0);

            if (!Opts.NoDist) {
                Restart = true;
                return STOP_ENUMERATION;
            } else {
                return NONE_STATUS;
            }
        }
    }

    CallbackStatus CEGSolver::ExpressionCallBackPBE(const GenExpressionBase* Exp,
                                                    const ESFixedTypeBase* Type,
                                                    uint32 ExpansionTypeID,
                                                    uint32 EnumeratorIndex)
    {
        if (ThePhase == PBEPhase::EnumerateTermExprs) {
            return PBEEnumerateTermExprs(Exp,
                                         Type,
                                         ExpansionTypeID,
                                         EnumeratorIndex);
        }

        CheckResourceLimits();
        NumExpressionsTried++;
        if (Opts.StatsLevel >= 4) {
            TheLogger.Log4(Exp->ToString()).Log4("... ");
        }

        // if phase terminal expression synthesis
        // - check if we have a consistent expression
        // - if not continue until we find a non-consistent expr where we reorganize and restart
        uint32 StatusRet = 0;
        uint32 FirstDupValidEval = 0;
        bool IsSetFirstDupValidEval = false;
        bool ConcValid = PBEEvalPtrs[0]->CheckConcreteValidity(Exp,
                                                               Type,
                                                               ExpansionTypeID,
                                                               StatusRet);
        if (!ConcValid) {
            if ((StatusRet & CONCRETE_EVAL_DIST) == 0) {
                if (Opts.StatsLevel >= 4) {
                    TheLogger.Log4("Invalid, Indist.").Log4("\n");
                }
                return DELETE_EXPRESSION;
            }
            if (Opts.StatsLevel >= 4) {
                if ((StatusRet & CONCRETE_EVAL_PART) != 0) {
                    TheLogger.Log4("Invalid, Dist (Partial).").Log4("\n");
                } else {
                    TheLogger.Log4("Invalid, Dist.").Log4("\n");
                }
            }
            NumDistExpressions++;
            return NONE_STATUS;
        }

        NumDistExpressions++;
        for (uint32 i = 1; i < PBEEvalPtrs.size() && ConcValid; ++i) {
            ConcValid = PBEEvalPtrs[i]->CheckConcreteValidity(Exp,
                                                              Type,
                                                              ExpansionTypeID,
                                                              StatusRet);

            if (!ConcValid) {
                if (IsSetFirstDupValidEval) {
                    // bring interesting examples forward
                    iter_swap(PBEEvalPtrs.begin() + FirstDupValidEval,
                              PBEEvalPtrs.begin() + i);
                    if (Opts.StatsLevel >= 6) {
                        TheLogger.Log4("Swapping ConcEval(").Log4(i).Log4(") ");
                        TheLogger.Log4("and ConcEval(").Log4(FirstDupValidEval).Log4(
                                ") \n");
                    }
                }
                if (Opts.StatsLevel >= 6) {
                    TheLogger.Log4("ConcEval(").Log4(i).Log4("), ");
                }
                if (Opts.StatsLevel >= 4) {
                    TheLogger.Log4("Invalid, Dist.").Log4("\n");
                }
                return NONE_STATUS;
            }

            if (!IsSetFirstDupValidEval) {
                FirstDupValidEval = i;
                IsSetFirstDupValidEval = true;
            }

            if (Opts.StatsLevel >= 6) {
                TheLogger.Log4("ConcEval(").Log4(i).Log4("), ");
                TheLogger.Log4("Duplicate valid.").Log4("\n");
            }
        }

        if (Opts.StatsLevel >= 4) {
            TheLogger.Log4("Valid.").Log4("\n");
        }

        this->Complete = true;
        Solutions.push_back(vector<pair<const SynthFuncOperator*, Expression>>());
        Solutions.back().push_back({SynthFuncs[0],GenExpressionBase::ToUserExpression(Exp, this)});

        return STOP_ENUMERATION;
    }

    CallbackStatus CEGSolver::PBEEnumerateTermExprs(const GenExpressionBase* Exp,
                                                    const ESFixedTypeBase* Type,
                                                    uint32 ExpansionTypeID,
                                                    uint32 EnumeratorIndex)
    {
        CheckResourceLimits();
        NumExpressionsTried++;
        // find a unique evaluator still not mapped to a terminal expression
        const uint32 CurrentEvalIdx = PBEUniqueEvalPtrs.back()->GetId();

        if (Opts.StatsLevel >= 4) {
            TheLogger.Log4(Exp->ToString()).Log4("... ");
            TheLogger.Log4("Eval[").Log4(PBEEvalPtrs[CurrentEvalIdx]->GetId()).Log4(
                    "], ");
        }

        uint32 StatusRet = 0;
        bool ConcValid = PBEEvalPtrs[CurrentEvalIdx]->CheckConcreteValidity(Exp,
                                                                            Type,
                                                                            ExpansionTypeID,
                                                                            StatusRet);
        if (!ConcValid) {
            if ((StatusRet & CONCRETE_EVAL_DIST) == 0) {
                if (Opts.StatsLevel >= 4) {
                    TheLogger.Log4("Invalid, Indist.").Log4("\n");
                }
                return DELETE_EXPRESSION;
            }
            if (Opts.StatsLevel >= 4) {
                if ((StatusRet & CONCRETE_EVAL_PART) != 0) {
                    TheLogger.Log4("Invalid, Dist (Partial).").Log4("\n");
                } else {
                    TheLogger.Log4("Invalid, Dist.").Log4("\n");
                }
            }
            NumDistExpressions++;
            return NONE_STATUS;
        }
        if (Opts.StatsLevel >= 4) {
            TheLogger.Log4("Valid.").Log4("\n");
        }

        // Valid expr was found for this example, keep it
        const uint32 TermExprIdx = PBETermExprs.size();
        PBEEvalTermExprMap[PBEEvalPtrs[CurrentEvalIdx]->GetId()] = TermExprIdx;
        PBETermExprs.push_back(GenExpressionBase::ToUserExpression(Exp, this));
        NumDistExpressions++;

        bool IsSetFirstInvalidEval = false;
        bool IsConcValidUsingPrevExpr = false;
        for (uint32 i = CurrentEvalIdx + 1; i < PBEEvalPtrs.size(); ++i) {
            const auto result = PBEEvalTermExprMap.find(PBEEvalPtrs[i]->GetId());

            if (result != PBEEvalTermExprMap.cend()) {
                continue;
            }
            IsConcValidUsingPrevExpr =
                    PBEEvalPtrs[i]->CheckConcreteValidity(Exp,
                                                          Type,
                                                          ExpansionTypeID,
                                                          StatusRet);

            if (IsConcValidUsingPrevExpr) {
                PBEEvalTermExprMap[PBEEvalPtrs[i]->GetId()] = TermExprIdx;
                if (Opts.StatsLevel >= 4) {
                    TheLogger.Log4("Eval[").Log4(i).Log4("], ");
                    TheLogger.Log4("Duplicate valid.").Log4("\n");
                }
            } else {
                if (!IsSetFirstInvalidEval) {
                    PBEUniqueEvalPtrs.push_back(PBEEvalPtrs[i].get());
                    IsSetFirstInvalidEval = true;
                }
                if (Opts.StatsLevel >= 4) {
                    TheLogger.Log4("Eval[").Log4(i).Log4("], ");
                    TheLogger.Log4("Invalid.").Log4("\n");
                }
            }
        }

        // check if a valid terminal expr was found for all examples
        if (PBEEvalTermExprMap.size() == PBEEvalPtrs.size()) {
            ThePhase = PBEPhase::EnumerateConditions;
        } else {
            ConcreteEvaluator::ResetSigStore(PBEUniqueEvalPtrs.back()->GetId());
        }
        Restart = true;
        return STOP_ENUMERATION;
    }

    // For multifunction synthesis
    CallbackStatus CEGSolver::ExpressionCallBack(GenExpressionBase const* const* Exps,
                                                 ESFixedTypeBase const* const* Types,
                                                 uint32 const* ExpansionTypeIDs)
    {
        NumExpressionsTried++;
        NumDistExpressions++;
        CheckResourceLimits();

        if (Opts.StatsLevel >= 4) {
            TheLogger.Log4("Trying Expressions:\n");
            for (uint32 i = 0; i < SynthFuncs.size(); ++i) {
                TheLogger.Log4(i).Log4(
                        (string) ". " + Exps[i]->ToString()).Log4(
                        "\n");
            }
            TheLogger.Log4("\n");
        }

        auto ConcValid =
                ConcEval->CheckConcreteValidity(Exps, Types, ExpansionTypeIDs);
        if (!ConcValid) {
            return NONE_STATUS;
        }
        bool SymbValid = CheckSymbolicValidity(Exps);
        if (SymbValid) {
            this->Complete = true;
            Solutions.push_back(vector<pair<const SynthFuncOperator*,
                                            Expression>>());
            for (uint32 i = 0; i < SynthFuncs.size(); ++i) {
                Solutions.back().push_back(pair<const SynthFuncOperator*,
                                                Expression>(SynthFuncs[i],
                                                            GenExpressionBase::ToUserExpression(
                                                                    Exps[i],
                                                                    this)));
            }
            return STOP_ENUMERATION;
        } else {
            SMTModel TheSMTModel;
            SMTConcreteValueModel ConcSMTModel;
            TP->GetConcreteModel(RelevantVars, TheSMTModel, ConcSMTModel, this);
            ConcEval->AddPoint(ConcSMTModel);
            ConcreteEvaluator::ResetSigStore(0);
            return NONE_STATUS;
        }
    }

    SolutionMap CEGSolver::Solve(const Expression& Constraint)
    {
        NumExpressionsTried = NumDistExpressions = (uint64) 0;
        Solutions.clear();
        // Announce that we're at the beginning of a solve
        Complete = false;
        // Gather all the synth funcs
        auto&& SFSet = SynthFuncGatherer::Do(Constraint);
        SynthFuncs =
                vector<const SynthFuncOperator*>(SFSet.begin(), SFSet.end());
        // Check the spec
        LetBindingChecker::Do(Constraint);
        // Rewrite the spec
        vector<pair<string, string>> ConstRelevantVars;
        RewrittenConstraint = SpecRewriter::Do(this,
                                               Constraint,
                                               BaseAuxVars,
                                               DerivedAuxVars,
                                               SynthFunAppMaps,
                                               ConstRelevantVars,
                                               PBEAntecedentExprs);

        if (Opts.StatsLevel >= 2) {
            TheLogger.Log2("Rewritten Constraint:").Log2("\n");
            TheLogger.Log2(RewrittenConstraint).Log2("\n");
        }

        OrigConstraint = Constraint;

        BaseExprs = vector<SMTExpr>(BaseAuxVars.size() + DerivedAuxVars.size());
        for (auto const& Op : BaseAuxVars) {
            // Set up SMT expressions for  aux vars
            BaseExprs[Op->GetPosition()] =
                    TP->CreateVarExpr(Op->GetName(),
                                      Op->GetEvalType()->GetSMTType());
            // Set up the relevant variables as the aux vars
            // which are essentially universally quantified
            // as well as any aux vars which are used as an
            // argument to a synth function
            RelevantVars.insert(Op->GetName());
        }

        for (auto const& Op : DerivedAuxVars) {
            BaseExprs[Op->GetPosition()] =
                    TP->CreateVarExpr(Op->GetName(),
                                      Op->GetEvalType()->GetSMTType());
        }

        // Assign IDs and delayed bindings to SynthFuncs
        // also gather the grammars
        const uint32 NumSynthFuncs = SynthFuncs.size();
        vector<const ESFixedTypeBase*> SynthFuncTypes(NumSynthFuncs);
        vector<Grammar*> SynthGrammars(NumSynthFuncs);
        for (uint32 i = 0; i < NumSynthFuncs; ++i) {
            auto CurGrammar = SynthFuncs[i]->GetSynthGrammar();
            SynthGrammars[i] = const_cast<Grammar*>(CurGrammar);
            SynthFuncs[i]->SetPosition(i);
            SynthFuncs[i]->SetNumLetVars(CurGrammar->GetNumLetBoundVars());
            SynthFuncs[i]->SetNumParams(CurGrammar->GetFormalParamVars().size());
            SynthFuncTypes[i] = SynthFuncs[i]->GetEvalType();
        }

        // Create the enumerator
        if (NumSynthFuncs == 1) {
            ExpEnumerator = new CFGEnumeratorSingle(this, SynthGrammars[0]);
        } else {
            ExpEnumerator = new CFGEnumeratorMulti(this, SynthGrammars);
        }

        vector<Expression> PBEConstraints;
        vector<vector<const AuxVarOperator*>> PBEBaseAuxVarVecs;
        vector<vector<const AuxVarOperator*>> PBEDerivedAuxVarVecs;
        vector<map<vector<uint32>, uint32>> PBESynthFunAppMap;

        // Check PBE mode and, if so, switch mode and do initialization
        if (ConstRelevantVars.size() == RelevantVars.size() &&
                ConstRelevantVars.size() == PBEAntecedentExprs.size()) {
            TheMode = CEGSolverMode::PBE;
            if (Opts.StatsLevel > 2) {
                TheLogger.Log1("\n").Log1(
                        "Programming-by-example constraints detected").Log1("\n");
            }

            PBEConsequentsInitializer::Do(RewrittenConstraint,
                                          PBEConsequentExprs);
            auto MapIt = SynthFunAppMaps.back().cbegin();
            PBESynthFunAppMap.push_back({{MapIt->first, MapIt->first.size()}});
            PBEInitializeEvals(ConstRelevantVars,
                               PBEConstraints,
                               PBEBaseAuxVarVecs,
                               PBEDerivedAuxVarVecs,
                               PBESynthFunAppMap,
                               SynthFuncTypes);
        } else {
            // Create the concrete evaluator
            ConcEval =
                    new ConcreteEvaluator(this,
                                          RewrittenConstraint,
                                          SynthFuncs.size(),
                                          BaseAuxVars,
                                          DerivedAuxVars,
                                          SynthFunAppMaps,
                                          SynthFuncTypes,
                                          TheLogger);
        }

        // Set up evaluation buffers/stacks for generated expressions
        GenExpressionBase::Initialize();

        uint32 NumRestarts = 0;
        PreSolve();
        do {
            Restart = false;
            for (uint32 i = NumSynthFuncs; i <= Opts.CostBudget && !Complete;
                 ++i) {
                if (Opts.StatsLevel >= 2) {
                    TheLogger.Log1("Trying expressions of size ").Log1(i).Log1(
                            "\n");
                }
                ExpEnumerator->EnumerateOfCost(i);
                if (Restart) {
                    ExpEnumerator->Reset();
                    ++NumRestarts;
                    break;
                }
            }
            if (Restart && Opts.StatsLevel >= 2) {
                TheLogger.Log1("Restarting enumeration... (").Log1(NumRestarts).Log1(
                        ")\n");
            }
        } while (Restart && !Complete);
        // We're done
        PostSolve();

        if (Opts.StatsLevel >= 1) {
            TheLogger.Log1("Tried ").Log1(NumExpressionsTried).Log1(
                    " expressions in all.\n");
            TheLogger.Log1(NumDistExpressions).Log1(" were distinguishable.\n");
            TheLogger.Log1("Needed ").Log1(NumRestarts).Log1(" Restarts.\n");
            double Time, Memory;
            ResourceLimitManager::GetUsage(Time, Memory);
            TheLogger.Log1("Total Time : ").Log1(Time).Log1(" seconds.\n");
            TheLogger.Log1("Peak Memory: ").Log1(Memory).Log1(" MB.\n");
        }

        EndSolve();
        return Solutions;
    }

    void CEGSolver::PBEInitializeEvals(vector<pair<string, string>>& ConstRelevantVars,
                                       vector<Expression>& PBEConstraints,
                                       vector<vector<const AuxVarOperator*>>& PBEBaseAuxVarVecs,
                                       vector<vector<const AuxVarOperator*>>& PBEDerivedAuxVarVecs,
                                       vector<map<vector<uint32>, uint32>>& PBESynthFunAppMap,
                                       vector<const ESFixedTypeBase*>& SynthFuncTypes)
    {
        PBEConstraints.reserve(PBEAntecedentExprs.size());
        PBEBaseAuxVarVecs.resize(PBEAntecedentExprs.size());
        PBEDerivedAuxVarVecs.resize(PBEAntecedentExprs.size());

        assert(SynthFunAppMaps.size() == 1
                       && SynthFunAppMaps.back().size() == PBEAntecedentExprs.size()
                       && "PBE does not support synthesis of multiple functions");

        // TODO: generalize this to functions with arity > 1
        auto AppMapIt = PBESynthFunAppMap.back().cbegin();
        PBEParamMapFixup Fixer(AppMapIt->first);
        for (uint i = 0; i < ConstRelevantVars.size(); ++i) {
            PBEAntecedentExprs[i]->Accept(&Fixer);
            PBEConstraints.push_back(CreateExpression("=>",
                                                      PBEAntecedentExprs[i],
                                                      PBEConsequentExprs[i]));
            BaseAuxVars[i]->SetPosition(0);
            PBEBaseAuxVarVecs[i].push_back(BaseAuxVars[i]);
            DerivedAuxVars[i]->SetPosition(AppMapIt->second);
            PBEDerivedAuxVarVecs[i].push_back(DerivedAuxVars[i]);
        }

        for (uint i = 0; i < ConstRelevantVars.size(); ++i) {
            PBEEvalPtrs.push_back(make_unique<ConcreteEvaluator>(this,
                                                                 PBEConstraints[i],
                                                                 SynthFuncs.size(),
                                                                 PBEBaseAuxVarVecs[i],
                                                                 PBEDerivedAuxVarVecs[i],
                                                                 PBESynthFunAppMap,
                                                                 SynthFuncTypes,
                                                                 TheLogger,
                                                                 i));

            SMTConcreteValueModel Model;
            TP->AddConcreteValueToModel(ConstRelevantVars[i].first,
                                        ConstRelevantVars[i].second,
                                        Model,
                                        this);
            PBEEvalPtrs.back()->AddPBEPoint(Model);
        }
        // initialization for first evaluator
        PBEUniqueEvalPtrs.push_back(PBEEvalPtrs.front().get());
        ConcreteEvaluator::ResetSigStore(0);
    }

    void CEGSolver::EndSolve()
    {
        GenExpressionBase::Finalize();
        ConcreteEvaluator::Finalize();
        delete ConcEval;
        ConcEval = nullptr;
        delete ExpEnumerator;
        ExpEnumerator = nullptr;
    }

} /* End namespace */


//
// CEGSolver.cpp ends here

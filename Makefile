ESOLVER_ROOT=$(realpath .)
PROJECT_NAME=esolver

include $(ESOLVER_ROOT)/Makefile.inc

SRC_DIR=$(ESOLVER_ROOT)/src
OBJ_DIR=$(ESOLVER_ROOT)/obj/$(BUILD_SUFFIX)

ESOLVER_MODULES= \
					descriptions \
					enumerators \
					exceptions \
					expressions \
					partitions \
					scoping \
					solvers \
					solverutils \
					utils \
					values \
					visitors \
					z3interface \
					main-solvers \
					logics \
					memmgmt \
					main \
					external/spookyhash \


ESOLVER_LIB_SOURCES= \
					BVLogic.cpp \
					Builtins.cpp \
					CEGSolver.cpp \
					CFGEnumerator.cpp \
					ConcreteEvaluator.cpp \
					ConcreteValueBase.cpp \
					ConstManager.cpp \
					CrossProductGenerator.cpp \
					ESException.cpp \
					ESType.cpp \
					ESolver.cpp \
					ESolverLogic.cpp \
					ESolverScope.cpp \
					EnumeratorBase.cpp \
					EvalRule.cpp \
					ExpCheckers.cpp \
					ExprManager.cpp \
					ExpressionVisitorBase.cpp \
					FunctorBase.cpp \
					GNCostPair.cpp \
					Gatherers.cpp \
					GenExpression.cpp \
					Grammar.cpp \
					GrammarNodes.cpp \
					Indent.cpp \
					LIALogic.cpp \
					Logger.cpp \
					MemStats.cpp \
					Operators.cpp \
					PartitionGenerator.cpp \
					ResourceLimitManager.cpp \
					ScopeManager.cpp \
					Signature.cpp \
					SpecRewriter.cpp \
					SpookyHash.cpp \
					SymPartitionGenerator.cpp \
					SynthLib2Solver.cpp \
					TheoremProver.cpp \
					TimeValue.cpp \
					TypeManager.cpp \
					UIDGenerator.cpp \
					UserExpression.cpp \
					ValueManager.cpp \
					Z3Objects.cpp \
					Z3TheoremProver.cpp \



ESOLVER_MAIN_SOURCES = \
					ESolverSynthLib.cpp \



VPATH=$(addsuffix /:, $(addprefix $(SRC_DIR)/, $(ESOLVER_MODULES)))

ESOLVER_LIB_OBJS=$(addprefix $(ESOLVER_ROOT)/obj/$(BUILD_SUFFIX)/, $(ESOLVER_LIB_SOURCES:.cpp=.o))
ESOLVER_LIB_DEPS=$(addprefix $(ESOLVER_ROOT)/obj/$(BUILD_SUFFIX)/, $(ESOLVER_LIB_SOURCES:.cpp=.d))

ESOLVER_MAIN_OBJS=$(addprefix $(ESOLVER_ROOT)/obj/$(BUILD_SUFFIX)/, $(ESOLVER_MAIN_SOURCES:.cpp=.o))
ESOLVER_MAIN_DEPS=$(addprefix $(ESOLVER_ROOT)/obj/$(BUILD_SUFFIX)/, $(ESOLVER_MAIN_SOURCES:.cpp=.d))

ESOLVER_LIB_STATIC=$(ESOLVER_ROOT)/lib/$(BUILD_SUFFIX)/libesolver.a
ESOLVER_LIB_DYNAMIC=$(ESOLVER_ROOT)/lib/$(BUILD_SUFFIX)/libesolver.so
ESOLVER_MAIN=$(ESOLVER_ROOT)/bin/$(BUILD_SUFFIX)/esolver-synthlib

default:			debug
debug:				all
opt:				all
optlto:				all
prof:				all
proflto:			all
eprof:				all
eproflto:			all
eopt:				all
eoptlto:			all
fullto:				all
fullltoprof:		all

all:				esolverlib esolver-synthlib

esolverlib:			$(ESOLVER_LIB_STATIC) $(ESOLVER_LIB_DYNAMIC)

esolver-synthlib:   $(ESOLVER_MAIN)

$(ESOLVER_MAIN):    $(ESOLVER_MAIN_DEPS) $(ESOLVER_MAIN_OBJS) \
					$(ESOLVER_LIB_STATIC) $(ESOLVER_LIB_DYNAMIC)

ifeq "x$(VERBOSE_BUILD)" "x"
	@echo "$(LDPRINTNAME) `basename $@`"; \
	$(LD) $(OPTFLAGS) $(ESOLVER_MAIN_OBJS) $(LINKFLAGS) -Wl,-Bstatic -lsynthlib2parser \
	-Wl,-Bdynamic -lboost_program_options -o $@
else
	$(LD) $(OPTFLAGS) $(ESOLVER_MAIN_OBJS) $(LINKFLAGS) -Wl,-Bstatic -lsynthlib2parser \
	-Wl,-Bdynamic -lboost_program_options -o $@
endif

$(ESOLVER_LIB_DYNAMIC):		$(ESOLVER_LIB_DEPS) $(ESOLVER_LIB_OBJS)
ifeq "x$(VERBOSE_BUILD)" "x"
	@echo "$(LDPRINTNAME) `basename $@`"; \
	$(LD) $(OPTFLAGS) -shared -o $@ $(ESOLVER_LIB_OBJS)
else
	$(LD) $(OPTFLAGS) -shared -o $@ $(ESOLVER_LIB_OBJS)
endif

$(ESOLVER_LIB_STATIC):		$(ESOLVER_LIB_DEPS) $(ESOLVER_LIB_OBJS)
ifeq "x$(VERBOSE_BUILD)" "x"
	@echo "$(ARPRINTNAME) `basename $@`"; \
	$(AR) $(ARFLAGS) $@ $(ESOLVER_LIB_OBJS)
else
	$(AR) $(ARFLAGS) $@ $(ESOLVER_LIB_OBJS)
endif

$(OBJ_DIR)/%.d:		%.cpp
ifeq "x$(VERBOSE_BUILD)" "x"
	@set -e; rm -f $@; \
	echo "$(DEPPRINTNAME) `basename $<`"; \
	$(CXX) -MM $(CXXFLAGS) $< > $@.$$$$; \
    sed 's,\($*\)\.o[ :]*,$(OBJ_DIR)/\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$
else
	@set -e; rm -f $@; \
	echo "Calculating dependencies for `basename $<`..."; \
	$(CXX) -MM $(CXXFLAGS) $< > $@.$$$$; \
    sed 's,\($*\)\.o[ :]*,$(OBJ_DIR)/\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$
endif

$(OBJ_DIR)/%.o:		%.cpp
ifeq "x$(VERBOSE_BUILD)" "x"
	@echo "$(CXXPRINTNAME) `basename $<` --> `basename $@`"; \
	$(CXX) $(CXXFLAGS) $(OPTFLAGS) -c $< -o $@
else
	$(CXX) $(CXXFLAGS) $(OPTFLAGS) -c $< -o $@
endif

.PHONY:	clean distclean

clean:
	rm -rf obj/debug/*
	rm -rf obj/opt/*
	rm -rf obj/prof/*
	rm -rf lib/debug/*
	rm -rf lib/opt/*
	rm -rf lib/prof/*
	rm -rf bin/debug/*
	rm -rf bin/opt/*
	rm -rf bin/prof/*

distclean:
	rm -rf obj/debug/*
	rm -rf obj/opt/*
	rm -rf obj/prof/*
	rm -rf lib/debug/*
	rm -rf lib/opt/*
	rm -rf lib/prof/*
	rm -rf bin/debug/*
	rm -rf bin/opt/*
	rm -rf bin/prof/*


ifneq ($(MAKECMDGOALS), clean)
ifneq ($(MAKECMDGOALS), distclean)
-include $(ESOLVER_LIB_DEPS)
endif
endif

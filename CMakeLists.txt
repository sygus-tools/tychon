cmake_minimum_required(VERSION 3.3)
project(tychon CXX)

set(BUILD_TYPES debug opt optlto profgen proflto)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -Wall -Wextra -pedantic\
  -Wno-write-strings -Wno-unused-parameter -Wno-unused-function\
  -D__STDC_LIMIT_MACROS -DDISABLE_STATIC_BP"
  CACHE STRING "Flags used by the C++ compiler for all builds."
  FORCE)

if(NOT CMAKE_BUILD_TYPE)
  message(STATUS "Setting build type to 'debug' as none was specified.")
  message(STATUS "Possible build types include ${BUILD_TYPES}")
  set(CMAKE_BUILD_TYPE debug)
else()
  message(STATUS "Current build type is ${CMAKE_BUILD_TYPE}")
endif()

###############################################################################
## dependencies ###############################################################
###############################################################################

find_package(Boost 1.36.0 REQUIRED COMPONENTS program_options system)

if(NOT Boost_FOUND)
  message(FATAL_ERROR "Boost was not found!! Please set variables BOOST_ROOT or"
    " CMAKE_PREFIX_PATH to search for it. Alternatively, you can manually set variables"
    " Boost_INCLUDE_DIRS and Boost_LIBRARY_DIRS")
endif()

###############################################################################
## compiler flags #########################################################
###############################################################################

set(libesolver_output_path ${CMAKE_SOURCE_DIR}/lib/${CMAKE_BUILD_TYPE})

include_directories(${Boost_INCLUDE_DIRS})
link_directories(${Boost_LIBRARY_DIRS})

if(CMAKE_PREFIX_PATH)
  include_directories(${CMAKE_PREFIX_PATH}/include)
endif()

set(CMAKE_CXX_FLAGS_DEBUG "-ggdb3 -O0 -fno-inline -fno-omit-frame-pointer" CACHE STRING
  "Flags used by the C++ compiler during debug builds."
  FORCE)

set(CMAKE_CXX_FLAGS_OPT "-O2 -g" CACHE STRING
  "Flags used by the C++ compiler during opt builds."
  FORCE)

set(CMAKE_CXX_FLAGS_OPTLTO "-O3 -flto=n" CACHE STRING
  "Flags used by the C++ compiler during optlto builds."
  FORCE)

set(PROFILE_DIR "${CMAKE_SOURCE_DIR}/bin/prof")

set(CMAKE_CXX_FLAGS_PROFGEN "-O3 -fprofile-generate=${PROFILE_DIR}" CACHE STRING
  "Flags used by the C++ compiler during prof builds."
  FORCE)

set(CMAKE_CXX_FLAGS_PROFLTO "-O3 -flto=n -fprofile-use=${PROFILE_DIR}" CACHE STRING
  "Flags used by the C++ compiler during proflto builds."
  FORCE)

###############################################################################
## target definitions #########################################################
###############################################################################

set(tychon_source_files
  src/descriptions/GrammarNodes.cpp
  src/descriptions/Operators.cpp
  src/descriptions/Grammar.cpp
  src/descriptions/Builtins.cpp
  src/descriptions/FunctorBase.cpp
  src/descriptions/ESType.cpp
  src/values/ValueManager.cpp
  src/values/Signature.cpp
  src/values/ConcreteValueBase.cpp
  src/z3interface/Z3TheoremProver.cpp
  src/z3interface/Z3Objects.cpp
  src/z3interface/TheoremProver.cpp
  src/main-solvers/SynthLib2Solver.cpp
  src/scoping/ScopeManager.cpp
  src/scoping/ESolverScope.cpp
  src/partitions/PartitionGenerator.cpp
  src/partitions/CrossProductGenerator.cpp
  src/partitions/SymPartitionGenerator.cpp
  src/external/spookyhash/SpookyHash.cpp
  src/solverutils/ConcreteEvaluator.cpp
  src/solverutils/TypeManager.cpp
  src/solverutils/ConstManager.cpp
  src/solverutils/EvalRule.cpp
  src/logics/BVLogic.cpp
  src/logics/ESolverLogic.cpp
  src/logics/LIALogic.cpp
  src/utils/Indent.cpp
  src/utils/ResourceLimitManager.cpp
  src/utils/UIDGenerator.cpp
  src/utils/GNCostPair.cpp
  src/utils/Logger.cpp
  src/utils/TimeValue.cpp
  src/utils/MemStats.cpp
  src/enumerators/EnumeratorBase.cpp
  src/enumerators/CFGEnumerator.cpp
  src/exceptions/ESException.cpp
  src/solvers/ESolver.cpp
  src/solvers/CEGSolver.cpp
  src/visitors/SpecRewriter.cpp
  src/visitors/Gatherers.cpp
  src/visitors/ExpCheckers.cpp
  src/visitors/ExpressionVisitorBase.cpp
  src/expressions/ExprManager.cpp
  src/expressions/GenExpression.cpp
  src/expressions/UserExpression.cpp
  src/visitors/PBEConsequentsInitializer.cpp
  src/visitors/DecisionTreeNodeLocator.cpp
  src/solverutils/DecisionTreeExprBuilder.cpp
  src/solverutils/DecisionTreeNode.cpp
  src/visitors/ExpressionSizeCounter.cpp)

set(main_source_file src/main/ESolverSynthLib.cpp)

add_library(libsynthlib2parser STATIC IMPORTED)
find_library(libsynthlib2parser_path NAMES "libsynthlib2parser.a")

if(NOT libsynthlib2parser_path)
  message(FATAL_EROR "Could not find synthlib2parser library!")
endif()

if(${CMAKE_BUILD_TYPE} MATCHES "lto")
  add_library(libz3 STATIC IMPORTED)
  find_library(libz3_path NAMES "libz3.a")
else()
  add_library(libz3 SHARED IMPORTED)
  find_library(libz3_path NAMES "libz3.so")
endif()

if(NOT libz3_path)
  message(FATAL_EROR "Could not find z3 library!")
endif()

set_target_properties(libsynthlib2parser PROPERTIES
  IMPORTED_LOCATION ${libsynthlib2parser_path})

set_target_properties(libz3 PROPERTIES
  IMPORTED_LOCATION ${libz3_path})

add_library(libesolver_static STATIC ${tychon_source_files})
add_executable(tychon ${main_source_file})

set_target_properties(libesolver_static PROPERTIES
  ARCHIVE_OUTPUT_DIRECTORY ${libesolver_output_path})

set_target_properties(libesolver_static PROPERTIES
  OUTPUT_NAME esolver
  CLEAN_DIRECT_OUTPUT 1)

set_target_properties(tychon PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin/${CMAKE_BUILD_TYPE})

target_link_libraries(tychon
  libesolver_static
  libsynthlib2parser
  libz3
  -fopenmp
  rt
  ${Boost_LIBRARIES})

###############################################################################
## packaging ##################################################################
###############################################################################

# Install dir is relative to CMAKE_INSTALL_PREFIX which is /usr/local by default
# install(TARGETS esolverlib-static
#   LIBRARY DESTINATION lib
#   ARCHIVE DESTINATION lib
# )

# set(public-headers

# )
#
# # Install header files
# install(FILES ${public-headers} DESTINATION ${CMAKE_INSTALL_PREFIX}/include/synthlib2parser/)

# for uninstalling use the following command
# $ xargs rm < install_manifest.txt

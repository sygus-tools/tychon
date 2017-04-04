cmake_minimum_required(VERSION 3.3)
project(esolver-synthlib CXX)

set(BUILD_TYPES debug opt optlto prof proflto)

if(NOT CMAKE_BUILD_TYPE)
  message(STATUS "Setting build type to 'debug' as none was specified.")
  message(STATUS "Possible build types include: debug, opt, optlto, prof ...")
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

set(synthlib_output_path ${CMAKE_SOURCE_DIR}/lib/${CMAKE_BUILD_TYPE})

set(CMAKE_CXX_FLAGS "-Wall -pedantic -Wno-write-strings\
 -Wno-unused-function -D__STDC_LIMIT_MACROS -DDISABLE_STATIC_BP -pipe -std=c++11"
   CACHE STRING "Flags used by the C++ compiler for all builds."
 FORCE
)

include_directories(${CMAKE_SOURCE_DIR}/src/include ${Boost_INCLUDE_DIRS})
link_directories(${synthlib_output_path} ${Boost_LIBRARY_DIRS})

set(CMAKE_CXX_FLAGS_DEBUG "-ggdb3 -O0 -fno-inline" CACHE STRING
    "Flags used by the C++ compiler during debug builds."
    FORCE)

set(CMAKE_CXX_FLAGS_OPT "-O3" CACHE STRING
    "Flags used by the C++ compiler during opt builds."
    FORCE)

set(CMAKE_CXX_FLAGS_OPTLTO "-O3 -flto" CACHE STRING
    "Flags used by the C++ compiler during optlto builds."
    FORCE)

set(CMAKE_CXX_FLAGS_PROF "-O3 -pg" CACHE STRING
    "Flags used by the C++ compiler during prof builds."
    FORCE)

set(CMAKE_CXX_FLAGS_PROFLTO "-O3 -pg" CACHE STRING
    "Flags used by the C++ compiler during proflto builds."
    FORCE)

###############################################################################
## target definitions #########################################################
###############################################################################

set(esolver_source_dir ${CMAKE_SOURCE_DIR}/src)

set(esolverlib_sources
  ${esolver_source_dir}/descriptions/GrammarNodes.cpp
  ${esolver_source_dir}/descriptions/Operators.cpp
  ${esolver_source_dir}/descriptions/Grammar.cpp
  ${esolver_source_dir}/descriptions/Builtins.cpp
  ${esolver_source_dir}/descriptions/FunctorBase.cpp
  ${esolver_source_dir}/descriptions/ESType.cpp
  ${esolver_source_dir}/values/ValueManager.cpp
  ${esolver_source_dir}/values/Signature.cpp
  ${esolver_source_dir}/values/ConcreteValueBase.cpp
  ${esolver_source_dir}/z3interface/Z3TheoremProver.cpp
  ${esolver_source_dir}/z3interface/Z3Objects.cpp
  ${esolver_source_dir}/z3interface/TheoremProver.cpp
  ${esolver_source_dir}/main-solvers/SynthLib2Solver.cpp
  ${esolver_source_dir}/scoping/ScopeManager.cpp
  ${esolver_source_dir}/scoping/ESolverScope.cpp
  ${esolver_source_dir}/partitions/PartitionGenerator.cpp
  ${esolver_source_dir}/partitions/CrossProductGenerator.cpp
  ${esolver_source_dir}/partitions/SymPartitionGenerator.cpp
  ${esolver_source_dir}/external/spookyhash/SpookyHash.cpp
  ${esolver_source_dir}/solverutils/ConcreteEvaluator.cpp
  ${esolver_source_dir}/solverutils/TypeManager.cpp
  ${esolver_source_dir}/solverutils/ConstManager.cpp
  ${esolver_source_dir}/solverutils/EvalRule.cpp
  ${esolver_source_dir}/logics/BVLogic.cpp
  ${esolver_source_dir}/logics/ESolverLogic.cpp
  ${esolver_source_dir}/logics/LIALogic.cpp
  ${esolver_source_dir}/utils/Indent.cpp
  ${esolver_source_dir}/utils/ResourceLimitManager.cpp
  ${esolver_source_dir}/utils/UIDGenerator.cpp
  ${esolver_source_dir}/utils/GNCostPair.cpp
  ${esolver_source_dir}/utils/Logger.cpp
  ${esolver_source_dir}/utils/TimeValue.cpp
  ${esolver_source_dir}/utils/MemStats.cpp
  ${esolver_source_dir}/enumerators/EnumeratorBase.cpp
  ${esolver_source_dir}/enumerators/CFGEnumerator.cpp
  ${esolver_source_dir}/exceptions/ESException.cpp
  ${esolver_source_dir}/solvers/ESolver.cpp
  ${esolver_source_dir}/solvers/CEGSolver.cpp
  ${esolver_source_dir}/visitors/SpecRewriter.cpp
  ${esolver_source_dir}/visitors/Gatherers.cpp
  ${esolver_source_dir}/visitors/ExpCheckers.cpp
  ${esolver_source_dir}/visitors/ExpressionVisitorBase.cpp
  ${esolver_source_dir}/expressions/ExprManager.cpp
  ${esolver_source_dir}/expressions/GenExpression.cpp
  ${esolver_source_dir}/expressions/UserExpression.cpp
)

set(main_source_file
  ${esolver_source_dir}/main/ESolverSynthLib.cpp
)

add_library(synthlib2parser STATIC IMPORTED)
add_library(z3 STATIC IMPORTED)

#set(CMAKE_FIND_LIBRARY_SUFFIXES ".a;.so")
find_library(synthlib2parser-lib NAMES "libsynthlib2parser.a")
find_library(z3-lib NAMES "libz3.a")

set_target_properties(synthlib2parser PROPERTIES
  IMPORTED_LOCATION ${synthlib2parser-lib}
)

set_target_properties(z3 PROPERTIES
  IMPORTED_LOCATION ${z3-lib}
)

add_library(esolverlib-static STATIC ${esolverlib_sources})
add_executable(esolver
  ${main_source_file}
)

set_target_properties(esolverlib-static PROPERTIES
  OUTPUT_NAME esolverlib
  CLEAN_DIRECT_OUTPUT 1
)

set_target_properties(esolverlib-static PROPERTIES
  ARCHIVE_OUTPUT_DIRECTORY ${synthlib_output_path}
)

set_target_properties(esolver PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin/${CMAKE_BUILD_TYPE}
)

link_directories($ENV{HOME}/prefix/lib)

target_link_libraries(esolver
  -fopenmp
  esolverlib-static
  rt
  synthlib2parser
  z3
  ${Boost_LIBRARIES}
)

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

cmake_minimum_required(VERSION 3.3)
project(tychon CXX)

set(BUILD_TYPES debug opt optlto prof proflto)

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

set(libtychon_output_path ${CMAKE_SOURCE_DIR}/lib/${CMAKE_BUILD_TYPE})

set(CMAKE_CXX_FLAGS "-Wall -pedantic -Wno-write-strings\
 -Wno-unused-function -D__STDC_LIMIT_MACROS -DDISABLE_STATIC_BP -pipe -std=c++11"
   CACHE STRING "Flags used by the C++ compiler for all builds."
 FORCE
)

include_directories(${Boost_INCLUDE_DIRS})
link_directories(${Boost_LIBRARY_DIRS})

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

set(CMAKE_CXX_FLAGS_PROFLTO "-O3 -flto -pg" CACHE STRING
    "Flags used by the C++ compiler during proflto builds."
    FORCE)

###############################################################################
## target definitions #########################################################
###############################################################################

set(tychon_source_dir ${CMAKE_SOURCE_DIR}/src)

set(esolverlib_sources
  ${tychon_source_dir}/descriptions/GrammarNodes.cpp
  ${tychon_source_dir}/descriptions/Operators.cpp
  ${tychon_source_dir}/descriptions/Grammar.cpp
  ${tychon_source_dir}/descriptions/Builtins.cpp
  ${tychon_source_dir}/descriptions/FunctorBase.cpp
  ${tychon_source_dir}/descriptions/ESType.cpp
  ${tychon_source_dir}/values/ValueManager.cpp
  ${tychon_source_dir}/values/Signature.cpp
  ${tychon_source_dir}/values/ConcreteValueBase.cpp
  ${tychon_source_dir}/z3interface/Z3TheoremProver.cpp
  ${tychon_source_dir}/z3interface/Z3Objects.cpp
  ${tychon_source_dir}/z3interface/TheoremProver.cpp
  ${tychon_source_dir}/main-solvers/SynthLib2Solver.cpp
  ${tychon_source_dir}/scoping/ScopeManager.cpp
  ${tychon_source_dir}/scoping/ESolverScope.cpp
  ${tychon_source_dir}/partitions/PartitionGenerator.cpp
  ${tychon_source_dir}/partitions/CrossProductGenerator.cpp
  ${tychon_source_dir}/partitions/SymPartitionGenerator.cpp
  ${tychon_source_dir}/external/spookyhash/SpookyHash.cpp
  ${tychon_source_dir}/solverutils/ConcreteEvaluator.cpp
  ${tychon_source_dir}/solverutils/TypeManager.cpp
  ${tychon_source_dir}/solverutils/ConstManager.cpp
  ${tychon_source_dir}/solverutils/EvalRule.cpp
  ${tychon_source_dir}/logics/BVLogic.cpp
  ${tychon_source_dir}/logics/ESolverLogic.cpp
  ${tychon_source_dir}/logics/LIALogic.cpp
  ${tychon_source_dir}/utils/Indent.cpp
  ${tychon_source_dir}/utils/ResourceLimitManager.cpp
  ${tychon_source_dir}/utils/UIDGenerator.cpp
  ${tychon_source_dir}/utils/GNCostPair.cpp
  ${tychon_source_dir}/utils/Logger.cpp
  ${tychon_source_dir}/utils/TimeValue.cpp
  ${tychon_source_dir}/utils/MemStats.cpp
  ${tychon_source_dir}/enumerators/EnumeratorBase.cpp
  ${tychon_source_dir}/enumerators/CFGEnumerator.cpp
  ${tychon_source_dir}/exceptions/ESException.cpp
  ${tychon_source_dir}/solvers/ESolver.cpp
  ${tychon_source_dir}/solvers/CEGSolver.cpp
  ${tychon_source_dir}/visitors/SpecRewriter.cpp
  ${tychon_source_dir}/visitors/Gatherers.cpp
  ${tychon_source_dir}/visitors/ExpCheckers.cpp
  ${tychon_source_dir}/visitors/ExpressionVisitorBase.cpp
  ${tychon_source_dir}/expressions/ExprManager.cpp
  ${tychon_source_dir}/expressions/GenExpression.cpp
  ${tychon_source_dir}/expressions/UserExpression.cpp
)

set(main_source_file
  ${tychon_source_dir}/main/ESolverSynthLib.cpp
)

add_library(libsynthlib2parser STATIC IMPORTED)
add_library(libz3 SHARED IMPORTED)

set(CMAKE_FIND_LIBRARY_SUFFIXES ".a;.so")
find_library(libsynthlib2parser_path NAMES "synthlib2parser")
find_library(libz3_path NAMES "z3")

set_target_properties(libsynthlib2parser PROPERTIES
  IMPORTED_LOCATION ${libsynthlib2parser_path}
)

set_target_properties(libz3 PROPERTIES
  IMPORTED_LOCATION ${libz3_path}
)

add_library(libtychon_static STATIC ${esolverlib_sources})
add_executable(tychon ${main_source_file})

set_target_properties(libtychon_static PROPERTIES
  OUTPUT_NAME libtychon
  CLEAN_DIRECT_OUTPUT 1
)

set_target_properties(libtychon_static PROPERTIES
  ARCHIVE_OUTPUT_DIRECTORY ${libtychon_output_path}
)

set_target_properties(tychon PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin/${CMAKE_BUILD_TYPE}
)

link_directories($ENV{HOME}/prefix/lib)

target_link_libraries(tychon
  libtychon_static
  libsynthlib2parser
  libz3
  -fopenmp
  rt
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

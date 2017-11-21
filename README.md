# Tychon #

An enumerative SyGuS solver for programming-by-example (PBE).

## Overview

Tychon (currently e3solver) builds on the original enumerative [solver] and
contributes several features. Our main addition is an optimized mode for
programming-by-example. In this mode, we use decision tree unification in order
to incrementally find an expression consistent with all examples.

More details can be found in this [report](https://blog.formallyapplied.com/docs/sygus17.pdf)
and [slides](https://www.react.uni-saarland.de/synt2017/Talks/SyGuSComp17_E3Solver.pdf).

## Structure

The project has three main branches:

  - `original-esolver`. Where we keep the original esolver.
  - `constraint-unified`. Where we apply basic fixes to esolver in order to make it
  solve PBE problems.
  - `master`. Where we merged our latest improved version for PBE.

## Dependencies

We have the following dependencies:

  - Boost libraries `system` and `program_options`
  - z3 library
  - Our version of synthlib2parser which can be found [here]


  [solver]: https://github.com/rishabhs/sygus-comp14/tree/master/solvers/enumerative
  [here]: https://github.com/sygus-tools/synthlib2parser

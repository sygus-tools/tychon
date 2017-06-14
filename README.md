# Tychon #

An enumerative SyGuS solver for programming-by-example (PBE).

## Overview

Tychon (currently e2solver) builds on the original enumerative [solver] and
contributes several fixes and features. Our main addition is an optimized mode for programming-by-example.

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

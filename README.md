# PeanoForte 🎹
A toy proof verifier for Peano-arithmetic written in C.

## Peano axioms
This verifier is built upon the [Peano axioms](https://en.wikipedia.org/wiki/Peano_axioms).
Simplified:

- There is a number called `0`.
- For every number `x`, there is a number `(succ x)`.
- If `(succ x) = (succ y)`, then `x = y`.
- If `PHI(0)` is true and `PHI(x) ==> PHI((succ x))`, then `PHI(x)` is true for all numbers `x`.

## Peano arithmetic
Look at the `examples/arithmetic.pf` example that builds a simple arithmetic in the world of Peano numbers.

## Build dependencies
The parser is built using [flex](https://github.com/westes/flex) and [GNU bison](https://www.gnu.org/software/bison/) which need to be installed.

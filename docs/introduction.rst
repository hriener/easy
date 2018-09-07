Introduction
============

Synopsis
--------

easy is a C++ library for manipulating exclusive-or sum-of-products (ESOP) forms.  An ESOP form is a two-level logic representation that consists of one level of multi-fanin AND-gates, followed by one level of multi-input XOR-gates.

The easy library provides data-structures and algorithms for the verification and synthesis of ESOP forms.  This page gives a brief overview of the implemented algorithms.

Verification
^^^^^^^^^^^^

easy implements simple truth table based algorithms to verify that an ESOP form realizes a completely-specified or incompletely-specified Boolean function or to verify that two ESOP forms are functionally equivalent.  Being truth table based, these methods are fast for ESOP forms of up to 16 Boolean variables.

* ``verify_esop`` verifies if an ESOP form implements a specification provided as completely-specified Boolean function

* ``implements_esop`` verifies if an ESOP form implements a specification provided as incompletely-specified Boolean function

* ``equivalent_esops`` verifies if two ESOP forms implement the same completely-specified Boolean function

Synthesis
^^^^^^^^^

easy implements various heuristic and exact methods to synthesize ESOP forms.

*Synthesis of ESOP forms using decomposition based methods*:
The Boolean function is systematically decomposed and the ESOP form is re-composed from the individual parts.

* ``esop_from_optimum_pkrm`` synthesizes an Pseudo-Kronecker Read-Muller (PKRM) expression (a special case of an ESOP form) using the algorithm described in [R. Drechsler, IEEE Trans. C 48(9), 1999, 987–990] and applies post-optimization to merge distance-1 cubes

* ``esop_from_pprm`` synthesizes an Positive Polarity Read-Muller (PPRM) expression (a special case of an ESOP form) by recursively applying the positive Davio decomposition

*Synthesis of ESOP forms with a fixed number of product terms using Boolean satisfiability*:
The problem of synthesizing an ESOP form with a restricted number of product terms is formulated as a propositional constraint satisfaction problem over Boolean decision variables in such a way that each satisfying assignment to the decision variables corresponds to an ESOP form.  The problem is solved by invoking a decision procedure for Boolean satisfiability.

* ``helliwell_synthesis`` synthesizes an exact ESOP form for an incompletely-specified Boolean function.  Under the hood, the algorithm formulates the synthesis problem using the Helliwell equation.

* ``exact_synthesis`` synthesizes an exact ESOP form for a possibly incompletely-specified Boolean function using a formulating of Boolean learning.

*Simplfications for translating XOR-clauses to CNF*:

* Gauss algorithm identifies and eliminates unnecessary variables from a set of XOR-clauses.

* XOR-clauses are translated to Conjunctive Normal Form (CNF) using a simple greedy strategy commonly known as Paar's method.

*Bounded synthesis with upward or downward search*:
A bounded synthesis procedure enables the search for an ESOP form with a small (or minimum) number of product terms.  A user specifies a lower and an upper bound on the number of product terms.  The procedure synthesizes ESOP forms while decreasing or increasing the bounds until an ESOP form with a minimum number of product terms is found.  Two search strategies are supported: Upward search starts from the lower bound and increases the bound if the synthesis problem is unsatisfiable.  Downward search starts from the upper bound and systematically decreases the bound if the synthesis problem is satisfiable.  The search proceeds until a minimum is obtained.

*Resource constraints*:
The synthesis procedure also supports resource constraints in terms of conflict limits.  If enabled, the SAT procedure is terminated after a fixed number of failing attempts to solve the satisfiability problem.  In this case, the result of intermediate synthesis problem is ``unknown'' and the search procedure proceeds to the next problem.  As a consequence, minimality of the solution cannot be guaranteed; however, the conflict limit allows the algorithm to step-over computationally hard instances of the SAT problem and improves the run-time predictability of the search procedure.

Shell Interface
^^^^^^^^^^^^^^^

``easy_shell`` is a simple shell interface that allows a user to apply the algorithms implemented in the easy library to benchmark problems.  ESOP forms in the PLA format are supported; specification are provided as truth tables.

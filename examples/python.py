#! /usr/bin/env python

from sdd import session, SDD, zero, one, Paths, Values, count_combinations
from itertools import product

with session():

  # Initial empty state
  state_space = zero()

  for value in xrange(3):

    # Construct some states bottom-up. "one" marks the end of a set of affectations variable (value).
    # Such a set is called a "path" in the SDD terminology.
    some_states = one()

    # We have 10 variables.
    for variable in xrange(4):
      # Add a new variable on top of the previous one.
      # Notice that you can use any Python object in place of Values, as long as it
      # is immutable and has the same interface. In fact, Values is just a thin wrapper
      # around a frozenset, redefining __lt__.
      some_states = SDD(variable, Values([value, value * 2]), some_states)

    # accumulate states
    state_space += some_states

  # Don't do it for huge SDDs!
  print state_space

  # Count the different number of combinations (not paths)
  print "--------------------------"
  print count_combinations(state_space), "combinations"

  # Let's show all paths
  # On the fly generation of all paths, with no guarantee on the order.
  for path in Paths(state_space):
    # When iterating on paths, variables are bottom-up.
    print "***************"
    for variable, values in zip(reversed(range(4)), path):
      print "variable", variable, "has values", values
  
  # Let's show all combinations
  # Dangerous for huge SDDs!
  print "--------------------------"
  for path in Paths(state_space):
    values = list(path)
    for c in product(*values):
      for var, value in zip(reversed(range(4)), c):
        print var, "=", value, ",",
      print
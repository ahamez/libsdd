#!/usr/bin/env python

from contextlib import contextmanager
from _sdd import *
from _sdd import _manager

@contextmanager
def session():
  m = _manager()
  yield

class Values(object):

  """
  A set of values on an SDD's arc.
  It's a thin wrapper around a frozenset, in order to provide a comparison operator.
  """

  def __init__(self):
    self.values = frozenset([])

  def __init__(self, values):
    self.values = frozenset(values)

  def __lt__(self, other):
    """Use lexicographic order to compare two sets of values."""
    for l, r in zip(self.values, other.values):
      if l < r: return True
      if l > r: return False
    return len(self.values) < len(other.values)

  def __hash__(self):
    return hash(self.values)

  def __eq__(self, other):
    return self.values == other.values

  def __len__(self):
    return len(self.values)

  def __str__(self):
    return str('{') + ','.join([str(x) for x in self.values]) + str('}')

  def __iter__(self):
    return self.values.__iter__()

  def __or__(self,other):
    return Values(self.values | other.values)

  def __and__(self,other):
    return Values(self.values & other.values)

  def __sub__(self,other):
    return Values(self.values - other.values)

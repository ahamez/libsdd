#!/usr/bin/env python

from contextlib import contextmanager
from _sdd import *
from _sdd import _manager

@contextmanager
def session():
  m = _manager()
  yield

class Values(object):

  def __init__(self):
    self.values = frozenset([])

  def __init__(self, values):
    self.values = frozenset(values)

  def union(self, other):
    return Values(self.values | other.values)

  def intersection(self, other):
    return Values(self.values & other.values)

  def difference(self, other):
    return Values(self.values - other.values)

  def __lt__(self, other):
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
    return self.union(other)

  def __and__(self,other):
    return self.intersection(other)

  def __sub__(self,other):
    return self.difference(other)

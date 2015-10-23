"""Helpers for libais."""


from types import NoneType


def MaybeToNumber(instance):
  """Convert to an int or float if possible."""
  if isinstance(instance, (float, int, NoneType)):
    return instance

  try:
    return int(instance)
  except (TypeError, ValueError):
    pass

  try:
    return float(instance)
  except (TypeError, ValueError):
    pass

  return instance

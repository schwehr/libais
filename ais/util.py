"""Helpers for libais."""


def MaybeToNumber(instance):
  """Convert to an int or float if possible."""
  if isinstance(instance, float) or isinstance(instance, int):
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

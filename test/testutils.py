import re
import six

known_bad = set((
    'addressed',
    'app_id',
    'data',
    'eta',  # TODO(schwehr): Fix this.
    'radio',
    'regional',
    'reserved',
    'structured',
))

precision = 5.0

def TextToNumber(text):
  try:
    return float(text)
  except (TypeError, ValueError):
    return text


def IsNumber(value):
  if isinstance(value, float):
    return True
  if isinstance(value, six.integer_types):
    return True
  return False


def DictDiff(a, b):
  """Compares two dictionaries. Intended to be used in a test to
  compare the actual return value from some function with a
  known-good value.

  Returns a dictionary with the following keys:

    removed: Dict of all key/value pairs in a but not in b.

    added: Dict of all key/value pairs in b but not in a.

    changed: Dict with all keys with different values in a and b.
    Values are pairs of the values from a and b.
  """

  def Compare(x, y):
    if x == y:
      return True
    x = TextToNumber(x)
    y = TextToNumber(y)
    if isinstance(x, six.string_types) and isinstance(y, six.string_types):
      # Collapse strings to just lower case a-z to avoid simple mismatches.
      new_x = re.sub(r'[^a-z]', r'', six.text_type(x).lower())
      new_y = re.sub(r'[^a-z]', r'', six.text_type(y).lower())
      if new_x == new_y:
        return True
    if IsNumber(x) and IsNumber(y):
      if abs(float(x) - float(y)) < precision:
        return True
    return False

  # TODO(redhog): Use sets and make this easier to follow.
  return {
      'removed': {key: a[key] for key in a
                  if key not in b and key not in known_bad},
      'changed': {key: (a[key], b[key]) for key in a
                  if key in b
                  and key not in known_bad
                  and not Compare(a[key], b[key])},
      'added': {key: b[key] for key in b
                if key not in a and key not in known_bad}
  }

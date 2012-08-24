pydablooms
==========

Python language bindings for dablooms. See main dablooms readme
for build and install instructions.

### Example usage
    
    >>> import pydablooms
    >>> bloom = pydablooms.Dablooms(capacity=1000,
    ...                             error_rate=.05,
    ...                             filepath='/tmp/bloom.bin')
    >>> bloom.add('foo', 2)
    1
    >>> bloom.check('bar')
    0
    >>> bloom.delete('foo', 2)
    0
    >>> bloom.check('foo')
    0
    >>> del bloom
    >>> bloom = pydablooms.load_dabloom(capacity=1000,
    ...                                 error_rate=.05,
    ...                                 filepath='/tmp/bloom.bin')
    >>>

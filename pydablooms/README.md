pydablooms
==========

Python language bindings for dablooms. See the main dablooms `README.md`
for build and install instructions. There is also a `test_pydablooms`
target in the Makefile (remember to specify any options to make you used
during the build).


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

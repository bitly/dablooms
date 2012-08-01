Dablooms
========

A Scalable, Counting, Bloom Filter
----------------------------------

### Overview
Bloom filters are a probablistic data structure that provide space-efficient
storage of elements at the cost of possible false postive on membership
queries.

This project aims to demonstrate a novel construction that can scale,
and provide not only the addition of new members, but reliable removal of existing
members.

**Dablooms** implements such a structure that takes additional metadata to classify
elements in order to make an intelligenet decision as to which bloom filter an element
should belong.

### Features
**Dablooms**, in addition to the above, has several features. 
    
* Implemented as a static C library
* Memory mapped
* 4 bit counters
* Pre-write and Post-write counters
* Python wrapper

For performance, the low-level operations are implemented in C. It is
memory mapped which gives us persistence at low cost.  Also, in an effort to
maintain memory efficiency, rather than using integers, or even whole bytes as
counters, we use only 4 bit counters.  Because of the scaling quality this provides
space efficiency without an added cost of additional false positives.

The bloom filter also employees pre-write and post-write counters.  These allow us 
determine failed writes, leaving us with a 'dirty' filter where an element is 
only partially written.

Also included is a wrapper to easily leverage the library in Python.

### Example
    
    >>> import pydablooms
    >>> bloom = pydablooms.Dablooms(capacity=1000,
    ...                            error_rate=.05,
    ...                            filepath='/tmp/bloom.bin',
    ...                            id=1)
    >>> bloom.add('foo', 2)
    1
    >>> bloom.check('bar')
    0
    >>> bloom.delete('foo', 2)
    0
    >>> bloom.check('foo')
    0

### Installing
After you have cloned the repo, type `make`, `make install` (`sudo` maybe needed).

To use a specific version of Python, build directory, or destination 
directory, use the `PYTHON`, `BLDDIR`, and `DESTDIR`, respectively. 
Look at the output of `make help` for more options.

An example build might be `make install PYTHON=python2.7 BLDDIR=/tmp/dablooms/bld DESTDIR=/tmp/dablooms/pkg`

### Testing
To run a quick and dirty test, type `make test`.  This test files uses `dict\words`
and defaults to `/usr/share/dict/words`. If your path differs, you can use the
`WORDS` flag to specific its location, such as `make test WORDS=/usr/dict/words`.

This will run a simple test script that iterates through a word
dictionary and adds each word to dablooms. It iterates again, removing every fifth
element. Lastly, it saves the file, opens a new filter, and iterates a third time 
checking the existence of each word. It prints results of the true negatives, 
false positives, true positives, and false negatives.

The maximum error rate for the filter is, by default, set to .05 (5%) and the
initial capacity is set to 100k.  Since the dictionary is near 500k, we should
have created 4 new filters in order to scale to size.

Check out the performance yourself, and checkout the size of the resulting file!

Background
==========
A Bloom filter is traditionally implemented as an array of `M` bits, where
M is the size of the bloom filter. On initialization all bits are set to zero.
A filter is also parameterized by a constant `k` that defines the number of hash
functions used to set and test bits in the filter.  Each hash function should
output one index in `M`.  When inserting an element `x` into the filter, the bits
in the `k` indices `h1(x), h2(x), ..., hk(X)` are set.

In order to query a Bloom filter, say for element `x`, it suffices to verify if
all bits in indexes `h1(x), h2(x), ..., hk(x)` are set. If one or more of these
bits is not set, then the queried element is definitely not present on the
filter. Otherwise, if all these bits are set, then the element is considered to
be on the filter. Given this procedure, an error probability exists for positive
matches, since the tested indexes might have been set by the insertion of other
elements.

### Counting Bloom Filters
Additionally, the same property that results in false positives *also* makes it 
difficult to remove an element from the filter as there is no
easy means of discerning if another element is hashed to the same bit.
Unsetting a bit that is hashed by multiple elements can cause **false
negatives**.  A means of circumventing this issue can be achieved by using
a counter, instead of a bit, which can be incremented when an element is hashed to a
given location, and decremented upon removal.  Memebership queries rely on whether a
given counter is greater than zero.  This reduces the exceptional
space-efficiency provided by the standard bloom filter.

Another important property of a Bloom filter is it's linear relationship between size 
and capacity that can be stored.  If given a maximum accepatable false positive
ratio, it is straight forward to determine how much space is needed.

If the maximum allowable error probability and the number of elements to store
are both known, it is relatively straightforward to dimension an appropriate
filter. However, it is not always possible to know in advance how many elements
will need to be stored. There is a tradeoff between over-dimensioning filters or
suffering from a ballooning error probability as it fills.

### Scalable Bloom Filters
Almeida, Baquero, Preguiça, Hutchison published a paper in 2006, on Scalable
Bloom Filters, which suggested a means of scaling bloom filters by creating
essentially a list of bloom filters that act as one large bloom filter.  When
more capacity is desired, a new filter is added to the list.

Membership queries are conducted on each filter with the positives
evaluated if the element is found in any one of the filters.  Naively, this
leads to an increasing compounding error probability since the probability
of a the given structure evaluates to:

    1 - 𝚺(1 - P)

It is possible to bound this error probability by adding a reducing tightening 
ratio, `r`. Thus the bound error probablity is represented as:

    1 - 𝚺(1 - P0 * r^i) where r is chosen as 0 < r < 1

And since size is simply a function of an error probability and a size, any
array of growth functions can be applied to scale the size of the bloom filter
as necessary.

However, this does not allow for the removal of elements from the filter.
Unfortunately, since an element can be in any filter, and bloom filters
inherently allow for false positives, a given element may appear to be in two or
more filters.  Thus it is not possible to simply convert each scaling bloom
filter to a counting bloom filter. If an element is inadvertently removed from 
a filter which did not contain it, it would introduce the possibility of
**false negatives**.

If however, an element can be removed from the correct filter, it maintains
the probable integrity of said filter.  Thus, a Scaling, Counting, Bloom Filter
is possible if upon additions and deletions you correctly decide which bloom 
filter contains the element.

### Enter Dablooms
Let us take a step back and ponder the value of a bloom filter.  A bloom filter
gives us cheap, memory efficient set operations, with no actual data stored 
about the given element. Rather, bloom filters allow us to test, with some
given error probability, the membership of an item.  This leads to the natural
hypothesis that the majority of operations performed on bloom filters are the
queries of membership, rather than the addition and removal of elements.  Thus,
for a scaling, counting, bloom filter we can optimize for membership queries at
the expensive of additions and removals.  This expense comes not in performance,
but in the addition of more metadata concerning an element and its relation to
the bloom filter.  With the addition of some sort of identification of an
element, which does not need to be unique as long as it is fairly distributed, it
is possible to correctly determine which filter an element should belong, thus
insuring we maintain the probable integrity of a given bloom filter with
accurate additions and removals.

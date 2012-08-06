[dablooms](http://github.com/bitly/dablooms)
========

A Scalable, Counting, Bloom Filter
----------------------------------

### Overview
This project aims to demonstrate a novel bloom filter implementation that can
scale, and provide not only the addition of new members, but reliable removal
of existing members.

Bloom filters are a probabilistic  data structure that provide space-efficient
storage of elements at the cost of possible false positive on membership
queries.

**dablooms** implements such a structure that takes additional metadata to classify
elements in order to make an intelligent decision as to which bloom filter an element
should belong.

### Features
**dablooms**, in addition to the above, has several features.

* Implemented as a static C library
* Memory mapped
* 4 bit counters
* Sequence counters for clean/dirty checks
* Python wrapper

For performance, the low-level operations are implemented in C.  It is also
memory mapped which provides async flushing and persistence at low cost.
In an effort to maintain memory efficiency, rather than using integers, or
even whole bytes as counters, we use only four bit counters. These four bit
counters allow up to 15 items to share a counter in the map. If more than a
small handful are sharing said counter, the bloom filter would be overloaded
(resulting in excessive false positives anyway) at any sane error rate, so
there is no benefit in supporting larger counters.

The bloom filter also employs change sequence numbers to track operations performed on the bloom
filter. These allow the application to determine failed writes, aka a 'dirty' filter where an
element is only partially written.  Upon restart, this information allows the application to
determine if a filter is clean or dirty and make an appropriate decision.
The counters also provide a means for us to identify a position
at which the bloom filter is valid in order to replay operations to "catch up" to a current state.

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

### Contributing
If you make changes to C portions of dablooms which you would like merged into the
upstream repository, it would help to have your code match our C coding style. We use
[astyle](http://astyle.sourceforge.net/), svn rev 353 or later, on our code, with the
following options:

    astyle --style=1tbs --lineend=linux --convert-tabs --preserve-date \
           --fill-empty-lines --pad-header --indent-switches           \
           --align-pointer=name --align-reference=name --pad-oper -n     <files>

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

## Bloom Filter Basics
Bloom filters are probabilistic data structures that provide
space-efficient storage of elements at the cost of occasional false positives on
membership queries, i.e. a bloom filter may state true on query when it in fact does
not contain said element. A bloom filter is traditionally implemented as an array of
`M` bits, where `M` is the size of the bloom filter. On initialization all bits are
set to zero. A filter is also parameterized by a constant `k` that defines the number
of hash functions used to set and test bits in the filter.  Each hash function should
output one index in `M`.  When inserting an element `x` into the filter, the bits
in the `k` indices `h1(x), h2(x), ..., hk(X)` are set.

In order to query a bloom filter, say for element `x`, it suffices to verify if
all bits in indices `h1(x), h2(x), ..., hk(x)` are set. If one or more of these
bits is not set then the queried element is definitely not present in the
filter. However, if all these bits are set, then the element is considered to
be in the filter. Given this procedure, an error probability exists for positive
matches, since the tested indices might have been set by the insertion of other
elements.

### Counting Bloom Filters: Solving Removals
The same property that results in false positives *also* makes it
difficult to remove an element from the filter as there is no
easy means of discerning if another element is hashed to the same bit.
Unsetting a bit that is hashed by multiple elements can cause **false
negatives**.  Using a counter, instead of a bit, can circumvent this issue.
The bit can be incremented when an element is hashed to a
given location, and decremented upon removal.  Membership queries rely on whether a
given counter is greater than zero.  This reduces the exceptional
space-efficiency provided by the standard bloom filter.

### Scalable Bloom Filters: Solving Scale
Another important property of a bloom filter is its linear relationship between size
and storage capacity. If the maximum allowable error probability and the number of elements to store
are both known, it is relatively straightforward to dimension an appropriate
filter. However, it is not always possible to know how many elements
will need to be stored a priori. There is a trade off between over-dimensioning filters or
suffering from a ballooning error probability as it fills.

Almeida, Baquero, Preguiça, Hutchison published a paper in 2006, on
[Scalable Bloom Filters](http://www.sciencedirect.com/science/article/pii/S0020019006003127),
which suggested a means of scalable bloom filters by creating essentially
a list of bloom filters that act as one large bloom filter. When greater
capacity is desired, a new filter is added to the list.

Membership queries are conducted on each filter with the positives
evaluated if the element is found in any one of the filters.  Naively, this
leads to an increasing compounding error probability since the probability
of the given structure evaluates to:

    1 - 𝚺(1 - P)

It is possible to bound this error probability by adding a reducing tightening
ratio, `r`. As a result, the bounded error probability is represented as:

    1 - 𝚺(1 - P0 * r^i) where r is chosen as 0 < r < 1

Since size is simply a function of an error probability and capacity, any
array of growth functions can be applied to scale the size of the bloom filter
as necessary.  We found it sufficient to pick .9 for `r`.

## Problems with Mixing Scalable and Counting Bloom Filters
Scalable bloom filters do not allow for the removal of elements from the filter.
In addition, simply converting each bloom filter in a scalable bloom filter into
a counting filter also poses problems. Since an element can be in any filter, and
bloom filters inherently allow for false positives, a given element may appear to
be in two or more filters. If an element is inadvertently removed from a filter
which did not contain it, it would introduce the possibility of **false negatives**.

If however, an element can be removed from the correct filter, it maintains
the integrity of said filter, i.e. prevents the possibility of false negatives. Thus,
a scaling, counting, bloom filter is possible if upon additions and deletions
one can correctly decide which bloom filter contains the element.

There are several advantages to using a bloom filters.  A bloom filter
gives the application cheap, memory efficient set operations, with no actual data stored
about the given element. Rather, bloom filters allow the application to test, with some
given error probability, the membership of an item.  This leads to the
conclusion that the majority of operations performed on bloom filters are the
queries of membership, rather than the addition and removal of elements.  Thus,
for a scaling, counting, bloom filter, we can optimize for membership queries at
the expense of additions and removals.  This expense comes not in performance,
but in the addition of more metadata concerning an element and its relation to
the bloom filter.  With the addition of some sort of identification of an
element, which does not need to be unique as long as it is fairly distributed, it
is possible to correctly determine which filter an element belongs to, thereby able
to maintain the integrity of a given bloom filter with accurate additions
and removals.

## Enter dablooms
dablooms is one such implementation of a scaling, counting, bloom filter that takes 
additional metadata during additions and deletions in the form of a monotonically 
increasing integer to classify elements such as a timestamp. This is used during 
additions/removals to easily classify an element into the correct bloom filter 
(essentially a comparison against a range).

dablooms is designed to scale itself using these monotonically increasing identifiers
and the given capacity. When a bloom filter is at capacity, dablooms will create a new
bloom filter using the to-be-added elements identifier as the beginning identifier for
the new bloom filter. Given the fact that the identifiers monotonically increase, new
elements will be added to the newest bloom filter. Note, in theory and as implemented,
nothing prevents one from adding an element to any "older" filter. You just run the
increasing risk of the error probability growing beyond the bound as it becomes
"overfilled".

You can then remove any element from any bloom filter using the identifier to intelligently
pick which bloom filter to remove from.  Consequently, as you continue to remove elements
from bloom filters that you are not continuing to add to, these bloom filters will become
more accurate.

Dablooms: A Scalable, Counting, Bloom Filter
----------------------------------

_Note_: this project has been mostly unmaintained for a while.

### Overview
This project aims to demonstrate a novel Bloom filter implementation that can
scale, and provide not only the addition of new members, but reliable removal
of existing members.

Bloom filters are a probabilistic  data structure that provide space-efficient
storage of elements at the cost of possible false positive on membership
queries.

**dablooms** implements such a structure that takes additional metadata to classify
elements in order to make an intelligent decision as to which Bloom filter an element
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
small handful are sharing said counter, the Bloom filter would be overloaded
(resulting in excessive false positives anyway) at any sane error rate, so
there is no benefit in supporting larger counters.

The Bloom filter also employs change sequence numbers to track operations performed
on the Bloom filter. These allow the application to determine if a write might have
only partially completed (perhaps due to a crash), leaving the filter in an
inconsistent state. The application can thus determine if a filter is ok or needs
to be recreated. The sequence number can be used to determine what a consistent but
out-of-date filter missed, and bring it up-to-date.

There are two sequence numbers (and helper functions to get them): "mem_seqnum" and
"disk_seqnum". The "mem" variant is useful if the user is sure the OS didn't crash,
and the "disk" variant is useful if the OS might have crashed since the Bloom filter
was last changed. Both values could be "0", meaning the filter is possibly
inconsistent from their point of view, or a non-zero sequence number that the filter
is consistent with. The "mem" variant is often non-zero, but the "disk" variant only
becomes non-zero right after a (manual) flush. This can be expensive (it's an fsync),
so the value can be ignored if not relevant for the application. For example, if the
Bloom file exists in a directory which is cleared at boot (like `/tmp`), then the
application can safely assume that any existing file was not affected by an OS crash,
and never bother to flush or check disk_seqnum. Schemes involving batching up changes
are also possible.

The dablooms library is not inherently thread safe, this is the clients responsibility.
Bindings are also not thread safe, unless they state otherwise.

### Installing
Clone the repo, or download and extract a tarball of a tagged version
[from github](https://github.com/bitly/dablooms/tags).
In the source tree, type `make`, `make install` (`sudo` may be needed).
This will only install static and dynamic versions of the C dablooms library "libdablooms".

To use a specific build directory, install prefix, or destination directory for packaging,
specify `BLDDIR`, `prefix`, or `DESTDIR` to make. For example:
`make install BLDDIR=/tmp/dablooms/bld DESTDIR=/tmp/dablooms/pkg prefix=/usr`

Look at the output of `make help` for more options and targets.

Also available are bindings for various other languages:

#### Python (pydablooms)
To install the Python bindings "pydablooms" (currently only compatibly with python 2.x)
run `make pydablooms`, `make install_pydablooms` (`sudo` may be needed).

To use and install for a specific version of Python installed on your system,
use the `PYTHON` option to make. For example: `make install_pydablooms PYTHON=python2.7`.
You can override the module install location with the `PY_MOD_DIR` option to make,
and the `BLDDIR` and `DESTDIR` options also affect pydablooms.

The Makefile attempts to determine the python module location `PY_MOD_DIR`
automatically. It prefers a location in `/usr/local`, but you can specify
`PY_MOD_DIR_ARG=--user` to try to use the location which `pip install --user`
would use in your HOME dir. You can instead specify `PY_MOD_DIR_ARG=--system`
to prefer the normal/central system python module dir.

See pydablooms/README.md for more info.

#### Go (godablooms)
The Go bindings "godablooms" are not integrated into the Makefile.
Install libdablooms first, then look at `godablooms/README.md`

### Contributing
If you make changes to C portions of dablooms which you would like merged into the
upstream repository, it would help to have your code match our C coding style. We use
[astyle](http://astyle.sourceforge.net/), svn rev 353 or later, on our code, with the
following options:

    astyle --style=1tbs --lineend=linux --convert-tabs --preserve-date \
           --fill-empty-lines --pad-header --indent-switches           \
           --align-pointer=name --align-reference=name --pad-oper -n     <files>

### Testing
To run a quick and dirty test, type `make test`. This test uses a list of words
and defaults to `/usr/share/dict/words`. If your path differs, you can use the
`WORDS` flag to specific its location, such as `make test WORDS=/usr/dict/words`.

This will run a simple test that iterates through a word list and
adds each word to dablooms. It iterates again, removing every fifth
element. Lastly, it saves the file, opens a new filter, and iterates a third time
checking the existence of each word. It prints results of the true negatives,
false positives, true positives, and false negatives, and the false positive rate.

The false positive rate is calculated by "false positives / (false positivies + true negatives)".
That is, what rate of real negatives are false positives. This is the interesting
statistic because the rate of false negatives should always be zero.

The test uses a maximum error rate of .05 (5%) and an initial capacity of 100k. If
the dictionary is near 500k, we should have created 4 new filters in order to scale to size.

A second test adds every other word in the list, and removes no words, causing each
used filter to stay at maximum capacity, which is a worse case for accuracy.

Check out the performance yourself, and checkout the size of the resulting file!

## Bloom Filter Basics
Bloom filters are probabilistic data structures that provide
space-efficient storage of elements at the cost of occasional false positives on
membership queries, i.e. a Bloom filter may state true on query when it in fact does
not contain said element. A Bloom filter is traditionally implemented as an array of
`M` bits, where `M` is the size of the Bloom filter. On initialization all bits are
set to zero. A filter is also parameterized by a constant `k` that defines the number
of hash functions used to set and test bits in the filter.  Each hash function should
output one index in `M`.  When inserting an element `x` into the filter, the bits
in the `k` indices `h1(x), h2(x), ..., hk(X)` are set.

In order to query a Bloom filter, say for element `x`, it suffices to verify if
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
space-efficiency provided by the standard Bloom filter.

### Scalable Bloom Filters: Solving Scale
Another important property of a Bloom filter is its linear relationship between size
and storage capacity. If the maximum allowable error probability and the number of elements to store
are both known, it is relatively straightforward to dimension an appropriate
filter. However, it is not always possible to know how many elements
will need to be stored a priori. There is a trade off between over-dimensioning filters or
suffering from a ballooning error probability as it fills.

Almeida, Baquero, Preguiça, Hutchison published a paper in 2006, on
[Scalable Bloom Filters](http://www.sciencedirect.com/science/article/pii/S0020019006003127),
which suggested a means of scalable Bloom filters by creating essentially
a list of Bloom filters that act as one large Bloom filter. When greater
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
array of growth functions can be applied to scale the size of the Bloom filter
as necessary.  We found it sufficient to pick .9 for `r`.

## Problems with Mixing Scalable and Counting Bloom Filters
Scalable Bloom filters do not allow for the removal of elements from the filter.
In addition, simply converting each Bloom filter in a scalable Bloom filter into
a counting filter also poses problems. Since an element can be in any filter, and
Bloom filters inherently allow for false positives, a given element may appear to
be in two or more filters. If an element is inadvertently removed from a filter
which did not contain it, it would introduce the possibility of **false negatives**.

If however, an element can be removed from the correct filter, it maintains
the integrity of said filter, i.e. prevents the possibility of false negatives. Thus,
a scaling, counting, Bloom filter is possible if upon additions and deletions
one can correctly decide which Bloom filter contains the element.

There are several advantages to using a Bloom filter. A Bloom filter gives the
application cheap, memory efficient set operations, with no actual data stored
about the given element. Rather, Bloom filters allow the application to test,
with some given error probability, the membership of an item. This leads to the
conclusion that the majority of operations performed on Bloom filters are the
queries of membership, rather than the addition and removal of elements. Thus,
for a scaling, counting, Bloom filter, we can optimize for membership queries at
the expense of additions and removals. This expense comes not in performance,
but in the addition of more metadata concerning an element and its relation to
the Bloom filter.  With the addition of some sort of identification of an
element, which does not need to be unique as long as it is fairly distributed, it
is possible to correctly determine which filter an element belongs to, thereby able
to maintain the integrity of a given Bloom filter with accurate additions
and removals.

## Enter dablooms
dablooms is one such implementation of a scaling, counting, Bloom filter that takes
additional metadata during additions and deletions in the form of a (generally)
monotonically  increasing integer to classify elements (possibly a timestamp).
This is used during additions/removals to easily determine the correct Bloom filter
for an element (each filter is assigned a range). Checking an item against the Bloom
filter, which is assumed to be the dominant activity, does not use the id (it works
like a normal scaling Bloom filter).

dablooms is designed to scale itself using these identifiers and the given capacity.
When a Bloom filter is at capacity, dablooms will create a new Bloom filter which
starts at the next id after the greatest id of the previous Bloom filter. Given the
fact that the identifiers monotonically increase, new elements will be added to the
newest Bloom filter. Note, in theory and as implemented, nothing prevents one from
adding an element to any "older" filter. You just run the increasing risk of the
error probability growing beyond the bound as it becomes "overfilled".

You can then remove any element from any Bloom filter using the identifier to intelligently
pick which Bloom filter to remove from.  Consequently, as you continue to remove elements
from Bloom filters that you are not continuing to add to, these Bloom filters will become
more accurate.

The "id" of an element does not need to be known to check the Bloom filter, but does need
to be known when the element is removed (and the same as when it was added). This might
be convenient if the item already has an appropriate id (almost always increasing for new
items) associated with it.

### Example use case
There is a database with a collection of entries. There is a series of items, each of which
you want to look up in the database; most will have no entry in the database, but some
will. Perhaps it's a database of spam links. If you use dablooms in front of the database,
you can avoid needing to check the database for almost all items which won't be found in
it anyway, and save a lot of time and effort. It's also much easier to distribute the
Bloom filter than the entire database. But to make it work, you need to determine an "id"
whenever you add to or remove from the Bloom filter. You could store the timestamp when
you add the item to the database as another column in the database, and give it to
`scaling_bloom_add()` as well. When you remove the item, you look it up in the database
first and pass the timestamp stored there to `scaling_bloom_remove()`. The timestamps for
new items will be equal or greater, and definitely greater over time. Instead of
timestamps, you could also use an auto-incrementing index. Checks against the Bloom
don't need to know the id and should be quick. If a check comes back negative, you can be
sure the item isn't in the database, and skip that query completely. If a check comes
back positive, you have to query the database, because there's a slight chance that the
item isn't actually in there.

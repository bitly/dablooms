cl-dablooms
===========

Common Lisp CFFI wrapper for the dablooms library

## Dependencies

You will need an ANSI-compliant Common Lisp implementation (I use SBCL)
and the [quicklisp](http://www.quicklisp.org/) library manager. The first
time you use quicklisp's quickload to load cl-dablooms, it will install
CFFI (and its dependencies).

## Usage

1. Install the dablooms shared library on your system library path
2. CD into the cl-dablooms directory and start your LISP REPL.
3. Load cl-dablooms into the repl with `(ql:quickload "cl-dablooms")`

Example:

    (dablooms:with-filter bloom (:capacity 1000
                                 :error-rate .05
                                 :filename "/tmp/bloom.bin")
      
      (dablooms:add bloom "foo" 2)
      
      (dablooms:check bloom "bar")

      (dablooms:remove bloom "foo" 2)

      (dablooms:check bloom "foo"))

The `with-filter` macro will ensure that the filter is destroyed after the body 
is finished running. You can make a bloom filter manually using `make-filter` and
the same keyword arguments, but you will have to clean it up manually 
afterwards using `destroy-filter`.

The key argument to the `add` `check` and `remove` functions can be either
strings or symbols. However, the symbols and strings do not have separate 
namespaces. That is, `(dablooms:check bloom "foo")` and 
`(dablooms:check bloom :foo)` are equivalent. 


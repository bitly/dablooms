(in-package :dablooms)

(define-foreign-library libdablooms
  (t (:default "libdablooms")))

(use-foreign-library libdablooms)

(defctype scaling-bloom :pointer)

(defcfun "new_scaling_bloom" scaling-bloom
  (capacity :unsigned-int) 
  (error_rate :double) 
  (filename :string))

(defcfun "new_scaling_bloom_from_file" scaling-bloom
  (capacity :unsigned-int)
  (error_rate :double)
  (filename :string))

(defcfun "free_scaling_bloom" :boolean
  (bloom scaling-bloom))

(defcfun "scaling_bloom_add" :boolean
  (bloom scaling-bloom)
  (s :string)
  (len :uint64)
  (id :uint64))

(defcfun "scaling_bloom_remove" :boolean
  (bloom scaling-bloom)
  (s :string)
  (len :uint64)
  (id :uint64))

(defcfun "scaling_bloom_check" :boolean
  (bloom scaling-bloom)
  (s :string)
  (len :uint64))

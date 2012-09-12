(in-package :cl-user)

(defpackage :dablooms
  (:use :cl :cffi)
  (:export #:make-filter #:destroy-filter #:with-filter 
           #:add #:check #:remove)
  (:shadow #:remove))

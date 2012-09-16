(in-package :cl-user)

(defpackage :cl-dablooms-asd
  (:use :cl :asdf))

(in-package :cl-dablooms-asd)

(defsystem :cl-dablooms
  :description "CFFI bindings to bit.ly's Dablooms library"
  :version "0.9.0"
  :author "Zhehao Mao"
  :license "MIT"
  :components ((:file "package")
               (:file "bindings")
               (:file "wrappers"))
  :depends-on (:cffi))

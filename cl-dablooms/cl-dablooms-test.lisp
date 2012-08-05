(ql:quickload 'cl-dablooms)

(dablooms:with-filter bloom (:capacity 1000
                             :error-rate 0.5
                             :filename "testbloom.bin"
                             :reset t)

  (when (dablooms:check bloom "test")
    (error "\"test\" found before insertion~%"))

  (unless (dablooms:add bloom "test" 2)
    (error "failed to insert \"test\"~%"))

  (unless (dablooms:check bloom "test")
    (error "\"test\" not found after insertion~%"))

  (unless (dablooms:remove bloom "test" 2)
    (error "failed to remove \"test\" from filter"))

  (when (dablooms:check bloom "test")
    (error "\"test\" found after removal"))
  
  (format t "----> CL-Dablooms: All tests passed"))

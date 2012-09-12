(in-package :dablooms)

(defun bloom-key (key)
  (cond ((stringp key) key)
        ((symbolp key) (symbol-name key))
        (t (write-to-string key))))

(defmacro make-filter (&key (capacity 1000) (error-rate .05) 
                            filename (reset nil))
  `(if (and (not ,reset) (probe-file ,filename)) 
     (new-scaling-bloom-from-file
       ,capacity (coerce ,error-rate 'double-float) ,filename)
     (new-scaling-bloom 
       ,capacity (coerce ,error-rate 'double-float) ,filename)))

(defmacro add (bloom key id)
  `(scaling-bloom-add ,bloom (bloom-key ,key) (length (bloom-key ,key)) ,id))

(defmacro remove (bloom key id)
  `(scaling-bloom-remove ,bloom (bloom-key ,key) (length (bloom-key ,key)) ,id))

(defmacro check (bloom key)
  `(scaling-bloom-check ,bloom (bloom-key ,key) (length (bloom-key ,key))))

(defmacro destroy-filter (bloom)
  `(free-scaling-bloom ,bloom))

(defmacro with-filter (filter-name filter-args &rest body)
  (let ((result-name (gensym)))
    `(let* ((,filter-name (make-filter ,@filter-args))
            (,result-name (progn ,@body)))
       (destroy-filter ,filter-name)
       ,result-name)))

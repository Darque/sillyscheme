(display
  (((lambda (X)
      ((lambda (procedure)
         (X (lambda (arg arg2) ((procedure procedure) arg arg2))))
       (lambda (procedure)
         (X (lambda (arg arg2) ((procedure procedure) arg arg2))))))
    (lambda (func-arg)
      (lambda (n ac)
        (if (= 0 n)
          ac
          (func-arg (- n 1) (* n ac))))))
   10 1))
(newline)

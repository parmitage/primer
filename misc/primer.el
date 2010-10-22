;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;; A simple emacs major mode for editing Primer code ;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

(require 'generic-x)

(define-generic-mode 'primer-mode
  '("#")
  '("fn" "end" "if" "then" "else" "where" "as")
  '(("=" . 'font-lock-operator)
    ("!" . 'font-lock-operator)
    (":" . 'font-lock-operator)
    ("+" . 'font-lock-operator)
    ("-" . 'font-lock-operator)
    ("*" . 'font-lock-operator)
    ("/" . 'font-lock-operator)
    ("<" . 'font-lock-operator)
    (">" . 'font-lock-operator)
    ("[0-9]+" . 'font-lock-variable-name-face))
  '(".pri\\'")
  nil
  "Major mode for editing Primer code.")

(provide 'primer-mode)
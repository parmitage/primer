;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;; A simple emacs major mode for editing Primer code ;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

(require 'generic-x)

(define-generic-mode 'primer-mode
  '("#")
  '("fn" "end" "if" "then" "else" "where" "is" "at" "as"
    "and" "or" "not" "mod" "true" "false"
    "int" "float" "char" "bool" "string" "list" "lambda")
  '(("=" . 'font-lock-operator)
    ("!" . 'font-lock-operator)
    (":" . 'font-lock-operator)
    ("+" . 'font-lock-operator)
    ("-" . 'font-lock-operator)
    ("*" . 'font-lock-operator)
    ("/" . 'font-lock-operator)
    ("<" . 'font-lock-operator)
    (">" . 'font-lock-operator)
    ("[A-Za-z][A-Za-z0-9]*" . 'font-lock-variable-name-face))
  '(".pri\\'")
  nil
  "Major mode for editing Primer code.")

(provide 'primer-mode)
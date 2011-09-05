;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;; A simple emacs major mode for editing Primer code ;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

(defvar primer-mode-hook nil)

(defvar primer-mode-map
  (let ((map (make-keymap)))
    (define-key map "\C-j" 'newline-and-indent)
    map))

;;;###autoload
(add-to-list 'auto-mode-alist '("\\.pri\\'" . primer-mode))

(defconst primer-font-lock-keywords
  (list
   '("^#.*"
     . font-lock-comment-face)
   '("\\<\\(else\\|if\\|let\\|then\\|using\\|\\|pragma\\|val\\|fun\\|in\\|match\\)\\>"
     . font-lock-keyword-face)
   '("\\(!=\\|\\+\\+\\|->\\|\\.\\.\\|<=\\|>=\\\|[&*+/<=>|~^-]\\)"
     . font-lock-builtin-face)
   '("\\<\\(and\\|mod\\|at\\|as\\|not\\|or\\)\\>"
     . font-lock-builtin-face)
   '("\\<\\(true\\|false\\|int\\|float\\|char\\|bool\\|string\\)\\>" . font-lock-constant-face)))

(defun primer-mode ()
  "Emacs major mode for editing Primer programs"
  (interactive)
  (kill-all-local-variables)
  ;(set-syntax-table primer-mode-syntax-table)
  (use-local-map primer-mode-map)
  (set (make-local-variable 'font-lock-defaults) '(primer-font-lock-keywords))
  ;(set (make-local-variable 'indent-line-function) 'primer-indent-line)
  (setq major-mode 'primer-mode)
  (setq mode-name "Primer")
  (run-hooks 'primer-mode-hook))

(provide 'primer-mode)
;;; turtle-mode.el --- Turtle editing support package

;; Author:   Martin Grabmueller <mgrabmue@cs.tu-berlin.de>
;; Keywords: languages

;;; Commentary:

;; A major mode for editing Turtle code.  It provides syntax
;; highlighting for Turtle code, does some limited context-dependant
;; indenting, and supports a native compile command.

;;; Code:

(defconst turtle-mode-version "1.0"
  "The version number of Turtle mode.")

(defgroup turtle nil
  "Major mode for editing Turtle code."
  :prefix "turtle-"
  :group 'languages)

(defconst turtle-emacs-major-version
  (if (boundp 'emacs-major-version)
      emacs-major-version
    (string-match "\\([0-9]+\\)\\.\\([0-9]+\\)" emacs-version)
    (string-to-int (substring emacs-version
			      (match-beginning 1) (match-end 1))))
  "Major version number of Emacs.")

(defconst turtle-emacs-minor-version
  (if (boundp 'emacs-minor-version)
      emacs-minor-version
    (string-match "\\([0-9]+\\)\\.\\([0-9]+\\)" emacs-version)
    (string-to-int (substring emacs-version
			      (match-beginning 2) (match-end 2))))
  "Minor version number of Emacs.")

(defconst turtle-xemacs-p (string-match "Lucid\\|XEmacs" emacs-version)
  "Non-nil when running under XEmacs or Lucid Emacs.")

(defvar turtle-menu-items '(turtle-menu-base-items
			    turtle-menu-compile-items
			    turtle-menu-version-items)
  "*List of menu item list to combine to create Turtle mode menu.

External programs which temporary add menu items to the Turtle mode
menu use this variable.  Please use the function `add-hook' to add
items.

Please call the function `turtle-menu-init' after every change to this
variable.")

(defvar turtle-menu-base-items
  '(("Indent"
     (("Indent Line" turtle-indent-line)))
    ("Edit"
     (("Comment Region" comment-region
       (if turtle-xemacs-p (mark) mark-active))
      ("Uncomment Region" turtle-uncomment-region
       (if turtle-xemacs-p (mark) mark-active))))
    nil
    ("Beginning of Subroutine" turtle-beginning-of-subroutine)
    ("End of Subroutine" turtle-end-of-subroutine)
    ("Mark Subroutine" turtle-mark-subroutine))
  "*Description of menu used in Turtle mode.

This variable must be a list. The elements are either nil representing
a horisontal line or a list with two or three elements.  The first is
the name of the menu item, the second is the function to call, or a
submenu, on the same same form as ITEMS.  The third optional argument
is an expression which is evaluated every time the menu is displayed.
Should the expression evaluate to nil the menu item is ghosted.

Example:
    '((\"Func1\" function-one)
      (\"SubItem\"
       ((\"Yellow\" function-yellow)
        (\"Blue\" function-blue)))
      nil
      (\"Region Funtion\" spook-function midnight-variable))

Call the function `turtle-menu-init' after modifying this variable.")

(defvar turtle-menu-compile-items
  '(("Compile"
     (("Compile Buffer" turtle-compile)
      ("Next Error"     next-error))))
  "*Description of the Compile menu used by Turtle mode.

Please see the documentation of `turtle-menu-base-items'.")

(defvar turtle-menu-version-items
  '(nil
    ("Version" turtle-mode-version))
  "*Description of the version menu used in Turtle mode.")

(defvar turtle-mode-syntax-table nil
  "Syntax table in use in Turtle buffers.")

;; FIXME::martin: make this a relative command name.
(defcustom turtle-compile-command "../turtle/turtle"
  "Command to compile Turtle programs."
  :type 'string
  :group 'turtle)

(if turtle-mode-syntax-table
    ()
  (let ((table (make-syntax-table)))
    (modify-syntax-entry ?\\ "\\" table)
    (modify-syntax-entry ?/  ". 124b" table)
    (modify-syntax-entry ?\n "> b" table)
    (modify-syntax-entry ?\^m "> b" table)
    (modify-syntax-entry ?*  ". 23"   table)
    (modify-syntax-entry ?+ "." table)
    (modify-syntax-entry ?- "." table)
    (modify-syntax-entry ?= "." table)
    (modify-syntax-entry ?< "." table)
    (modify-syntax-entry ?> "." table)
    (modify-syntax-entry ?\' "\"" table)
    (modify-syntax-entry ?? "_" table)
    (modify-syntax-entry ?! "_" table)
    (setq turtle-mode-syntax-table table)))

(defvar turtle-mode-map nil
  "Keymap used in Turtle mode.")

(if turtle-mode-map
    ()
  (let ((map (make-sparse-keymap)))
    (define-key map "\^i" 'turtle-tab)
    (define-key map "\C-cf" 'turtle-fun)
    (define-key map "\C-cc" 'turtle-constraint)
    (define-key map "\C-j"  'turtle-newline)
    (define-key map "\C-c\C-z" 'suspend-emacs)
    (define-key map "\C-c\C-c" 'turtle-compile)
    (define-key map "\C-\M-b" 'turtle-beginning-of-subroutine)
    (define-key map "\C-\M-f" 'turtle-end-of-subroutine)
    (define-key map "\C-\M-h" 'turtle-mark-subroutine)
    (define-key map "\C-c\C-t\C-i" 'turtle-import)
    (define-key map "\C-c\C-t\C-x" 'turtle-export)
    (define-key map "\C-c\C-t\C-t" 'turtle-module-template)
    (define-key map "\C-c\C-t\C-h" 'turtle-find-handcoded-part)
    (setq turtle-mode-map map)))

(defcustom turtle-indent 2
  "*This variable gives the indentation in Turtle mode."
  :type 'integer
  :group 'turtle)
  
;;;###autoload
(defun turtle-mode ()
  "This is a mode intended to support program development in Turtle.
All control constructs of Turtle can be reached by typing C-c
followed by the first character of the construct.
\\<turtle-mode-map>
  \\[turtle-fun] function
  \\[turtle-constraint] constraint
  \\[suspend-emacs] suspend Emacs
  \\[turtle-compile] compile
  \\[next-error] next-error

   `turtle-indent' controls the number of spaces for each indentation.
   `turtle-compile-command' holds the command to compile a Turtle program."
  (interactive)
  (kill-all-local-variables)
  (use-local-map turtle-mode-map)
  (setq major-mode 'turtle-mode)
  (setq mode-name "Turtle")

  (set-syntax-table turtle-mode-syntax-table)

  (turtle-menu-init)
;  (make-local-variable 'indent-line-function)
;  (setq indent-line-function 'c-indent-line)

  (make-local-variable 'require-final-newline)
  (setq require-final-newline t)

  (make-local-variable 'comment-start)
  (setq comment-start "/* ")
  (make-local-variable 'comment-end)
  (setq comment-end " */")
  (make-local-variable 'comment-column)
  (setq comment-column 41)
  (make-local-variable 'comment-start-skip)
  (setq comment-start-skip "/\\*+ *\\|//+ *")

;  (make-local-variable 'comment-indent-function)
;  (setq comment-indent-function 'c-comment-indent)
;  (make-local-variable 'parse-sexp-ignore-comments)
;  (setq parse-sexp-ignore-comments t)
  (make-local-variable 'font-lock-defaults)
  (setq font-lock-defaults
	'((turtle-font-lock-keywords
	   turtle-font-lock-keywords-1 turtle-font-lock-keywords-2)
	  nil nil ((?_ . "w") (?. . "w") (?< . ". 1") (?> . ". 4")) nil
	  ))
  (run-hooks 'turtle-mode-hook))


(defconst turtle-font-lock-keywords-1
  '(
    ;; Module definition, subroutines.
    ("\\<\\(module\\|fun\\|constraint\\)\\>[ \t]*\\(\\sw+\\)?"
     (1 font-lock-keyword-face) (2 font-lock-function-name-face nil t))

    ;; Export/import directives.
    ("\\<\\(export\\|import\\)\\>"
     (1 font-lock-keyword-face)
     (font-lock-match-c-style-declaration-item-and-skip-to-next
      nil (goto-char (match-end 0))
      (1 font-lock-constant-face)))
    )
  "Subdued level highlighting for Turtle mode.")

(defconst turtle-font-lock-keywords-2
  (append turtle-font-lock-keywords-1
   (eval-when-compile
     (let ((turtle-types
	    (regexp-opt
	     '("int" "long" "real" "bool" "char")))
	   (turtle-keywords
	    (regexp-opt
	     '("and" "array" "datatype" "do" "else" "end" "false" "if" "not" 
	       "list" "public" "elsif" "in" "const" "foreign"
	       "of" "or" "out" "prefer" "require" "retract" "return"
	       "string" "then" "true" "type" "var" "while" "null")))
	   (turtle-builtins
	    (regexp-opt
	     '("hd" "tl" "sizeof")))
	   )
       (list
	;; Keywords except those fontified elsewhere.
	(concat "\\<\\(" turtle-keywords "\\)\\>")

	;; Builtins.
	(cons (concat "\\<\\(" turtle-builtins "\\)\\>")
	      'font-lock-builtin-face)

	;; Type names.
	(cons (concat "\\<\\(" turtle-types "\\)\\>") 'font-lock-type-face)
	))))
  "Gaudy level highlighting for Turtle modes.")

(defvar turtle-font-lock-keywords turtle-font-lock-keywords-1
  "Default expressions to highlight in Turtle mode.")

(defun turtle-mode-version ()
  "Return the current version of Turtle mode."
  (interactive)
  (if (interactive-p)
      (message "Turtle mode version %s, written by Martin Grabmueller"
	       turtle-mode-version))
  turtle-mode-version)

(defun turtle-menu-init ()
  "Init menus for Turtle mode.

The variable `turtle-menu-items' contain a description of the Turtle
mode menu.  Normally, the list contains atoms, representing variables
bound to pieces of the menu.

Should any variable describing the menu configuration, this function
should be called."
    (turtle-menu-install "Turtle" turtle-menu-items turtle-mode-map t))


(defun turtle-menu-install (name items keymap &optional popup)
  "Install a menu on Emacs 19 or XEmacs based on an abstract description.

NAME is the name of the menu.

ITEMS is a list. The elements are either nil representing a horisontal
line or a list with two or three elements.  The first is the name of
the menu item, the second the function to call, or a submenu, on the
same same form as ITEMS.  The third optional element is an expression
which is evaluated every time the menu is displayed.  Should the
expression evaluate to nil the menu item is ghosted.

KEYMAP is the keymap to add to menu to.  (When using XEmacs, the menu
will only be visible when this meny is the global, the local, or an
activated minor mode keymap.)

If POPUP is non-nil, the menu is bound to the XEmacs `mode-popup-menu'
variable, i.e. it will popup when pressing the right mouse button.

Please see the variable `turtle-menu-base-items'."
  (cond (turtle-xemacs-p
	 (let ((menu (turtle-menu-xemacs name items keymap)))
	   ;; We add the menu to the global menubar.
	   ;;(funcall (symbol-function 'set-buffer-menubar)
	   ;;         (symbol-value 'current-menubar))
	   (funcall (symbol-function 'add-submenu) nil menu)
	   (setcdr turtle-xemacs-popup-menu (cdr menu))
	   (if (and popup (boundp 'mode-popup-menu))
	       (funcall (symbol-function 'set)
			'mode-popup-menu turtle-xemacs-popup-menu))))
	((>= turtle-emacs-major-version 19)
	 (define-key keymap (vector 'menu-bar (intern name))
	   (turtle-menu-make-keymap name items)))
	(t nil)))


(defun turtle-menu-make-keymap (name items)
  "Build a menu for Emacs 19."
  (let ((menumap (funcall (symbol-function 'make-sparse-keymap)
			  name))
	(count 0)
	id def first second third)
    (setq items (reverse items))
    (while items
      ;; Replace any occurence of atoms by their value.
      (while (and items (atom (car items)) (not (null (car items))))
	(if (and (boundp (car items))
		 (listp (symbol-value (car items))))
	    (setq items (append (reverse (symbol-value (car items)))
				(cdr items)))
	  (setq items (cdr items))))
      (setq first (car-safe (car items)))
      (setq second (car-safe (cdr-safe (car items))))
      (setq third (car-safe (cdr-safe (cdr-safe (car items)))))
      (cond ((null first)
	     (setq count (+ count 1))
	     (setq id (intern (format "separator-%d" count)))
	     (setq def '("--" . nil)))
	    ((and (consp second) (eq (car second) 'lambda))
	     (setq count (+ count 1))
	     (setq id (intern (format "lambda-%d" count)))
	     (setq def (cons first second)))
	    ((symbolp second)
	     (setq id second)
	     (setq def (cons first second)))
	    (t
	     (setq count (+ count 1))
	     (setq id (intern (format "submenu-%d" count)))
	     (setq def (turtle-menu-make-keymap first second))))
      (define-key menumap (vector id) def)
      (if third
	  (put id 'menu-enable third))
      (setq items (cdr items)))
    (cons name menumap)))


(defun turtle-menu-xemacs (name items &optional keymap)
  "Build a menu for XEmacs."
  (let ((res '())
	first second third entry)
    (while items
      ;; Replace any occurence of atoms by their value.
      (while (and items (atom (car items)) (not (null (car items))))
	(if (and (boundp (car items))
		 (listp (symbol-value (car items))))
	    (setq items (append (reverse (symbol-value (car items)))
				(cdr items)))
	  (setq items (cdr items))))
      (setq first (car-safe (car items)))
      (setq second (car-safe (cdr-safe (car items))))
      (setq third (car-safe (cdr-safe (cdr-safe (car items)))))
      (cond ((null first)
	     (setq res (cons "------" res)))
	    ((symbolp second)
	     (setq res (cons (vector first second (or third t)) res)))
	    ((and (consp second) (eq (car second) 'lambda))
	     (setq res (cons (vector first (list 'call-interactively second)
				     (or third t)) res)))
	    (t
	     (setq res (cons (cons first
				   (cdr (turtle-menu-xemacs
					 first second)))
			     res))))
      (setq items (cdr items)))
    (setq res (reverse res))
    ;; When adding a menu to a minor-mode keymap under Emacs 19,
    ;; it disappears when the mode is disabled.  The expression
    ;; generated below imitates this behaviour.
    ;; (This could be expressed much clearer using backquotes,
    ;; but I don't want to pull in every package.)
    (if keymap
	(let ((expr (list 'or
			  (list 'eq keymap 'global-map)
			  (list 'eq keymap (list 'current-local-map))
			  (list 'symbol-value
				(list 'car-safe
				      (list 'rassq
					    keymap
					    'minor-mode-map-alist))))))
	  (setq res (cons ':included (cons expr res)))))
    (cons name res)))




(defun turtle-uncomment-region (beg end)
  "Uncomment all commented lines in the region."
  (interactive "r")
  (comment-region beg end -1))


(defun turtle-newline ()
  "Insert a newline and indent following line like previous line."
  (interactive)
  (let ((hpos (current-indentation)))
    (turtle-indent-line)
    (newline)
    (turtle-indent-line)))

(defun turtle-indent-line ()
  "Indent the current line according to the current context."
  (interactive)
  (let ((oldpos (current-column))
	(oldindent (current-indentation))
	(lines 1))
    (previous-line 1)
    (while (looking-at "^$")
      (previous-line 1)
      (setq lines (+ lines 1)))
    (beginning-of-line)
    (let ((hpos (current-indentation)))
      (if (looking-at
	   "[ \t]*\\<\\(public\\|fun\\|constraint\\|if\\|while\\|in\\|else\\|elsif\\|import\\|export\\|require\\)\\>")
	  (setq hpos (+ hpos turtle-indent)))
      (next-line lines)
      (beginning-of-line)
      (delete-horizontal-space)
      (if (looking-at "\\<\\(end\\|else\\|elsif\\)\\>")
	  (setq hpos (- hpos turtle-indent)))
      (indent-to hpos)
      (beginning-of-line)
      (let ((newpos (+ oldpos (- (current-indentation) oldindent))))
	(if (> newpos 0)
	    (move-to-column newpos))))))

(defun turtle-tab ()
  "Indent to next tab stop."
  (interactive)
  (turtle-indent-line))

(defun turtle-beginning-of-subroutine (&optional arg)
  "Move the cursor to the beginning of the current subroutine."
  (interactive "p")
  (or arg (setq arg 1))
  (if (< arg 0)
      (turtle-end-of-subroutine (- arg))
    (let ((indent (current-indentation)))
      (re-search-backward "^[ \t]*\\<\\(public\\|fun\\|constraint\\)\\>")
      (while (> (current-indentation) indent)
	(re-search-backward "^[ \t]*\\<\\(public\\|fun\\|constraint\\)\\>")))))

(defun turtle-end-of-subroutine (&optional arg)
  "Move the cursor to the end of the current subroutine."
  (interactive "p")
  (or arg (setq arg 1))
  (if (< arg 0)
      (turtle-beginning-of-subroutine (- arg))
    (if (not (looking-at "\\<\\(public\\|fun\\|constraint\\)\\>"))
	(turtle-beginning-of-subroutine))
    (let ((indent (current-indentation)))
      (re-search-forward "\\<end\\>")
      (while (> (current-indentation) indent)
	(re-search-forward "\\<end\\>"))
      (next-line 1)
      (beginning-of-line))))

(defun turtle-mark-subroutine ()
  "Mark the current subroutine."
  (interactive)
  (push-mark (point))
  (turtle-end-of-subroutine)
  ;; Sets the region. In Emacs 19 and XEmacs, we wants to activate
  ;; the region.
  (condition-case nil
      (push-mark (point) nil t)
    (error (push-mark (point))))
  (turtle-beginning-of-subroutine)
  ;; The above function deactivates the mark.
  (if (boundp 'deactivate-mark)
      (funcall (symbol-function 'set) 'deactivate-mark nil)))


;; The following is taken from pascal.el from the Emacs distribution.
;;
(defun turtle-get-default-symbol ()
  "Return symbol around current point as a string."
  (save-excursion
    (buffer-substring (progn
			(skip-chars-backward " \t")
			(skip-chars-backward "a-zA-Z0-9_")
			(point))
		      (progn
			(skip-chars-forward "a-zA-Z0-9_")
			(point)))))

(defun turtle-get-default-qualified-symbol ()
  "Return qualified identifier around point as a string."
  (save-excursion
    (buffer-substring (progn
			(skip-chars-backward " \t")
			(skip-chars-backward "a-zA-Z0-9_.")
			(point))
		      (progn
			(skip-chars-forward "a-zA-Z0-9_.")
			(point)))))

(defun turtle-export ()
  "Add the symbol the point is on to the export list."
  (interactive)
  (save-excursion
    (let ((sym (turtle-get-default-symbol)))
      (if (re-search-backward "^export" nil t)
	  (progn
	    (re-search-forward ";" nil t)
	    (backward-char 1)
	    (insert ", ")
	    (insert sym))
	(progn
	  (if (re-search-backward "^\\(module\\|import\\)" nil t)
	      (progn
		(forward-line 1)
		(insert "\nexport ")
		(insert sym)
		(insert ";\n"))))))))

(defun turtle-import ()
  "Add the symbol the point is on to the import list, leaving point behind it."
  (interactive)
  (let ((sym (turtle-get-default-symbol)))
    (if (re-search-backward "^import" nil t)
	(progn
	  (re-search-forward ";" nil t)
	  (backward-char 1)
	  (insert ", ")
	  (insert sym))
      (progn
	(if (re-search-backward "^module" nil t)
	    (progn
	      (forward-line 1)
	      (insert "\import ")
	      (insert sym)
	      (insert ";\n")))))))

(defun turtle-fun ()
  (interactive)
  (insert "fun ")
  (let ((name (read-string "Name: " ))
	args)
    (insert name " (")
    (insert (read-string "Arguments: ") "): ")
    (setq args (read-string "Result Type: "))
    (insert args)
    (turtle-newline)
    (turtle-newline)
    (insert "end;")
    (end-of-line 0)
    (turtle-tab)))

(defun turtle-constraint ()
  (interactive)
  (insert "constraint ")
  (let ((name (read-string "Name: " ))
	args)
    (insert name " (")
    (insert (read-string "Arguments: ") ")")
    (turtle-newline)
    (turtle-newline)
    (insert "end;")
    (end-of-line 0)
    (turtle-tab)))

(defun turtle-find-handcoded-part ()
  (interactive)
  (find-file (concat (buffer-name) ".i")))

(defun turtle-module-template (module-name)
  (interactive "sModule name:")
  (find-file (concat module-name ".t"))
  (insert "// ")
  (insert module-name)
  (insert ".t -- <One-line module comment>
//
// Copyright (C) ")
  (insert (format-time-string "%Y"))
  (insert " ")
  (insert (user-full-name))
  (insert " <")
  (insert user-mail-address)
  (insert ">")
  (insert "
// 
// This is free software; you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2, or (at your option)
// any later version.
//  
// This software is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this package; see the file COPYING.  If not, write to the
// Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
// MA 02111-1307, USA.

// Commentary:
//
//* <Longer module comment>

module ")
  (insert module-name)
  (insert ";

fun main(argv: list of string): int
  return 0;
end;

// End of ")
  (insert module-name)
  (insert ".t.
")
  (re-search-backward "<One-line" nil t))

  
(defun turtle-compile ()
  (interactive)
  (compile (concat turtle-compile-command " " (buffer-name))))

;; Put files with names ending in `.t' into Turtle-mode automatically.
;;
(let ((a '("\\.t\\'" . turtle-mode)))
  (or (assoc (car a) auto-mode-alist)
      (setq auto-mode-alist (cons a auto-mode-alist))))

(provide 'turtle-mode)

;;; turtle-mode.el ends here

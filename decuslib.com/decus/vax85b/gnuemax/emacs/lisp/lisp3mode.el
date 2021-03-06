;; Lisp mode, and its ideosyncratic commands. 
;; Copyright (C) 1985 Richard M. Stallman. 
 
;; This file is part of GNU Emacs. 
 
;; GNU Emacs is distributed in the hope that it will be useful, 
;; but without any warranty.  No author or distributor 
;; accepts responsibility to anyone for the consequences of using it 
;; or for whether it serves any particular purpose or works at all, 
;; unless he says so in writing. 
 
;; Everyone is granted permission to copy, modify and redistribute 
;; GNU Emacs, but only under the conditions described in the 
;; document "GNU Emacs copying permission notice".   An exact copy 
;; of the document is supposed to have been given to you along with 
;; GNU Emacs so that you can know how you may redistribute it all. 
;; It should be in a file named COPYING.  Among other things, the 
;; copyright notice and this notice must be preserved on all copies. 
 
 
(defvar lisp-mode-syntax-table nil "") 
(defvar lisp-mode-abbrev-table nil "") 
 
(if (not lisp-mode-syntax-table) 
    (let ((i 0)) 
      (setq lisp-mode-syntax-table (make-syntax-table)) 
      (set-syntax-table lisp-mode-syntax-table) 
      (while (< i ?0) 
	(modify-syntax-entry i "_   ") 
	(setq i (1+ i))) 
      (setq i (1+ ?9)) 
      (while (< i ?A) 
	(modify-syntax-entry i "_   ") 
	(setq i (1+ i))) 
      (setq i (1+ ?Z)) 
      (while (< i ?a) 
	(modify-syntax-entry i "_   ") 
	(setq i (1+ i))) 
      (setq i (1+ ?z)) 
      (while (< i 128) 
	(modify-syntax-entry i "_   ") 
	(setq i (1+ i))) 
      (modify-syntax-entry ?  "    ") 
      (modify-syntax-entry ?\t "    ") 
      (modify-syntax-entry ?\n ">   ") 
      (modify-syntax-entry ?\f ">   ") 
      (modify-syntax-entry ?\; "<   ") 
      (modify-syntax-entry ?` "'   ") 
      (modify-syntax-entry ?' "'   ") 
      (modify-syntax-entry ?, "'   ") 
      (modify-syntax-entry ?. "'   ") 
      (modify-syntax-entry ?# "'   ") 
      (modify-syntax-entry ?\" "\"    ") 
      (modify-syntax-entry ?\\ "\\   ") 
      (modify-syntax-entry ?\( "()  ") 
      (modify-syntax-entry ?\) ")(  "))) 
 
(define-abbrev-table 'lisp-mode-abbrev-table ()) 
 
(defvar lisp-mode-map nil "") 
(defvar lisp-interaction-mode-map nil "") 
 
(defun lisp-mode () 
  "Major mode for editing Lisp code. 
Commands: 
Delete converts tabs to spaces as it moves back. 
Tab indents for Lisp; with argument, shifts rest 
 of expression rigidly with the current line. 
Meta-Control-Q does Tab on each line starting within following expression. 
Meta-Control-X evaluates the largest Lisp expression around or after dot. 
Blank lines separate paragraphs.  Semicolons start comments." 
  (interactive) 
  (kill-all-local-variables) 
  (if (not lisp-mode-map) 
      (progn 
       (setq lisp-mode-map (make-sparse-keymap)) 
       (define-key lisp-mode-map "\e\^X" 'eval-defun) 
       (define-key lisp-mode-map "\e\^q" 'indent-sexp) 
       (define-key lisp-mode-map "\177" 'backward-delete-char-untabify) 
       (define-key lisp-mode-map "\t" 'lisp-indent-line))) 
  (use-local-map lisp-mode-map) 
  (set-syntax-table lisp-mode-syntax-table) 
  (setq major-mode 'lisp-mode) 
  (setq mode-name "Lisp") 
  (setq local-abbrev-table lisp-mode-abbrev-table) 
  (make-local-variable 'paragraph-start) 
  (setq paragraph-start (concat "^$\\|" page-delimiter)) 
  (make-local-variable 'paragraph-separate) 
  (setq paragraph-separate paragraph-start) 
  (make-local-variable 'indent-line-function) 
  (setq indent-line-function 'lisp-indent-line) 
  (make-local-variable 'comment-start) 
  (setq comment-start ";") 
  (make-local-variable 'comment-start-skip) 
  (setq comment-start-skip ";* *") 
  (make-local-variable 'comment-column) 
  (setq comment-column 40) 
  (make-local-variable 'comment-indent-hook) 
  (setq comment-indent-hook 'lisp-comment-indent) 
  (and (boundp 'lisp-mode-hook) 
       lisp-mode-hook 
       (funcall lisp-mode-hook))) 
 
(defun lisp-interaction-mode () 
  "Major mode for typing and evaluating Lisp forms. 
Like Lisp mode except that Linefeed evals the Lisp expression 
 before dot, and prints its value into the buffer, advancing dot. 
Commands: 
Delete converts tabs to spaces as it moves back. 
Tab indents for Lisp; with argument, shifts rest 
 of expression rigidly with the current line. 
Meta-Control-Q does Tab on each line starting within following expression. 
Meta-Control-X evaluates the largest Lisp expression around or after dot. 
Paragraphs are separated only by blank lines.  Semicolons start comments." 
 
  (interactive) 
  (lisp-mode) 
  (if (not lisp-interaction-mode-map) 
      (progn 
       (setq lisp-interaction-mode-map (make-sparse-keymap)) 
       (define-key lisp-interaction-mode-map "\e\^X" 'eval-defun) 
       (define-key lisp-interaction-mode-map "\n" 'eval-print-last-sexp) 
       (define-key lisp-interaction-mode-map "\e\^q" 'indent-sexp) 
       (define-key lisp-interaction-mode-map "\177" 'backward-delete-char-untabify) 
       (define-key lisp-interaction-mode-map "\t" 'lisp-indent-line))) 
  (use-local-map lisp-interaction-mode-map) 
  (setq major-mode 'lisp-interaction-mode) 
  (setq mode-name "Lisp Interaction")) 
 
(fset 'scheme-mode 'lisp-mode) 
 
(defun eval-print-last-sexp (arg) 
  "Evaluate sexp before point; print value into current buffer." 
  (interactive "P") 
  (eval-region 
   (let ((stab (syntax-table))) 
     (unwind-protect 
      (save-excursion 
       (set-syntax-table lisp-mode-syntax-table) 
       (forward-sexp -1) 
       (dot)) 
      (set-syntax-table stab))) 
   (dot) 
   (current-buffer))) 
 
(defun lisp-comment-indent (addr) 
  (save-excursion 
   (goto-char addr) 
   (if (looking-at ";;;") 
       (current-column) 
     (if (looking-at ";;") 
	 (let ((tem (calculate-lisp-indent))) 
	   (if (listp tem) (car tem) tem)) 
       comment-column)))) 
 
(defconst lisp-indent-offset nil "") 
(defconst lisp-indent-hook 'lisp-indent-hook "") 
 
(defun lisp-indent-line (&optional whole-exp) 
  "Indent current line as Lisp code. 
With argument, indent any additional lines of the same expression 
rigidly along with this one." 
  (interactive "P") 
  (let ((indent (calculate-lisp-indent)) 
	shift-amt 
	beg end 
	(pos (- (dot-max) (dot)))) 
    (beginning-of-line) 
    (setq beg (dot)) 
    (skip-chars-forward " \t") 
    (if (looking-at "[ \t]*;;;") 
	;; Don't alter indentation of a ;;; comment line. 
	nil 
      (if (listp indent) (setq indent (car indent))) 
      (setq shift-amt (- indent (current-column))) 
      (if (zerop shift-amt) 
	  nil 
	(delete-region beg (dot)) 
	(indent-to indent)) 
      ;; If initial dot was within line's indentation, 
      ;; position after the indentation.  Else stay at same point in text. 
      (if (> (- (dot-max) pos) (dot)) 
	  (goto-char (- (dot-max) pos))) 
      ;; If desired, shift remaining lines of expression the same amaount. 
      (and whole-exp (not (zerop shift-amt)) 
	   (save-excursion 
	    (goto-char beg) 
	    (forward-sexp 1) 
	    (setq end (dot)) 
	    (goto-char beg) 
	    (forward-line 1) 
	    (setq beg (dot)) 
	    (> end beg)) 
	   (indent-code-rigidly beg end shift-amt))))) 
 
(defun calculate-lisp-indent (&optional parse-start) 
  "Return appropriate indentation for current line as Lisp code. 
In usual case returns an integer: the column to indent to. 
Can instead return a list, whose car is the column to indent to. 
This means that following lines at the same level of indentation 
should not necessarily be indented the same way. 
The second element of the list is the buffer position 
of the start of the containing expression." 
  (save-excursion 
    (beginning-of-line) 
    (let ((indent-point (dot)) 
	  state paren-depth 
	  desired-indent 
	  (retry t) 
	  last-sexp 
	  containing-sexp) 
      (if parse-start 
	  (goto-char parse-start) 
	(beginning-of-defun)) 
      (while (< (dot) indent-point) 
	(setq state (parse-partial-sexp (dot) indent-point 0))) 
      (while (progn (setq paren-depth (car state)) 
		    (and retry (> paren-depth 0))) 
	(setq retry nil) 
	(setq last-sexp (car (cdr (cdr state)))) 
	(setq containing-sexp (car (cdr state))) 
	;; Position following last unclosed open. 
	(goto-char (1+ containing-sexp)) 
	;; Is there a complete sexp since then? 
	(if (and last-sexp (> last-sexp (dot))) 
	    ;; Yes.  Does it start on a later line? 
	    (progn 
	     (if (progn (forward-line 1) 
		        (>= last-sexp (dot))) 
		;; Yes. 
		;; Is it the first nonblank thing on its line? 
		(progn 
		  (goto-char last-sexp) 
		  (beginning-of-line) 
		  (skip-chars-forward " \t") 
		  (if (/= (dot) last-sexp) 
		      ;; No.  We can't use it.  Can we use first thing on line?> 
		      (progn 
			(beginning-of-line) 
			(if (= (car (parse-partial-sexp  
				      (dot) 
				      last-sexp)) 
			       0) 
			    ;; Yes, it's at the correct level.  Go there. 
			    (progn 
			      (beginning-of-line) 
			      (skip-chars-forward " \t")) 
			  ;; It loses.  Try moving back one sexp. 
			  (setq state 
			    (parse-partial-sexp 
			      (1+ containing-sexp) 
			      last-sexp 
			      0)) 
			  (setq retry t))))) 
	      (if retry 
		  nil 
		;; Last sexp starts on same line as containing open. 
		(if lisp-indent-offset 
		    (setq desired-indent 
			  (+ lisp-indent-offset 
			     (progn (goto-char containing-sexp) 
				    (current-column)))) 
		  (goto-char (1+ containing-sexp)) 
		  (forward-sexp 1) 
		  (if (> (dot) last-sexp) 
		      ;; If that is too far, indent under the open paren. 
		      (goto-char (1+ containing-sexp)) 
		    ;; Else stop before 1st char of next element. 
		    (parse-partial-sexp (dot) indent-point 
					1 t) 
		    ;; Back up over prefix operators. 
		    (backward-prefix-chars))))) 
	     ;; Dot is at the point to indent under. 
	     ;; Call indentation hook. 
	     (if (and (not retry) lisp-indent-hook) 
		 (setq desired-indent 
		       (funcall lisp-indent-hook indent-point state)))))) 
      (or desired-indent 
	  (setq desired-indent (current-column))) 
      (goto-char indent-point) 
      desired-indent))) 
 
(defun lisp-indent-hook (indent-point state) 
  (let ((normal-indent (current-column))) 
    (save-excursion 
     (goto-char (1+ (car (cdr state)))) 
     (re-search-forward "\\sw\\|\\s_\\|") 
     (if (/= (dot) (car (cdr state))) 
	(let ((function (buffer-substring (progn (forward-char -1) (dot)) 
					  (progn (forward-sexp 1) (dot)))) 
	      method) 
	  (setq method (get (intern-soft function) 'lisp-indent-hook)) 
	  (if (or (eq method 'defun) 
		  (and (null method) 
		       (> (length function) 3) 
		       (string-equal (substring function 0 3) 
				     "def"))) 
	      (lisp-indent-defform state indent-point) 
	    (if (integerp method) 
		(lisp-indent-specform method state indent-point) 
	      (if method 
		  (funcall method state indent-point))))))))) 
 
(defconst lisp-body-indent 2 "") 
 
(defun lisp-indent-specform (count state indent-point) 
  (let ((containing-form-start (car (cdr state))) 
	body-indent) 
    (goto-char containing-form-start) 
    (setq body-indent (+ lisp-body-indent (current-column))) 
    (forward-char 1) 
    (parse-partial-sexp (dot) (dot-max) 1 t) 
    (forward-sexp 1) 
    (parse-partial-sexp (dot) (dot-max) 1 t) 
    (if (or (<= (current-column) body-indent) 
	    (condition-case () 
		(progn 
		 (while (and (> count 0) 
			     (progn 
			      (parse-partial-sexp (dot) (dot-max) 1 t) 
			      (not (eobp)))) 
		   (forward-sexp 1) 
		   (setq count (1- count))) 
		 (or (> count 0) (> (dot) indent-point))) 
	     (error t))) 
        (list normal-indent containing-form-start) 
      (and (condition-case () 
		(progn 
		 (parse-partial-sexp (dot) (dot-max) 1 t) 
		 (or (eobp) 
		     (progn 
		      (forward-sexp 1) 
		      (> (dot) indent-point)))) 
	      (error t)) 
	    body-indent)))) 
 
(defun lisp-indent-defform (state indent-point) 
  (goto-char (car (cdr state))) 
  (forward-line 1) 
  (if (> (dot) (car (cdr (cdr state)))) 
      (progn 
       (goto-char (car (cdr state))) 
       (+ lisp-body-indent (current-column))))) 
 
(put 'let 'lisp-indent-hook 1) 
(put 'let* 'lisp-indent-hook 1) 
(put 'while 'lisp-indent-hook 1) 
(put 'if 'lisp-indent-hook 2) 
(put 'catch 'lisp-indent-hook 1) 
(put 'condition-case 'lisp-indent-hook 2) 
(put 'unwind-protect 'lisp-indent-hook 1) 
(put 'with-output-to-temp-buffer 'lisp-indent-hook 1) 
 
(defun indent-sexp () 
  "Indent each line of the list starting just after dot." 
  (interactive) 
  (let ((indent-stack (list nil)) 
	restart outer-loop-done inner-loop-done state 
	this-indent 
	(next-depth 0)) 
    (save-excursion 
      (forward-sexp 1)) 
    (save-excursion 
      (setq outer-loop-done nil) 
      (while (and (not (eobp)) (not outer-loop-done)) 
	(setq last-depth next-depth) 
	(setq innerloop-done nil) 
	(while (and (not innerloop-done) 
		    (not (and (eobp) (setq outer-loop-done t)))) 
	  (setq state (parse-partial-sexp (dot) (progn (end-of-line) (dot)) 
					  nil nil state)) 
	  (setq next-depth (car state)) 
	  (if (car (nthcdr 4 state)) 
	      (progn (indent-for-comment) 
		     (end-of-line) 
		     (setcar (nthcdr 4 state) nil))) 
	  (if (car (nthcdr 3 state)) 
	      (forward-line 1) 
	    (setq innerloop-done t))) 
	(if (<= next-depth 0) 
	    (setq outer-loop-done t)) 
	(if outer-loop-done 
	    nil 
	  (while (> last-depth next-depth) 
	    (setq indent-stack (cdr indent-stack) 
		  last-depth (1- last-depth))) 
	  (while (< last-depth next-depth) 
	    (setq indent-stack (cons nil indent-stack) 
		  last-depth (1+ last-depth))) 
	  (forward-line 1) 
	  (skip-chars-forward " \t") 
	  (if (or (eobp) (looking-at "[;\n]")) 
	      nil 
	    (if (and (car indent-stack) 
		     (>= (car indent-stack) 0)) 
		(setq this-indent (car indent-stack)) 
	      (let ((val (calculate-lisp-indent 
			  (if (car indent-stack) 
			      (- (car indent-stack)))))) 
		(if (integerp val) 
		    (setcar indent-stack 
			    (setq this-indent val)) 
		  (setcar indent-stack (- (car (cdr val)))) 
		  (setq this-indent (car val))))) 
	    (or (= (current-column) this-indent) 
		(progn 
		 (delete-region (dot) (progn (beginning-of-line) (dot))) 
		 (indent-to this-indent))))))))) 
 
(defun indent-code-rigidly (start end arg) 
  "Indent all lines of code, starting in the region, sideways by ARG columns. 
Does not affect lines starting inside comments or strings, 
assuming that the start of the region is not inside them. 
Called from a program, takes three arguments, START, END and ARG." 
  (interactive "r\np") 
  (let (state) 
    (save-excursion 
     (goto-char end) 
     (setq end (dot-marker)) 
     (goto-char start) 
     (or (bolp) 
	 (setq state (parse-partial-sexp (dot) 
					 (progn 
					  (forward-line 1) (dot)) 
					 nil nil state))) 
     (while (< (dot) end) 
       (or (car (nthcdr 3 state)) (car (nthcdr 4 state)) 
	   ;; If line does not start in string or comment, indent it 
	   (let ((indent (current-indentation))) 
	     (delete-region (dot) (progn (skip-chars-forward "[ \t]") (dot))) 
	     (or (eolp) 
		 (indent-to (max 0 (+ indent arg)) 0)))) 
       (setq state (parse-partial-sexp (dot) 
				       (progn 
					(forward-line 1) (dot)) 
				       nil nil state)))))) 

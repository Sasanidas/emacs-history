;; Run subshell under Emacs.
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
(defvar last-input-start nil 
  "In a shell-mode buffer, marker for start of last unit of input.") 
(defvar last-input-end nil 
  "In a shell-mode buffer, marker for start of last unit of input.") 
 
(defvar shell-mode-map nil "") 
 
(defvar shell-directory-stack nil 
  "List of directories saved by pushd in this buffer's shell.") 
 
(defun shell-mode () 
  "Major mode for interacting with an inferior shell. 
Shell name is same as buffer name, sans the asterisks. 
Return at end of buffer sends line as input. 
Return not at end copies rest of line to end and sends it. 
C-D at end of buffer sends end-of-file as input. 
C-D not at end or with arg deletes or kills characters. 
C-U and C-W are kill commands, imitating normal Unix input editing. 
C-C interrupts the shell or its current subjob if any. 
C-Z stops, likewise.  C-\ sends quit signal, likewise. 
 
cd, pushd and popd commands given to the shell are watched 
by Emacs to keep this buffer's default directory 
the same as the shell's working directory." 
  (interactive) 
  (kill-all-local-variables) 
  (setq major-mode 'shell-mode) 
  (setq mode-name "Shell") 
  (setq mode-line-format  
	"--%1*%1*-Emacs: %17b   %M   %[(%m: %s)%]----%3p--%-") 
  (use-local-map shell-mode-map) 
  (make-local-variable 'shell-directory-stack) 
  (setq shell-directory-stack nil) 
  (make-local-variable 'last-input-start) 
  (setq last-input-start (make-marker)) 
  (make-local-variable 'last-input-end) 
  (setq last-input-end (make-marker))) 
 
(if shell-mode-map 
    nil 
  (setq shell-mode-map (make-keymap)) 
  (define-key shell-mode-map "\^m" 'shell-send-input) 
  (define-key shell-mode-map "\" 'delete-char-or-send-eof) 
  (define-key shell-mode-map "\^U" 'kill-shell-input) 
  (define-key shell-mode-map "\^W" 'backward-kill-word) 
  (define-key shell-mode-map "\^C" 'interrupt-shell-subjob) 
  (define-key shell-mode-map "\^Z" 'stop-shell-subjob) 
  (define-key shell-mode-map "\" 'quit-shell-subjob) 
  (define-key shell-mode-map "\^X\^K" 'kill-output-from-shell) 
  (define-key shell-mode-map "\^Xk" 'show-output-from-shell) 
  (define-key shell-mode-map "\e=" 'copy-last-shell-input)) 
 
(defun shell () 
  "Run an inferior shell, with I/O through buffer *shell*. 
If buffer exists but shell process is not running, make new shell. 
Program used is taken from ESHELL environment variable, 
 or else from SHELL if there is no ESHELL. 
The buffer is put in shell-mode, giving commands for sending input 
and controlling the subjobs of the shell.  See shell-mode. 
See also variable shell-prompt-pattern." 
  (interactive) 
  (let* ((prog (or (getenv "ESHELL") (getenv "SHELL"))) 
	 (name (file-name-nondirectory prog))) 
    (make-shell "shell" prog 
		(concat "if (-f ~/.emacs_" name ")" 
			"source ~/.emacs_" name "\n")))) 
 
(defun make-shell (name program &optional startstring) 
  (let ((buffer (get-buffer-create (concat "*" name "*"))) 
	proc status) 
    (setq proc (get-buffer-process buffer)) 
    (if proc 
	(setq status (process-status proc))) 
    (if (memq status '(run stop)) 
	nil 
      (if proc (delete-process proc)) 
      (setq proc (start-process name buffer program)) 
      (if startstring 
	  (send-string proc startstring)) 
      (setq name (process-name proc))) 
    (switch-to-buffer buffer) 
    (goto-char (dot-max)) 
    (set-mark (dot)) 
    (shell-mode))) 
 
;; Problem is, what mode to use? 
;(defun lisp () 
;  (interactive) 
;  (make-shell "lisp" "lisp")) 
 
;In loaddefs.el now. 
;(defconst shell-prompt-pattern 
;  "^[^ ]*>" 
;  "*Regexp used by Newline command to match subshell prompts. 
;Anything from beginning of line up to the end of what this pattern matches 
;is deemed to be prompt, and is not reexecuted.") 
 
(defun shell-send-input () 
  "Send input to subshell. 
At end of buffer, sends all text after last output 
 as input to the subshell, including a newline inserted at the end. 
Not at end, copies current line to the end of the buffer and sends it, 
after first attempting to discard any prompt at the beginning of the line 
by matching the regexp that is the value of shell-prompt-pattern if possible. 
This regexp should start with ^." 
  (interactive) 
  (end-of-line) 
  (if (eobp) 
      (let ((mark (process-mark (get-buffer-process (current-buffer))))) 
       (newline) 
       (if (/= (dot) mark) 
	   (progn 
	    (move-marker last-input-start mark) 
	    (move-marker last-input-end (dot))))) 
    (beginning-of-line) 
    (re-search-forward shell-prompt-pattern nil t) 
    (let ((copy (buffer-substring (dot) 
				  (progn (forward-line 1) (dot))))) 
      (goto-char (dot-max)) 
      (move-marker last-input-start (dot)) 
      (insert copy) 
      (move-marker last-input-end (dot)))) 
  (save-excursion 
   (goto-char last-input-start) 
   (if (or (looking-at "pushd\n") 
	   (looking-at "popd\n")) 
       (if shell-directory-stack 
	   (progn 
	    (cd (car shell-directory-stack)) 
	    (setq shell-directory-stack (cdr shell-directory-stack))))) 
   (if (looking-at "pushd ") 
       (progn 
	(forward-char 6) 
	(skip-chars-forward " \t") 
	(if (file-directory-p (buffer-substring (dot) (1- last-input-end))) 
	    (progn 
	     (setq shell-directory-stack 
		   (cons default-directory shell-directory-stack)) 
	     (cd (buffer-substring (dot) (1- last-input-end))))))) 
   (if (looking-at "cd\n") 
       (cd (getenv "HOME"))) 
   (if (looking-at "cd ") 
       (progn 
	(forward-char 3) 
	(skip-chars-forward " \t") 
	(if (file-directory-p (buffer-substring (dot) 
						(1- last-input-end))) 
	    (cd (buffer-substring (dot) (1- last-input-end))))))) 
  (let ((process (get-buffer-process (current-buffer)))) 
    (send-region process last-input-start last-input-end) 
    (set-marker (process-mark process) (dot)))) 
 
(defun delete-char-or-send-eof (arg killp) 
  "At end of buffer, send eof to subshell.  Otherwise delete character." 
  (interactive "p\nP") 
  (if (and (eobp) (not killp)) 
      (process-send-eof) 
    (delete-char arg killp))) 
 
(defun kill-output-from-shell () 
  "Kill all output from shell since last input." 
  (interactive) 
  (goto-char (dot-max)) 
  (kill-region last-input-end (dot)) 
  (insert "> output flushed ***\n")) 
 
(defun show-output-from-shell () 
  "Display start of this batch of shell output at top of window." 
  (interactive) 
  (set-window-start (selected-window) last-input-end)) 
 
(defun copy-last-shell-input () 
  "Copy previous shell input, sans newline, and insert before dot." 
  (interactive) 
  (insert (buffer-substring last-input-end last-input-start)) 
  (delete-char -1)) 
 
(defun interrupt-shell-subjob () 
  "Interrupt this shell's current subjob." 
  (interactive) 
  (interrupt-process nil t)) 
 
(defun kill-shell-subjob () 
  "Send kill signal to this shell's current subjob." 
  (interactive) 
  (kill-process nil t)) 
 
(defun quit-shell-subjob () 
  "Send quit signal to this shell's current subjob." 
  (interactive) 
  (quit-process nil t)) 
 
(defun stop-shell-subjob () 
  "Stop this shell's current subjob." 
  (interactive) 
  (stop-process nil t)) 
 
(defun kill-shell-input () 
  "Kill all text since last stuff output by the shell or its subjobs." 
  (interactive) 
  (kill-region (process-mark (get-buffer-process (current-buffer))) 
	       (dot))) 

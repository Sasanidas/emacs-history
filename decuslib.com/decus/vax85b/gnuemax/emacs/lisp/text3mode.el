;; Text mode, and its ideosyncratic commands. 
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
(defvar text-mode-syntax-table nil 
  "Syntax table used while in text mode.") 
 
(defvar text-mode-abbrev-table nil 
  "Abbrev table used while in text mode.") 
 
(if (null text-mode-syntax-table) 
    (let ((st (syntax-table))) 
      (unwind-protect 
       (progn 
	(setq text-mode-syntax-table (make-syntax-table)) 
	(set-syntax-table text-mode-syntax-table) 
	(modify-syntax-entry ?\" "    ") 
	(modify-syntax-entry ?\\ "    ") 
	(modify-syntax-entry ?[ "(]  ") 
	(modify-syntax-entry ?] ")[  ") 
	(modify-syntax-entry ?{ "(}  ") 
	(modify-syntax-entry ?} "){  ") 
	(modify-syntax-entry ?' "w   ")) 
       (set-syntax-table st)))) 
 
(defvar text-mode-map nil) 
 
(if (not text-mode-map) 
    (progn 
      (setq text-mode-map (make-sparse-keymap)) 
      (define-key text-mode-map "\t" 'tab-to-tab-stop) 
      (define-key text-mode-map "\es" 'center-line) 
      (define-key text-mode-map "\eS" 'center-paragraph))) 
 
(defun text-mode () 
  "Major mode for editing text intended for humans to read. 
Tab is tab-to-tab-stop.  (Set tab stops with edit-tab-stops.) 
Meta-s is center-line.  Meta-S is center-paragraph." 
  (interactive) 
  (kill-all-local-variables) 
  (use-local-map text-mode-map) 
  (setq mode-name "Text") 
  (setq major-mode 'text-mode) 
  (define-abbrev-table 'text-mode-abbrev-table ()) 
  (setq local-abbrev-table text-mode-abbrev-table) 
  (set-syntax-table text-mode-syntax-table) 
  (and (boundp 'text-mode-hook) 
       text-mode-hook 
       (funcall text-mode-hook))) 
 
(defvar indented-text-mode-map nil) 
 
(defun indented-text-mode () 
  "Major mode for editing indented text intended for humans to read. 
Tab runs indent-relative, indent under previous line. 
Meta-s is center-line.  Meta-S is center-paragraph." 
  (interactive) 
  (text-mode) 
  (if (not indented-text-mode-map) 
      (progn 
	(setq indented-text-mode-map (make-keymap)) 
	(define-key indented-text-mode-map "\t" 'indent-relative) 
	(define-key indented-text-mode-map "\es" 'center-line) 
	(define-key indented-text-mode-map "\eS" 'center-paragraph))) 
  (make-local-variable 'indent-line-function) 
  (setq indent-line-function 'indent-relative) 
  (use-local-map indented-text-mode-map) 
  (setq mode-name "Indented Text") 
  (setq major-mode 'indented-text-mode)) 
 
(defun center-paragraph () 
  "Center each line in the paragraph at or after dot. 
See center-line for more info." 
  (interactive) 
  (save-excursion 
    (forward-paragraph) 
    (or (bolp) (newline 1)) 
    (let ((end (dot))) 
      (backward-paragraph) 
      (center-region (dot) end)))) 
 
(defun center-region (from to) 
  "Center each line starting in the region. 
See center-line for more info." 
  (interactive "r") 
  (if (> from to) 
      (let ((tem to)) 
	(setq to from from tem))) 
  (save-excursion 
    (goto-char from) 
    (while (< (dot) to) 
      (center-line) 
      (forward-line 1)))) 
 
(defun center-line () 
  "Center the line dot is on. 
This means adjusting its indentation to match 
the distance between the end of the text and fill-column." 
  (interactive) 
  (save-excursion 
    (let (line-length) 
      (beginning-of-line) 
      (delete-horizontal-space) 
      (end-of-line) 
      (delete-horizontal-space) 
      (setq line-length (current-column)) 
      (beginning-of-line) 
      (indent-to  
	(+ left-margin  
	   (/ (- fill-column left-margin line-length) 2)))))) 

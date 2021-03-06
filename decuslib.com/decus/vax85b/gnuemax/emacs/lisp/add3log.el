;; Change log maintenance commands for Emacs 
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
(defun add-change-log-entry () 
  "Find change log file and add an entry for today." 
  (interactive) 
  (find-file (expand-file-name 
	      (read-file-name "Log file (default ChangeLog): " 
			      nil "ChangeLog"))) 
  (setq mode-string "Change Log") 
  (goto-char (dot-min)) 
  (if (looking-at (substring (current-time-string) 0 10)) 
      (progn  
       (forward-line 1) 
       (while (looking-at "\\W") 
	 (forward-line 1)) 
       (forward-line -1) 
       (delete-blank-lines) 
       (if (looking-at "\n") 
	   (delete-char 1) 
	 (forward-line 1)) 
       (open-line 2) 
       (indent-to left-margin)) 
    (insert (current-time-string) 
	    "  " (user-full-name) 
	    "  (" (user-login-name) 
	    " at " (system-name) ")\n") 
    (setq left-margin 8) 
    (setq fill-column 74) 
    (indent-to left-margin) 
    (open-line 2))) 

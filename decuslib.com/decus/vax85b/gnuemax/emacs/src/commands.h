/* Definitions needed by most editing commands. 
   Copyright (C) 1985 Richard M. Stallman. 
 
This file is part of GNU Emacs. 
 
GNU Emacs is distributed in the hope that it will be useful, 
but without any warranty.  No author or distributor 
accepts responsibility to anyone for the consequences of using it 
or for whether it serves any particular purpose or works at all, 
unless he says so in writing. 
 
Everyone is granted permission to copy, modify and redistribute 
GNU Emacs, but only under the conditions described in the 
document "GNU Emacs copying permission notice".   An exact copy 
of the document is supposed to have been given to you along with 
GNU Emacs so that you can know how you may redistribute it all. 
It should be in a file named COPYING.  Among other things, the 
copyright notice and this notice must be preserved on all copies.  */ 
 
 
 
#define Ctl(c) ((c)&037) 
 
/* Define the names of keymaps, just so people can refer to them in calls to defkey */ 
 
struct Lisp_Vector *GlobalMap;	/* default global key bindings */ 
 
struct Lisp_Vector *ESCmap;		/* The keymap used for globally bound 
				   ESC-prefixed default commands */ 
 
struct Lisp_Vector *CtlXmap;		/* The keymap used for globally bound 
				   ^X-prefixed default commands */ 
 
Lisp_Object MinibufLocalMap;	/* The keymap used by the minibuf for local 
				   bindings when spaces are allowed in the 
				   minibuf */ 
 
Lisp_Object MinibufLocalNSMap;/* The keymap used by the minibuf for local 
				   bindings when spaces are not allowed in 
				   the minibuf */ 
 
 /* keymap used for minibuffers when doing completion */ 
Lisp_Object MinibufLocalCompletionMap; 
 
 /* keymap used for minibuffers when doing completion and require a match */ 
Lisp_Object MinibufLocalMustMatchMap; 
 
char LastKeyStruck;		/* The last key struck as a command */ 
 
int immediate_quit;		/* Nonzero means ^G should call error right away 
				   rather than notifying eval to do so later. */ 
 
/* Previous command symbol found here for comparison */ 
extern Lisp_Object last_command; 
 
Lisp_Object Vexecuting_macro; 
 
/* Nonzero if input is coming from the keyboard */ 
 
#define INTERACTIVE (NULL (Vexecuting_macro)) 
 
/* Set this nonzero to force redisplay to consider all windows. */ 
 
int Cant1WinOpt; 
 
/* Set this nonzero to force reconsideration of mode line. */ 
 
int RedoModes; 
 
/* Nonzero means control characters display as ^letter */ 
 
int CtlArrow; 

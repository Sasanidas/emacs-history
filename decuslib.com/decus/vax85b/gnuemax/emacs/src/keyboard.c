/* Keyboard input; editor command loop. 
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
#include "config.h" 
#include <stdio.h> 
#undef NULL 
#include "lisp.h" 
#include "macros.h" 
#include "window.h" 
#include "commands.h" 
#include "buffer.h" 
#include <sgtty.h> 
#include <signal.h> 
#include <setjmp.h> 
 
/* Following definition copied from eval.c */ 
 
struct backtrace 
  { 
    struct backtrace *next; 
    Lisp_Object *function; 
    Lisp_Object *args;	/* Points to vector of args. */ 
    int nargs;		/* length of vector */ 
	       /* if nargs is UNEVALLED, args points to slot holding list of unevalled args */ 
    char evalargs; 
  }; 
 
/* Non-nil disable property on a command means 
 do not execute it; call disabled-command-hook's value instead. */ 
Lisp_Object Qdisabled, Vdisabled_command_hook; 
 
int recent_keys_index;	/* Index for storing next element into recent_keys */ 
int total_keys;		/* Total number of elements stored into recent_keys */ 
char recent_keys[100];	/* Holds last 100 keystrokes */ 
 
extern struct backtrace *backtrace_list; 
 
static jmp_buf getcjmp;  /* for longjmp to where kbd input is being done. */ 
 
static int waiting; /* True while doing kbd input */ 
 
static int echoing;  /* True while inside EchoKeys */ 
 
static int quit_during_echo;  /* 1 if C-G typed while echoing was 1. */ 
 
static executing;   /* True while within execution of a command */ 
 
int immediate_quit;	/* Nonzero means C-G should cause immediate error-signal. */ 
 
int help_char;		/* Character to recognize as the help char.  */ 
 
Lisp_Object Vhelp_form;	/* Form to execute when help char is typed.  */ 
 
extern struct Lisp_Vector *CurrentGlobalMap; 
 
/* If not -1, a character to be read as the next command input */ 
 
int unread_command_char; 
 
/* Char to use as prefix when a meta character is typed in. 
 This is bound on entry to minibuffer in case Esc is changed there.  */ 
 
int meta_prefix_char; 
 
static auto_save_interval;	/* The number of keystrokes between 
				   auto-saves. */ 
static Keystrokes;		/* The number of keystrokes since the last 
				   auto-save. */ 
 
Lisp_Object last_command;	/* Previous command, represented by a Lisp object. 
				   Does not include prefix commands and arg setting commands */ 
 
Lisp_Object this_command;	/* If a command sets this, 
				   the value goes into previous-command for the next command. */ 
 
Lisp_Object Qtop_level;		/* Catch tag for throwing back to top level loop */ 
 
Lisp_Object Qself_insert_command; 
Lisp_Object Qforward_char; 
Lisp_Object Qbackward_char; 
 
/* read_key_sequence stores here the command definition of the 
 key sequence that it reads */ 
Lisp_Object read_key_sequence_cmd; 
 
/* Form to evaluate (if non-nil) when Emacs is started */ 
Lisp_Object Vtop_level; 
 
/* User-supplied string to translate input characters through */ 
Lisp_Object Vkeyboard_translate_table; 
 
FILE *dribble;			/* File in which we write all commands we read */ 
int dribble_count; 
 
/* Nonzero if input is available */ 
int input_pending; 
 
/* Nonzero if should obey 0200 bit in input chars as "Meta" */ 
int MetaFlag; 
 
/* Buffer for pre-read keyboard input */ 
unsigned char kbd_buffer [256]; 
 
/* Number of characters available in kbd_buffer.  */ 
int kbd_count; 
 
/* Pointer to next available character in kbd_buffer.  */ 
unsigned char *kbd_ptr; 
 
/* Address (if not 0) of word to zero out 
 if a SIGIO interrupt happens */ 
long *input_available_clear_word; 
 
static char KeyBuf[10];		/* Buffer for keys from get_char () */ 
static NextK;			/* Next index into KeyBuf */ 
static echo_keystrokes;		/* True iff we are to echo keystrokes */ 
static Echo1;			/* Stuff for final echo */ 
unsigned char *keys_prompt;	/* String to display in front of echoed keystrokes, or 0 */ 
 
#define	min(a,b)	((a)<(b)?(a):(b)) 
 
static char echobuf[100]; 
 
EchoThem (notfinal) 
     register notfinal; 
{ 
  char *p; 
  int i; 
  int arg; 
  Lisp_Object tem; 
 
  extern char *push_key_description (); 
 
  if (!(echo_keystrokes && (NextK || keys_prompt))) 
    return; 
 
  echoing = 1; 
  p = echobuf; 
  if (keys_prompt) 
    { 
      strcpy (p, keys_prompt); 
      p += strlen (p); 
    } 
  for (i = 0; i < NextK; i++) 
    { 
      p = push_key_description (KeyBuf[i], p); 
      *p++ = ' '; 
    } 
  if (notfinal && NextK) 
    p[-1] = '-'; 
  *p = 0; 
  minibuf_message = echobuf; 
 
  if (notfinal) 
    Echo1++;		/* set echoed-flag */ 
  if (notfinal >= 0) 
    DoDsp (0); 
 
  echoing = 0; 
 
  if (quit_during_echo) 
    { 
      quit_during_echo = 0; 
      sigsetmask (0); 
      input_pending = 0; 
      unread_command_char = -1; 
      _longjmp (getcjmp, 1); 
    } 
} 
 
Lisp_Object recursive_edit_unwind (), command_loop (); 
 
DEFUN ("recursive-edit", Frecursive_edit, Srecursive_edit, 0, 0, "", 
  "Invoke the editor command loop recursively.\n\ 
Do (throw 'exit nil) within the command loop to make this function return,\n\ 
or (throw 'exit t) to make this function signal an error.\n\ 
This function is called by the editor initialization\n\ 
to begin editing.") 
  () 
{ 
  int count = specpdl_ptr - specpdl; 
  Lisp_Object val; 
 
  RecurseDepth++; 
  RedoModes++; 
 
  record_unwind_protect (recursive_edit_unwind, 
			 (RecurseDepth && 
			  bf_cur != XBUFFER (XWINDOW (selected_window)->buffer)) 
			 ? Fcurrent_buffer () 
			 : Qnil); 
 
  if (RecurseDepth) 
    { 
      val = internal_catch (Qexit, command_loop, Qnil); 
      if (!NULL (val)) 
	Fsignal (Qquit, Qnil); 
    } 
  else 
    while (1) 
      internal_catch (Qtop_level, command_loop, Qnil); 
 
  unbind_to (count); 
  return Qnil; 
} 
 
Lisp_Object 
recursive_edit_unwind (buffer) 
     Lisp_Object buffer; 
{ 
  if (!NULL (buffer)) 
    Fset_buffer (buffer); 
  RecurseDepth--; 
  RedoModes++; 
  executing = 1; 
} 
 
DEFUN ("top-level", Ftop_level, Stop_level, 0, 0, "", 
  "Exit all recursive editing levels.") 
  () 
{ 
  Fthrow (Qtop_level, Qnil); 
} 
 
DEFUN ("exit-recursive-edit", Fexit_recursive_edit, Sexit_recursive_edit, 0, 0, "", 
  "Exit from the innermost recursive edit or minibuffer.") 
  () 
{ 
  if (RecurseDepth) 
    Fthrow (Qexit, Qnil); 
  error ("No recursive edit is in progress"); 
} 
 
DEFUN ("abort-recursive-edit", Fabort_recursive_edit, Sabort_recursive_edit, 0, 0, "", 
  "Abort the command that requested this recursive edit or minibuffer input.") 
  () 
{ 
  if (RecurseDepth) 
    Fthrow (Qexit, Qt); 
  error ("No recursive edit is in progress"); 
} 
 
Lisp_Object command_loop_1 (), command_loop_2 (); 
Lisp_Object cmd_error (); 
Lisp_Object Fcommand_execute (); 
Lisp_Object top_level_1 (); 
 
/* Entry to editor command loop */ 
 
Lisp_Object 
command_loop () 
{ 
  Lisp_Object tem; 
 
  /* On entry to the outer level, run the startup file */ 
  if (!RecurseDepth) 
    internal_condition_case (top_level_1, Qerror, cmd_error); 
 
  while (1) 
    { 
      tem = internal_condition_case (command_loop_1, Qerror, cmd_error); 
      if (NULL (tem)) 
	break;			/* If command_loop_1 returned */ 
    } 
} 
 
Lisp_Object 
top_level_1 () 
{ 
  if (!NULL (Vtop_level)) 
    Feval (Vtop_level); 
  else if (!NULL (Vpurify_flag)) 
    message ("Bare impure Emacs (standard Lisp code not loaded)"); 
  else 
    message ("Bare Emacs (standard Lisp code not loaded)"); 
  return Qnil; 
} 
 
Lisp_Object 
cmd_error (data) 
     Lisp_Object data; 
{ 
  Lisp_Object tem, tail, errname; 
  int i; 
 
  Fdiscard_input (); 
  Ding (); 
 
  Vquit_flag = Qnil; 
  Vinhibit_quit = Qt; 
  Vstandard_input = Qt; 
  Vstandard_output = Qt; 
  minibuf_message = 0; 
 
  errname = Fcar (data); 
 
  if (EQ (errname, Qerror)) 
    { 
      if (!LISTP (data)) data = Qnil; 
      data = Fcdr (data); 
      if (!LISTP (data)) data = Qnil; 
      tem = Fcar (data); 
    } 
  else 
    tem = Fget (errname, Qerror_message); 
 
  /* Print an error message including the data items. 
     This is done by printing it into a scratch buffer 
     and then making a copy of the text in the buffer. */ 
   
  if (!LISTP (data)) data = Qnil; 
  tail = Fcdr (data); 
 
  /* For file-error, make error message by concatenating 
     all the data items.  They are all strings.  */ 
  if (EQ (errname, Qfile_error)) 
    tem = XCONS (tail)->car, tail = XCONS (tail)->cdr; 
 
  if (XTYPE (tem) == Lisp_String) 
    Fprinc (tem, Qt); 
  else 
    write_string_1 ("peculiar error", -1, Qt); 
 
  for (i = 0; LISTP (tail); tail = Fcdr (tail), i++) 
    { 
      write_string_1 (i ? ", " : ": ", 2, Qt); 
      if (EQ (errname, Qfile_error)) 
	Fprinc (Fcar (tail), Qt); 
      else 
	Fprin1 (Fcar (tail), Qt); 
    } 
 
  Vquit_flag = Qnil; 
  Vinhibit_quit = Qnil; 
 
  if (MinibufDepth) 
    { 
      Fsit_for (make_number (3)); 
      minibuf_message = 0; 
    } 
 
  return Qt; 
} 
 
void 
debugger (sig, data) 
     Lisp_Object sig, data; 
{ 
  Fterpri (Qnil); 
  Fprinc (Fget (sig, Qerror_message), Qnil); 
  putchar (' '); 
  Fprin1 (data, Qnil); 
  Fterpri (Qnil); 
  abort (); 
} 
 
/* This is the actual command reading loop, 
 sans error-handling encapsulation */ 
 
Lisp_Object 
command_loop_1 () 
{ 
  Lisp_Object cmd; 
  int lose; 
  int nonundocount; 
  char keybuf[30]; 
  int i; 
 
  Vprefix_arg = Qnil; 
  executing = 0; 
  waiting = 0; 
  Echo1 = 0; 
  NextK = 0; 
  Vstandard_input = Qt; 
  Vstandard_output = Qt; 
  last_command = Qt; 
  nonundocount = 0; 
   
  while (1) 
    { 
      if (defining_kbd_macro) 
	finalize_kbd_macro_chars (); 
 
      if (XBUFFER (XWINDOW (selected_window)->buffer) != bf_cur) 
	SetBfp (XBUFFER (XWINDOW (selected_window)->buffer)); 
 
      i = read_key_sequence (keybuf, sizeof keybuf, 0); 
      if (!i) 
	return Qnil; 
      LastKeyStruck = keybuf[i - 1]; 
       
      cmd = read_key_sequence_cmd; 
      if (NULL (cmd)) 
	{ 
	  Ding (); 
	  defining_kbd_macro = 0; 
	  RedoModes++; 
	  Vprefix_arg = Qnil; 
	} 
      else 
	{ 
	  executing = 1; 
	  this_command = cmd; 
	  if (NULL (Vprefix_arg)) 
	    { 
	      if (EQ (cmd, Qforward_char) && dot <= NumCharacters) 
		{ 
		  lose = CharAt (dot); 
		  SetDot (dot + 1); 
		  if (lose >= ' ' && lose < 0177 
		      && (XFASTINT (XWINDOW (selected_window)->last_modified) 
			  >= bf_modified) 
		      && (XFASTINT (XWINDOW (selected_window)->last_dot) 
			  == dot) 
		      && !Cant1WinOpt && NULL (Vexecuting_macro)) 
		    direct_output_forward_char (1); 
		  goto directly_done; 
		} 
	      else if (EQ (cmd, Qbackward_char) && dot > FirstCharacter) 
		{ 
		  SetDot (dot - 1); 
		  lose = CharAt (dot); 
		  if (lose >= ' ' && lose < 0177 
		      && (XFASTINT (XWINDOW (selected_window)->last_modified) 
			  >= bf_modified) 
		      && (XFASTINT (XWINDOW (selected_window)->last_dot) 
			  == dot) 
		      && !Cant1WinOpt && NULL (Vexecuting_macro)) 
		    direct_output_forward_char (-1); 
		  goto directly_done; 
		} 
	      else if (EQ (cmd, Qself_insert_command)) 
		{ 
		  if (NULL (Vexecuting_macro) && 
		      !EQ (minibuf_window, selected_window)) 
		    { 
		      if (!nonundocount || nonundocount >= 20) 
			{ 
			  Fundo_boundary (); 
			  nonundocount = 0; 
			} 
		      nonundocount++; 
		    } 
		  lose = (XFASTINT (XWINDOW (selected_window)->last_modified) 
			  < bf_modified) 
		    || (XFASTINT (XWINDOW (selected_window)->last_dot) 
			  != dot) 
		    || bf_modified <= bf_cur->save_modified 
		    || Cant1WinOpt || !NULL (Vexecuting_macro); 
		  if (SelfInsert (LastKeyStruck)) 
		    { 
		      lose = 1; 
		      nonundocount = 0; 
		    } 
		  if (!lose 
		      && (dot == NumCharacters + 1 || CharAt (dot) == '\n') 
		      && LastKeyStruck >= ' ' 
		      && LastKeyStruck < 0177) 
		    direct_output_for_insert (LastKeyStruck); 
		  goto directly_done; 
		} 
	    } 
 
	  /* Here for a command that isn't executed directly */ 
 
	  nonundocount = 0; 
	  if (NULL (Vprefix_arg) && NULL (Vexecuting_macro) && 
	      !EQ (minibuf_window, selected_window)) 
	    Fundo_boundary (); 
	  Fcommand_execute (cmd, Qnil); 
 
	directly_done: 
	  if (NULL (Vexecuting_macro)) 
	    executing = 0; 
	} 
 
      if (NULL (Vprefix_arg)) 
	{ 
	  last_command = this_command; 
	  NextK = 0; 
	  Echo1 = 0; 
	} 
    } 
} 
 
/* Input of single characters from keyboard */ 
 
Lisp_Object print_help (); 
 
int echo_flag; 
int echo_now; 
 
/* Alarm interrupt calls this and requests echoing at earliest safe time. */ 
request_echo () 
{ 
  if (echo_now) 
    EchoThem (1); 
  else 
    echo_flag = 1; 
} 
 
/* read a character from the keyboard; call the redisplay if needed */ 
/* commandflag nonzero means auto-saving may be considered */ 
get_char (commandflag) 
     int commandflag; 
{ 
  register int c; 
  register alarmtime = echo_keystrokes; 
  Lisp_Object tem; 
  extern request_echo (); 
 
  if ((c = unread_command_char) >= 0) 
    { 
      unread_command_char = -1; 
      goto reread; 
    } 
 
  if (!NULL (Vexecuting_macro)) 
    { 
      if (XTYPE (Vexecuting_macro) != Lisp_String 
	  || XSTRING (Vexecuting_macro)->size <= executing_macro_index) 
	return -1; 
      QUIT; 
      c = XSTRING (Vexecuting_macro)->data[executing_macro_index++]; 
      goto reread; 
    } 
 
  if (!input_pending && !detect_input_pending ()) 
    { 
      DoDsp (0); 
      if (auto_save_interval > 0 && commandflag 
	  && Keystrokes > auto_save_interval 
	  && Keystrokes > 20) 
	{ 
	  Fdo_auto_save (Qnil); 
	  Keystrokes = 0; 
	} 
    } 
 
  Keystrokes++; 
 
  if (_setjmp (getcjmp)) 
    { 
      c = Ctl('g'); 
      unread_command_char = -1; 
      waiting = 0; 
      minibuf_message = 0; 
 
      goto non_reread; 
    } 
 
  /* If echoing already, we will echo without delay, so need no interrupt */ 
  /* If no keys accumulated yet, need no interrupt. */ 
  /* If minibuffer active, do not echo keystrokes. */ 
  if (Echo1 || !NextK || MinibufDepth) 
    alarmtime = 0; 
    /* Message turns off echoing unless more keystrokes turn it on again. */ 
  if (minibuf_message && minibuf_message != echobuf) 
    alarmtime = 0, Echo1 = 0, NextK = 0; 
 
  if (Echo1)		/* If already echoing, put a dash at the end now */ 
    EchoThem (1); 
 
  /* Else start echoing if user waits more than `alarmtime' seconds. */ 
  if (alarmtime > 0) 
    { 
      /* This interrupt either calls EchoThem right away 
	 or sets echo_flag, which causes EchoThem to be called 
	 by set_waiting_for_input's next invocation.  */ 
      signal (SIGALRM, request_echo); 
      echo_flag = 0; 
      echo_now = 0; 
      alarm ((unsigned) alarmtime); 
    } 
 
  set_waiting_for_input (0); 
 
  c = kbd_buffer_get_char (); 
 
  clear_waiting_for_input (); 
 
  /* Cancel alarm if it was set and has not already gone off. */ 
  if (alarmtime > 0) alarm (0); 
 
  minibuf_message = 0; 
 
  if (c < 0) return -1; 
 
 non_reread: 
  c &= MetaFlag ? 0377 : 0177; 
 
  if (XTYPE (Vkeyboard_translate_table) == Lisp_String 
      && XSTRING (Vkeyboard_translate_table)->size > c) 
    c = XSTRING (Vkeyboard_translate_table)->data[c]; 
 
  total_keys++; 
  recent_keys[recent_keys_index] = c; 
  recent_keys_index = (recent_keys_index + 1) % sizeof recent_keys; 
 
  store_kbd_macro_char (c); 
  if (NextK >= sizeof KeyBuf) 
    NextK = 0; 
  KeyBuf[NextK++] = c; 
 
  /* If already echoing, echo right away. */ 
  if (Echo1) 
    EchoThem (0); 
 
  if (dribble) 
    { 
      putc (c, dribble); 
      if (!--dribble_count) 
	fflush (dribble), dribble_count = 20; 
    } 
 
 reread: 
  /* Process the help character specially if enabled */ 
  if (c == help_char && !NULL (Vhelp_form)) 
    { 
      tem = Feval (Vhelp_form); 
      if (XTYPE (tem) == Lisp_String) 
	internal_with_output_to_temp_buffer ("*Help*", print_help, tem); 
 
      NextK = 0; 
      c = get_char (0); 
      /* Remove the help from the screen */ 
      if (remove_help_window) 
	Fdelete_windows_on (Fget_buffer (build_string ("*Help*"))); 
    } 
 
  return c; 
} 
 
Lisp_Object 
print_help (object) 
     Lisp_Object object; 
{ 
  Fprinc (object, Qnil); 
  return Qnil; 
} 
 
/* Low level keyboard input. 
 Read characters into kbd_buffer 
 from which they are obtained by kbd_buffer_get_char.  */ 
 
kbd_buffer_get_char () 
{ 
  register int c; 
  int nread; 
 
  if (!kbd_count) 
    { 
#ifdef subprocesses 
      wait_reading_process_input (0, -1, 1); 
#else 
#ifdef INTERRUPT_INPUT 
      while (!kbd_count) pause (); 
#endif 
#endif 
#ifndef INTERRUPT_INPUT 
      ioctl (0, FIONREAD, &nread); 
      if (nread > sizeof kbd_buffer) 
	nread = sizeof kbd_buffer; 
      kbd_count = read (0, kbd_buffer, nread); 
      kbd_ptr = kbd_buffer; 
#endif 
      if (!kbd_count) 
	{ 
	  input_pending = 0; 
	  return -1; 
	} 
    } 
  input_pending = --kbd_count > 0; 
  c = *kbd_ptr;			/* *kbd_ptr++ would have a timing error. */ 
  kbd_ptr++;			/* See kbd_buffer_store_char. */ 
  return c; 
} 
 
#ifdef INTERRUPT_INPUT 
 
/* Set this for debugging, to have a way to get out */ 
static int stop_character; 
 
/* Store a character obtained at interrupt level into kbd_buffer, fifo */ 
kbd_buffer_store_char (c) 
     int c; 
{ 
  int temp; 
 
  if (!MetaFlag) 
    c &= 0177; 
  else 
    c &= 0377; 
 
  if (c == 07) 
    { 
      interrupt_signal (); 
      return; 
    } 
 
  if (c && c == stop_character) 
    { 
      kill (getpid (), SIGTSTP); 
      return; 
    } 
 
  if (kbd_ptr != kbd_buffer) 
    { 
      bcopy (kbd_ptr, kbd_buffer, kbd_count); 
      kbd_ptr = kbd_buffer; 
    } 
 
  if (kbd_count < sizeof kbd_buffer) 
    { 
      kbd_buffer[kbd_count++] = c; 
    } 
} 
 
input_available_signal () 
{ 
  unsigned char buf[64]; 
  int nread; 
  register int i; 
 
  if (input_available_clear_word) 
    *input_available_clear_word = 0; 
 
  while (1) 
    { 
      ioctl (0, FIONREAD, &nread); 
      if (!nread) 
	break; 
 
      if (nread > sizeof buf) 
	nread = sizeof buf; 
 
      nread = read (0, buf, nread); 
 
      for (i = 0; i < nread; i++) 
	kbd_buffer_store_char (buf[i]); 
    } 
} 
#endif INTERRUPT_INPUT 
 
/* Read a sequence of keys that ends with a non prefix character, 
 and store them in keybuf, a buffer of size bufsize. 
 Prompt with `prompt'.  Echo starting immediately unless `prompt' is 0. 
 Return the length of the key sequence stored. 
*/ 
 
int 
read_key_sequence (keybuf, bufsize, prompt) 
     char *keybuf; 
     unsigned char *prompt; 
{ 
  register int i; 
  Lisp_Object nextlocal, nextglobal; 
  register int c, nextc; 
  Lisp_Object local, global, local1, global1; 
 
  keys_prompt = prompt; 
 
  if (prompt) 
    NextK = 0; 
  if (prompt && INTERACTIVE) 
    EchoThem (1); 
 
  nextlocal = bf_cur->keymap; 
  XSETTYPE (nextglobal, Lisp_Vector); 
  XSETVECTOR (nextglobal, CurrentGlobalMap); 
 
  i = 0; 
  nextc = -1; 
  while (!NULL (nextlocal) || !NULL (nextglobal)) 
    { 
      if (i == bufsize) 
	error ("key sequence too long"); 
 
      if (nextc >= 0) 
	{ 
	  c = nextc; 
	  nextc = -1; 
	} 
      else 
	c = get_char (!prompt); 
 
      if (c < 0) 
	return 0; 
      if (c >= 0200) 
	{ 
	  nextc = c & 0177; 
	  c = meta_prefix_char; 
	} 
 
      keybuf[i++] = c; 
 
      global = !NULL (nextglobal) 
	? get_keyelt (access_keymap (nextglobal, c)) 
	  : Qnil; 
 
      local = !NULL (nextlocal) 
	? get_keyelt (access_keymap (nextlocal, c)) 
	  : Qnil; 
 
      nextlocal = Qnil; 
      nextglobal = Qnil; 
 
      read_key_sequence_cmd = !NULL (local) ? local : global; 
 
      /* trace symbols to their function definitions */ 
 
      while (XTYPE (global) == Lisp_Symbol && !NULL (global) 
	     && !EQ (global, Qunbound)) 
	global = XSYMBOL (global)->function; 
      while (XTYPE (local) == Lisp_Symbol && !NULL (local) 
	     && !EQ (local, Qunbound)) 
	local = XSYMBOL (local)->function; 
 
      /* Are the definitions prefix characters? */ 
 
      if (XTYPE (local) == Lisp_Vector || 
	  (LISTP (local) && EQ (XCONS (local)->car, Qkeymap)) 
	  || 
	  /* If nextc is set, we are processing a prefix char 
	     that represents a meta-bit. 
	     Let a global prefix definition override a local non-prefix. 
	     This is for minibuffers that redefine Escape for completion. 
	     A real Escape gets completion, but Meta bits get ESC-prefix.  */ 
	  ((NULL (local) || nextc >= 0) 
	   && (XTYPE (global) == Lisp_Vector || 
	       (LISTP (global) && EQ (XCONS (global)->car, Qkeymap))))) 
	{ 
	  if (XTYPE (local) == Lisp_Vector || 
	      (LISTP (local) && EQ (XCONS (local)->car, Qkeymap))) 
	    nextlocal = local; 
	  else 
	    nextlocal = Qnil; 
 
	  if (XTYPE (global) == Lisp_Vector || 
	      (LISTP (global) && EQ (XCONS (global)->car, Qkeymap))) 
	    nextglobal = global; 
	  else 
	    nextglobal = Qnil; 
	} 
    } 
 
  keys_prompt = 0; 
  return i; 
} 
 
DEFUN ("read-key-sequence", Fread_key_sequence, Sread_key_sequence, 1, 1, 0, 
  "Read a sequence of keystrokes and return as a string.\n\ 
The sequence is sufficient to specify a non-prefix command\n\ 
starting from the current local and global keymaps.\n\ 
One arg, PROMPT, a prompt string.") 
  (prompt) 
     Lisp_Object prompt; 
{ 
  char keybuf[30]; 
  register int i; 
 
  CHECK_STRING (prompt, 0); 
  i = read_key_sequence (keybuf, sizeof keybuf, XSTRING (prompt)->data); 
  return make_string (keybuf, i); 
} 
 
extern Lisp_Object Fexecute_kbd_macro (); 
 
DEFUN ("command-execute", Fcommand_execute, Scommand_execute, 1, 2, 0, 
 "Execute CMD as an editor command.\n\ 
CMD may be an APPLYable Lisp function,\n\ 
 or a string (which is executed as a keyboard macro).") 
     (cmd, record) 
     Lisp_Object cmd, record; 
{ 
  Lisp_Object final; 
  Lisp_Object tem; 
  Lisp_Object prefixarg; 
  struct backtrace backtrace; 
 
  prefixarg = Vprefix_arg, Vprefix_arg = Qnil; 
  Vcurrent_prefix_arg = prefixarg; 
 
  tem = Fget (cmd, Qdisabled); 
  if (!NULL (tem)) 
    return Fapply (Vdisabled_command_hook, Qnil); 
 
  while (1) 
    { 
      final = cmd; 
      while (XTYPE (final) == Lisp_Symbol) 
	{ 
	  if (EQ (Qunbound, XSYMBOL (final)->function)) 
	    Fsymbol_function (final);    /* Get an error! */ 
	  final = XSYMBOL (final)->function; 
	} 
 
      if (LISTP (final) && (tem = Fcar (final), EQ (tem, Qautoload))) 
	Fload (Fcar (Fcdr (final)), Qnil, Qnil); 
      else break; 
    } 
 
  if (LISTP (final) || XTYPE (final) == Lisp_Subr) 
    { 
      backtrace.next = backtrace_list; 
      backtrace_list = &backtrace; 
      backtrace.function = &Qcall_interactively; 
      backtrace.args = &cmd; 
      backtrace.nargs = 1; 
      backtrace.evalargs = 0; 
 
      tem = Fcall_interactively (cmd, record); 
 
      backtrace_list = backtrace.next; 
      return tem; 
    } 
  if (XTYPE (final) == Lisp_String) 
    { 
      return Fexecute_kbd_macro (final, prefixarg); 
    } 
  return Qnil; 
} 
 
detect_input_pending () 
{ 
  if (!input_pending) 
    { 
      if (kbd_count) 
	input_pending = kbd_count; 
#ifndef INTERRUPT_INPUT 
      else 
	ioctl (0, FIONREAD, &input_pending); 
#endif 
    } 
  return input_pending; 
} 
 
DEFUN ("input-pending-p", Finput_pending_p, Sinput_pending_p, 0, 0, 0, 
  "T if command input is currently available with no waiting.\n\ 
Actually, the value is NIL only if we can be sure that no input is available.") 
  () 
{ 
  if (unread_command_char >= 0) return Qt; 
 
  return detect_input_pending () ? Qt : Qnil; 
} 
 
DEFUN ("recent-keys", Frecent_keys, Srecent_keys, 0, 0, 0, 
  "Return string of last 100 chars read from terminal.") 
  () 
{ 
  Lisp_Object val; 
  if (total_keys < sizeof recent_keys) 
    return make_string (recent_keys, total_keys); 
 
  val = make_string (recent_keys, sizeof recent_keys); 
  bcopy (recent_keys + recent_keys_index, 
	 XSTRING (val)->data, 
	 sizeof recent_keys - recent_keys_index); 
  bcopy (recent_keys, 
	 XSTRING (val)->data + sizeof recent_keys - recent_keys_index, 
	 recent_keys_index); 
  return val; 
} 
 
DEFUN ("this-command-keys", Fthis_command_keys, Sthis_command_keys, 0, 0, 0, 
  "Return string of the keystrokes that invoked this command.") 
  () 
{ 
  return make_string (KeyBuf, NextK); 
} 
 
DEFSIMPLE ("last-key-struck", Flast_key_struck, Slast_key_struck, 
	   "Return the last character input from the command input stream, as a number.", 
	   Lisp_Int, XSETINT, LastKeyStruck) 
 
DEFSIMPLE ("recursion-depth", Frecursion_depth, Srecursion_depth, 
	   "Return the current depth in recursive edits.", 
	   Lisp_Int, XSETINT, RecurseDepth) 
 
DEFUN ("open-dribble-file", Fopen_dribble_file, Sopen_dribble_file, 0, 0, "", 
  "Start writing all keyboard characters to a file named dribble.") 
  () 
{ 
  dribble = fopen ("dribble", "w"); 
  dribble_count = 20; 
  return Qnil; 
} 
 
DEFUN ("discard-input", Fdiscard_input, Sdiscard_input, 0, 0, 0, 
  "Discard the contents of the terminal input buffer.\n\ 
Also flush any kbd macro definition in progress.") 
  () 
{ 
  struct sgttyb buf; 
 
  defining_kbd_macro = 0; 
  RedoModes++; 
 
  unread_command_char = -1; 
  ioctl (0, TIOCGETP, &buf); 
  ioctl (0, TIOCSETP, &buf); 
 
  kbd_count = 0; 
  input_pending = 0; 
 
  return Qnil; 
} 
 
DEFUN ("suspend-emacs", Fsuspend_emacs, Ssuspend_emacs, 0, 1, "", 
  "Stop Emacs and return to superior process.  You can resume.\n\ 
If optional arg STUFFSTRING is non-nil, its characters are stuffed\n\ 
to be read as terminal input by Emacs's superior shell.") 
  (stuffstring) 
     Lisp_Object stuffstring; 
{ 
  unsigned char *p; 
  int count; 
 
  if (!NULL (stuffstring)) 
    CHECK_STRING (stuffstring, 0); 
 
  RstDsp (); 
  if (!NULL (stuffstring)) 
    { 
      p = XSTRING (stuffstring)->data; 
      count = XSTRING (stuffstring)->size; 
      while (count-- > 0) 
	ioctl (0, TIOCSTI, p++); 
      ioctl (0, TIOCSTI, "\n"); 
    } 
  kill (0, SIGTSTP); 
  InitDsp (); 
  return Qnil; 
} 
 
set_waiting_for_input (word_to_clear) 
     long *word_to_clear; 
{ 
  input_available_clear_word = word_to_clear; 
 
  /* Tell interrupt_signal to throw back to get_char,  */ 
  waiting = 1; 
 
  /* If interrupt_signal was called before and buffered a C-g, 
     make it run again now, to avoid timing error.  */ 
  if (unread_command_char >= 0) 
    kill (getpid (), SIGINT); 
 
  /* Tell alarm signal to echo right away */ 
  echo_now = 1; 
 
  /* If alarm has gone off already, echo now.  */ 
  if (echo_flag) 
    { 
      EchoThem (1); 
      echo_flag = 0; 
    } 
} 
 
clear_waiting_for_input () 
{ 
  /* Tell interrupt_signal not to throw back to get_char,  */ 
  waiting = 0; 
  echo_now = 0; 
  input_available_clear_word = 0; 
} 
 
/* This routine is called at interrupt level in response to C-G. 
 If INTERRUPT_INPUT is non-nil, this is the handler for SIGINT. 
 Otherwise, it is called from kbd_buffer_store_char, in handling SIGIO. 
 
 If within get_char, above, it returns to get_char and makes it read C-G. 
 Otherwise it sets the Lisp variable  quit-flag  not-nil. 
 This causes  eval  to throw, when it gets a chance. 
 If  quit-flag  is already non-nil, it stops the job right away.  */ 
 
interrupt_signal () 
{ 
  Lisp_Object tem; 
  char c; 
 
  Echo1 = 0; 
 
  if (!executing) 
    { 
      unread_command_char = Ctl('g'); 
    } 
  else 
    { 
      if (!NULL (Vquit_flag)) 
	{ 
	  fflush (stdout); 
	  RstDsp (); 
	  sigsetmask (0); 
	  kill (0, SIGTSTP); 
	  printf ("Auto-save? (y or n) "); 
	  fflush (stdout); 
	  if (((c = getchar ()) & ~040) == 'Y') 
	    Fdo_auto_save (); 
	  while (c != '\n') c = getchar (); 
	  printf ("Dump core image? (y or n) "); 
	  fflush (stdout); 
	  if (((c = getchar ()) & ~040) == 'Y') 
	    abort (); 
	  while (c != '\n') c = getchar (); 
	  InitDsp (); 
	} 
      else 
	{ 
	  /* If executing a function that wants to be interrupted out of 
	     and the user has not deferred quitting by binding `inhibit-quit' 
	     then quit right away.  */ 
	  if (immediate_quit && NULL (Vinhibit_quit)) 
	    { 
	      sigsetmask (0); 
	      Fsignal (Qquit, Qnil); 
	    } 
	  /* Else request quit when it's safe */ 
	  Vquit_flag = Qt; 
	} 
    } 
 
  if (waiting) 
    { 
      if (echoing) 
	quit_during_echo = 1; 
      else 
	{ 
	  sigsetmask (0); 
	  input_pending = 0; 
	  unread_command_char = -1; 
	  _longjmp (getcjmp, 1); 
	} 
    } 
} 
 
init_keyboard () 
{ 
  RecurseDepth = -1;	/* Correct, before outermost invocation of editor loop */ 
  keys_prompt = 0; 
  immediate_quit = 0; 
  unread_command_char = -1; 
  recent_keys_index = 0; 
  total_keys = 0; 
  kbd_count = 0; 
  input_pending = 0; 
  signal (SIGINT, interrupt_signal); 
#ifdef INTERRUPT_INPUT 
  signal (SIGIO, input_available_signal); 
#endif INTERRUPT_INPUT 
  sigsetmask (0); 
  dribble = 0; 
} 
 
syms_of_keyboard () 
{ 
  Qself_insert_command = intern ("self-insert-command"); 
  staticpro (&Qself_insert_command); 
 
  Qforward_char = intern ("forward-char"); 
  staticpro (&Qforward_char); 
 
  Qbackward_char = intern ("backward-char"); 
  staticpro (&Qbackward_char); 
 
  Qtop_level = intern ("top-level"); 
  staticpro (&Qtop_level); 
 
  Qdisabled = intern ("disabled"); 
  staticpro (&Qdisabled); 
 
  defsubr (&Sread_key_sequence); 
  defsubr (&Srecursive_edit); 
  defsubr (&Sinput_pending_p); 
  defsubr (&Scommand_execute); 
  defsubr (&Srecent_keys); 
  defsubr (&Sthis_command_keys); 
  defsubr (&Ssuspend_emacs); 
  defsubr (&Sabort_recursive_edit); 
  defsubr (&Sexit_recursive_edit); 
  defsubr (&Slast_key_struck); 
  defsubr (&Srecursion_depth); 
  defsubr (&Stop_level); 
  defsubr (&Sdiscard_input); 
  defsubr (&Sopen_dribble_file); 
 
  DefLispVar ("disabled-command-hook", &Vdisabled_command_hook, 
    "Value is called instead of any command that is disabled\n\ 
\(has a non-nil  disabled  property)."); 
 
  DefBoolVar ("meta-flag", &MetaFlag, 
    "*Non-nil means treat 0200 bit in terminal input as Meta bit."); 
 
  DefIntVar ("last-key", &LastKeyStruck, 
    "Last terminal input character as an integer."); 
 
  DefIntVar ("unread-command-char", &unread_command_char, 
    "Character to be read as next input from command input stream, or -1 if none."); 
 
  DefIntVar ("meta-prefix-char", &meta_prefix_char, 
    "Meta-prefix character code.  Meta-foo as command input\n\ 
turns into this character followed by foo."); 
  meta_prefix_char = 033; 
 
  DefLispVar ("last-command", &last_command, 
    "The last command executed.  Normally a symbol with a function definition,\n\ 
but can be whatever was found in the keymap, or whatever that command left in this-command."); 
  last_command = Qnil; 
 
  DefLispVar ("this-command", &this_command, 
    "The command now being executed.\n\ 
The command can set this variable; whatever is put here\n\ 
will be in  last-command  during the following command."); 
  this_command = Qnil; 
 
  DefIntVar ("auto-save-interval", &auto_save_interval, 
    "*Number of keyboard input characters between auto-saves."); 
  auto_save_interval = 300; 
 
  DefIntVar ("echo-keystrokes", &echo_keystrokes, 
    "*Nonzero means echo unfinished commands after this many seconds of pause."); 
  echo_keystrokes = 1; 
 
  DefIntVar ("help-char", &help_char, 
    "Character to recognize as meaning Help.\n\ 
When it is read, do (eval help-form), and display result if it's a string.\n\ 
If help-form's value is nil, this char can be read normally."); 
  help_char = Ctl ('H'); 
 
  DefLispVar ("help-form", &Vhelp_form, 
    "Form to execute when character help-char is read.\n\ 
If the form returns a string, that string is displayed.\n\ 
If help-form is nil, the help char is not recognized."); 
  Vhelp_form = Qnil; 
 
  DefLispVar ("top-level", &Vtop_level, 
    "Form to evaluate when Emacs starts up.\n\ 
Useful to set before you dump a modified Emacs."); 
  Vtop_level = Qnil; 
 
  DefLispVar ("keyboard-translate-table", &Vkeyboard_translate_table, 
    "String used as translate table for keyboard input, or nil.\n\ 
Each character is looked up in this string and the contents used instead.\n\ 
If string is of length N, character codes N and up are untranslated.");  
  Vkeyboard_translate_table = Qnil; 
} 
 
keys_of_keyboard () 
{ 
  defkey (GlobalMap, Ctl ('Z'), "suspend-emacs"); 
  defkey (CtlXmap, Ctl ('Z'), "suspend-emacs"); 
  defkey (ESCmap, Ctl ('C'), "exit-recursive-edit"); 
  defkey (GlobalMap, Ctl ('C'), "exit-recursive-edit"); 
  defkey (GlobalMap, Ctl (']'), "abort-recursive-edit"); 
} 

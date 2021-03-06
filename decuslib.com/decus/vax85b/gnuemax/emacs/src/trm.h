/* terminal control module header file */ 
 
/*		Copyright (c) 1981,1980 James Gosling 
   Enhancements copyright (c) 1984 Fen Labalme and Chris Torek 
   Distributed by Fen Labalme, with permission from James Gosling, 
     under the following conditions: 
 
   Permission is granted to anyone to make or distribute 
   verbatim copies of Emacs 
   provided that the copyright notice and permission notice are preserved; 
   and provided that the recipient is not asked to waive or limit his right 
   to redistribute copies as permitted by this permission notice; 
   and provided that anyone possessing a machine-executable copy 
   is granted access to copy the source code, in machine-readable form, 
   in some reasonable manner.  Distribution of executable code without 
   the source code must be accompanied promenantly by this permission notice 
   and a statement informing the recipient of how to obtain the source code. 
 
   Permission is granted to distribute derived works or modified versions 
   of Emacs or portions of Emacs under the above conditions, with 
   the additional requirement that the entire derivative or modified work 
   must be covered by a permission notice identical to this one. 
 
   Anything distributed with and usable only in conjunction with something 
   derived from Emacs, whose useful purpose is conceptually 
   to extend or adapt or add capabilities to Emacs, is to be considered 
   as part of a modified version of Emacs under the requirements above. 
 
   This software is distributed in the hope that it will be useful, 
   but there is no warranty of any sort, and no contributor accepts 
   responsibility for the consequences of using this program 
   or for whether it serves any particular purpose. 
 
 In other words, you are welcome to use, share and improve this program. 
 You are forbidden to forbid anyone else to use, share and improve 
 what you give them.  Help stamp out software-hoarding!  */ 
 
int MetaFlag; 
 
struct TrmControl { 
    int     (*t_topos) ();	/* move the cursor to the indicated 
				   (row,column); (1,1) is the upper left */ 
    int     (*t_reset) ();	/* reset terminal (screen is in unkown state, 
				   convert it to a known one) */ 
    int     (*t_INSmode) ();	/* set or reset character insert mode */ 
    int     (*t_HLmode) ();	/* set or reset highlighting */ 
    int     (*t_inslines) ();	/* insert n lines */ 
    int     (*t_dellines) ();	/* delete n lines */ 
    int     (*t_blanks) ();	/* print n blanks */ 
    int     (*t_init) ();	/* initialize terminal settings */ 
    int     (*t_cleanup) ();	/* clean up terminal settings */ 
    int     (*t_wipeline) ();	/* erase to the end of the line */ 
    int     (*t_wipescreen) ();	/* erase the entire screen */ 
    int     (*t_delchars) ();	/* delete n characters */ 
    int     (*t_writechars) ();	/* write characters; either inserting or 
				   overwriting according to the current 
				   character insert mode. */ 
    int     (*t_window) ();	/* set the screen window so that IDline 
				   operations only affect the first n 
				   lines of the screen */ 
    int     (*t_flash) ();	/* Flash the screen -- not set if this 
				   terminal type won't support it. */ 
    int     (*t_UpdateBegin) ();/* Called by the redisplay driver just 
				   before it starts updating.  This is 
				   for terminals liek the SUN that need to 
				   do things like removing the cursor */ 
    int     (*t_UpdateEnd) ();	/* Called by the redisplay driver just 
				   after it's finished updating.  This is 
				   for terminals like the SUN that need to 
				   do things like redrawing the cursor */ 
    int     (*t_ReSize) ();	/* If non-null, this is called by the window 
				   manager before it begins to lay out and 
				   redisplay a new screen.  This is generally 
				   used by terminal drivers that need to 
				   respond to changing window sizes -- this 
				   procedure is where the size change 
				   calculations should be done */ 
/* Costs are expressed as number_affected*mf + ov. 
	Cost to insert one character: 1*ICmf+ICov 
	Cost to delete n characters:  n*DCmf+DCov 
   Line costs are different because they have a pad factor. 
	cost to insert n lines at line l: 
	    n*(ILnov+(length+1-l)*ILnpf)+ILov+(length+1-l)*ILpf 
	nov is the per-line cost, ov is the per-insert-op cost 
	npf is the per-line pad, pf is the per-insert-op pad. */ 
    double	t_ILnpf; 
    int		t_ILnov; 
    double	t_ILpf; 
    int		t_ILov; 
    double	t_DLnpf; 
    int		t_DLnov; 
    double	t_DLpf; 
    int		t_DLov; 
    double	t_ICmf;		/* insert character multiply factor */ 
    int		t_ICov;		/* insert character overhead */ 
    double	t_DCmf;		/* delete character multiply factor */ 
    int		t_DCov;		/* delete character overhead */ 
    int		t_length;		/* screen length */ 
    int		t_width;		/* screen width */ 
    int     t_needspaces;	/* set true iff the terminal needs to have 
				   real spaces in the middle of lines in 
				   order to have character insertion work -- 
				   this only matters on terminals that 
				   distinguish between real and imaginary 
				   blanks. */ 
    int	    t_modeline;		/* An integer which (may) affect the 
				   type of highlighting done on the mode line. 
				   A value of 0 specifies that no highlighting 
				   is desired, non-zero values specify other 
				   types of highlighting. */ 
    char    t_padc;		/* character to pad with */ 
}; 
 
#define MissingFeature 99999	/* IC and IL overheads should be set to this 
				   value if the corresponding feature is 
				   missing */ 
struct TrmControl tt;		/* terminal specific information for the 
				   current display */ 
 
int NoOperation (); 

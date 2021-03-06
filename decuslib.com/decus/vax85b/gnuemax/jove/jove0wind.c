Y
/* 
   Jonathan Payne at Lincoln-Sudbury Regional High School 5-25-83 
 
   jove_wind.c 
 
   This creates/deletes/divides/grows/shrinks windows.  */ 
 
#include "jove.h" 
#include "termcap.h" 
 
char	onlyone[] = "You only have one window!"; 
char	toosmall[] = "Resulting window would be too small"; 
 
Window	*curwind, 
	*fwind = 0; 
 
/* First line in a Window */ 
 
FLine(w) 
Window	*w; 
{ 
	Window	*wp = fwind; 
	int	lineno = -1; 
 
	do { 
		if (wp == w) 
			return lineno + 1; 
		lineno += wp->w_height; 
		wp = wp->w_next; 
	} while (wp != fwind); 
	complain("window?"); 
	/* NOTREACHED */ 
} 
 
initwinds(b) 
Buffer	*b; 
{ 
	Window	*wp = fwind; 
 
	do { 
		if (wp->w_bufp == b) { 
			SetTop(wp, (wp->w_line = b->b_dot)); 
			zero_wind(wp); 
		} 
		wp = wp->w_next; 
	} while (wp != fwind); 
} 
 
Window * 
getwind() 
{ 
	Window	*wp; 
	 
	wp = (Window *) emalloc(sizeof (Window)); 
	return wp; 
} 
 
/* Delete `wp' from the screen.  If it is the only window left 
   on the screen, then complain.  It gives its body 
   to the next window if there is one, otherwise the previous 
   window gets the body.  */ 
 
del_wind(wp) 
Window	*wp; 
{ 
	Window	*prev = wp->w_prev; 
 
	if (wp->w_next == wp) 
		complain(onlyone); 
 
	wp->w_prev->w_next = wp->w_next; 
	wp->w_next->w_prev = wp->w_prev; 
	 
	if (fwind == wp) { 
		fwind = wp->w_next; 
		fwind->w_height += wp->w_height; 
		if (curwind == wp) 
			SetWind(fwind); 
	} else { 
		prev->w_height += wp->w_height; 
		if (curwind == wp) 
			SetWind(prev); 
	} 
	free((char *) wp); 
} 
 
/* Divide the `wp'.  Complains if `wp' is too small to be split. 
   It returns the new window. */ 
 
Window * 
div_wind(wp) 
Window	*wp; 
{ 
	Window	*new; 
 
	if (wp->w_height < 4) 
		complain(toosmall); 
 
	new = getwind(); 
	new->w_offset = 0; 
	new->w_numlines = 0; 
	/* Reset the window bounds */ 
	new->w_height = (wp->w_height / 2); 
	wp->w_height -= new->w_height; 
 
	/* Set the lines such that w_line is the center in each Window */ 
	new->w_line = wp->w_line; 
	new->w_bufp = wp->w_bufp; 
	new->w_top = prev_line(new->w_line, HALF(new)); 
 
	/* Link the new window into the list */ 
	new->w_prev = wp; 
	new->w_next = wp->w_next; 
	new->w_next->w_prev = new; 
	wp->w_next = new; 
	return new; 
} 
 
/* Return one window previous to `wp'.  If at the first window 
   on screen, then go to the last window. */ 
 
Window * 
prev_wind(wp) 
Window	*wp; 
{ 
	return wp->w_prev; 
} 
 
/* Next window from `wp' */ 
 
Window * 
next_wind(wp) 
Window	*wp; 
{ 
	return wp->w_next; 
} 
 
/* Initialze the first window setting the bounds to the size of the 
   screen.  There is no buffer with this window.  See parse for the 
   setting of this window. */ 
 
winit() 
{ 
	curwind = fwind = getwind(); 
 
	curwind->w_line = curwind->w_top = (Line *) 0; 
	curwind->w_numlines = 0; 
	curwind->w_char = 0; 
	curwind->w_next = curwind->w_prev = fwind; 
	curwind->w_height = LI - 1; 
} 
 
/* Change window into the previous window.  curwind becomes the new 
 * window 
 */ 
 
PrevWindow() 
{ 
	Window	*new = prev_wind(curwind); 
 
	if (new == curwind) 
		complain(onlyone); 
	SetWind(new); 
} 
 
/* Make new the current Window */ 
 
SetWind(new) 
Window	*new; 
{ 
	curwind->w_line = curline; 
	curwind->w_char = curchar; 
	if (new == curwind) 
		return; 
	SetBuf(new->w_bufp); 
	if (!inlist(new->w_bufp->b_first, new->w_line)) { 
		new->w_line = curline; 
		new->w_char = curchar; 
	} 
	DotTo(new->w_line, new->w_char); 
	if (curchar > strlen(linebuf)) 
		new->w_char = curchar = strlen(linebuf); 
	curwind = new; 
} 
 
/* Delete the current window if it isn't the only one left */ 
 
DelCurWindow() 
{ 
	del_wind(curwind); 
} 
 
/* Return the number of windows being displayed right now */ 
 
numwindows() 
{ 
	Window	*wp = fwind; 
	int	num = 0; 
 
	do { 
		num++; 
		wp = wp->w_next; 
	} while (wp != fwind); 
	return num; 
} 
 
AdjWindows() 
{ 
	Window	*wp = fwind; 
	int	nwind = numwindows(); 
	int	avail = LI - 1; 
	int	targetSize = avail / nwind; 
	int	xtraLines = avail % nwind; 
 
	do { 
		wp->w_height = targetSize + ((xtraLines > 0) ? 1 : 0); 
		xtraLines --; 
		wp = wp->w_next; 
	} while (wp != fwind); 
} 
 
WindFind() 
{ 
	Buffer	*savebuf = curbuf, 
		*newb; 
	extern int 
		FindTag(), 
		BufSelect(), 
		FindFile(); 
	extern int	Interactive; 
	extern struct funct	*FindFunc(); 
 
	switch (waitfor(2)) { 
	case 't': 
	case 'T': 
		ExecFunc(FindFunc(FindTag), Interactive); 
		break; 
 
	case 'b': 
	case 'B': 
		ExecFunc(FindFunc(BufSelect), Interactive); 
		break; 
 
	case 'f': 
	case 'F': 
		ExecFunc(FindFunc(FindFile), Interactive); 
		break; 
 
	default: 
		complain("T: find-tag, F: find-file, B: select-buffer"); 
	} 
 
	newb = curbuf; 
	SetBuf(savebuf); 
	tiewind(curwind, curbuf); 
	/* Now it's as if we did nothing */ 
 
	if (numwindows() == 1) 
		SetWind(div_wind(curwind)); 
	else 
		SetWind(next_wind(curwind)); 
	SetBuf(newb); 
	tiewind(curwind, curbuf); 
} 
 
/* Go into one window mode by deleting all the other windows */ 
 
OneWindow() 
{ 
	while (curwind->w_next != curwind) 
		del_wind(curwind->w_next); 
} 
 
Window * 
windbp(bp) 
register struct buffer	*bp; 
{ 
 
	register Window	*wp = fwind; 
 
	if (bp == 0) 
		return 0; 
	do { 
		if (wp->w_bufp == bp) 
			return wp; 
		wp = wp->w_next; 
	} while (wp != fwind); 
	return 0; 
} 
 
/* Look for a window containing a buffer whose name is `name' */ 
 
Window * 
windlook(name) 
register char	*name; 
{ 
	return windbp(buf_exists(name)); 
} 
 
/* Change window into the next window.  curwind becomes the new 
 * window 
 */ 
 
NextWindow() 
{ 
	Window	*new = next_wind(curwind); 
 
	if (new == curwind) 
		complain(onlyone); 
	SetWind(new); 
} 
 
/* Scroll the next/prev Window */ 
 
PageNWind() 
{ 
	if (numwindows() == 1) 
		complain(onlyone); 
	NextWindow(); 
	NextPage(); 
	PrevWindow(); 
} 
 
PagePWind() 
{ 
	if (numwindows() == 1) 
		complain(onlyone); 
	PrevWindow(); 
	NextPage(); 
	NextWindow(); 
} 
 
UPageNWind() 
{ 
	if (numwindows() == 1) 
		complain(onlyone); 
	NextWindow(); 
	PrevPage(); 
	PrevWindow(); 
} 
 
UPagePWind() 
{ 
	if (numwindows() == 1) 
		complain(onlyone); 
	PrevWindow(); 
	PrevPage(); 
	NextWindow(); 
} 
 
 
Window * 
w_nam_typ(name, type) 
char	*name; 
{ 
	register Window *w; 
	register Buffer	*b; 
 
	b = buf_exists(name); 
	w = fwind; 
	if (b) do { 
		if (w->w_bufp == b) 
			return w; 
	} while ((w = w->w_next) != fwind); 
 
	w = fwind; 
	do { 
		if (w->w_bufp->b_type == type) 
			return w; 
	} while ((w = w->w_next) != fwind); 
 
	return 0; 
} 
 
/* Put a window with the buffer `name' in it.  Erase the buffer if 
   `clobber' is non-zero. */ 
 
pop_wind(name, clobber, btype) 
char	*name; 
{ 
	Window	*wp; 
	Buffer	*newb; 
 
	if (! (clobber&0x2)) { 
		if ((wp = w_nam_typ(name, btype)) == 0) { 
			if (numwindows() == 1) 
				SplitWind(); 
			else 
				PrevWindow(); 
		} else 
			SetWind(wp); 
	} 
	newb = do_select((Window *) 0, name); 
	if (clobber) 
		initlist(newb); 
	tiewind(curwind, newb); 
	if (btype != -1) 
		newb->b_type = btype; 
	SetBuf(newb); 
} 
 
GrowWindow() 
{ 
	WindSize(curwind, abs(exp)); 
} 
 
ShrWindow() 
{ 
	WindSize(curwind, -abs(exp)); 
} 
 
/* Change the size of the window by inc.  First arg is the window, 
   second is the increment. */ 
 
WindSize(w, inc) 
register Window	*w; 
{ 
	if (numwindows() == 1) 
		complain(onlyone); 
 
	if (inc < 0) {	/* Shrinking this Window */ 
		if (w->w_height + inc < 2) 
			complain(toosmall); 
		w->w_height += inc; 
		w->w_prev->w_height -= inc; 
	} else 
		WindSize(w->w_next, -inc); 
} 
 
/* Set the topline of the window, calculating its number in the buffer. 
   This is for numbering the lines only. */ 
 
SetTop(w, line) 
Window	*w; 
register Line	*line; 
{ 
	register Line	*lp = w->w_bufp->b_first; 
	register int	num = 0; 
 
	w->w_top = line; 
	if (w->w_numlines) 
		while (lp) { 
			num++; 
			if (line == lp) 
				break; 
			lp = lp->l_next; 
		} 
	w->w_topnum = num; 
} 
 
WNumLines() 
{ 
	zero_wind(curwind); 
	/* So the redisplay will know to update the screen even if it 
	   looks like there are no differences. */ 
 
	curwind->w_numlines = !curwind->w_numlines; 
	SetTop(curwind, curwind->w_top); 
} 
 
zero_wind(wp) 
register Window	*wp; 
{ 
	register int	i, 
			upper; 
 
	upper = FLine(wp); 
	for (i = 0; i < wp->w_height; i++) 
		oimage[upper + i].Line = (Line *) -1; 
} 
 
/* Return the line number that `line' occupies in `windes' */ 
 
in_window(windes, line) 
register Window	*windes; 
register Line	*line; 
{ 
	register int	i; 
	Line	*top = windes->w_top; 
 
	for (i = 0; top && i < windes->w_height - 1; i++, top = top->l_next) 
		if (top == line) 
			return FLine(windes) + i; 
	return -1; 
} 
 
SplitWind() 
{ 
	curwind = div_wind(curwind); 
} 
 
 

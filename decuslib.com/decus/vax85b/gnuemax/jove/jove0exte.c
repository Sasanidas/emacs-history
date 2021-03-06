/*R
   Jonathan Payne at Lincoln-Sudbury Regional High School 4-19-83 
   
   jove_extend.c 
 
   This contains the TENEX style command completion routine and 
   various random routines.  This is not well organized...use the 
   tags feature of jove to find your way around...  */ 
 
#include "jove.h" 
 
int	InJoverc = 0; 
 
extern int	getch(), 
		getchar(); 
 
BindAKey() 
{ 
	BindSomething(commands); 
} 
 
BindMac() 
{ 
	BindSomething(macros); 
} 
 
extern int	EscPrefix(), 
		CtlxPrefix(), 
		MiscPrefix(); 
 
data_obj ** 
IsPrefix(fp) 
data_obj	*fp; 
{ 
	int	(*func)(); 
 
	if (fp == 0 || fp->Type != FUNCTION) 
		return 0; 
	func = ((struct funct *) fp)->f_func; 
	if (func == EscPrefix) 
		return pref1map; 
	if (func == CtlxPrefix) 
		return pref2map; 
	if (func == MiscPrefix) 
		return miscmap; 
	return 0; 
} 
 
addgetc() 
{ 
	int	c; 
	char	strbuf[10]; 
 
	Asking = strlen(mesgbuf); 
	c = (*Getchar)(); 
	Asking = 0; 
	PPchar(c, strbuf); 
	add_mess(strbuf); 
	return c; 
} 
 
BindWMap(map, key, func) 
data_obj	**map, 
		*func; 
{ 
	data_obj	*fp = map[key], 
			**prefp; 
 
	if (prefp = IsPrefix(fp)) { 
		add_mess(" "); 
		BindWMap(prefp, addgetc(), func); 
	} else 
		map[key] = func; 
} 
 
BindSomething(funcs) 
data_obj	*funcs; 
{ 
	char	*prompt; 
	data_obj	*d; 
 
	prompt = FuncName(); 
	if ((d = findcom(funcs, prompt, NOTHING)) == 0) 
		return; 
	s_mess("%s%s ", prompt, d->Name); 
	BindWMap(mainmap, addgetc(), d); 
} 
 
UnBound() 
{ 
	Beep(); 
} 
 
/* Describe key */ 
 
DescWMap(map, key) 
data_obj	**map; 
{ 
	data_obj	*fp = map[key], 
			**prefp; 
 
	if (fp == 0) 
		add_mess(" is unbound"); 
	else if (prefp = IsPrefix(fp)) { 
		add_mess(" "); 
		DescWMap(prefp, addgetc()); 
	} else 
		add_mess(" is bound to %s", fp->Name); 
} 
 
KeyDesc() 
{ 
	message(FuncName()); 
	DescWMap(mainmap, addgetc()); 
} 
 
DescCom() 
{ 
	data_obj	*fp; 
	Window *savewp = curwind; 
 
	if ((fp = findcom(commands, FuncName(), NOTHING)) == 0) 
		return; 
	message(fp->Name); 
	ignore(UnixToBuf("Command Description", 1, 0, exp_p == 0, 
					FINDCOM, 
					"describe", 
					fp->Name, 0)); 
	message(NullStr); 
	SetWind(savewp); 
} 
 
DescBindings() 
{ 
	if (UseBuffers) 
		TellWBuffers("Bindings", 1); 
	else 
		TellWScreen(0); 
	DescMap(mainmap, NullStr); 
	if (UseBuffers) { 
		ToFirst(); 
		NotModified(); 
	} 
	StopTelling(); 
} 
 
PPchar(c, str) 
int	c; 
char	*str; 
{ 
	char	*cp = str; 
 
	if (c == '\033') 
		strcpy(cp, "ESC"); 
	else if (c < ' ') 
		ignore(sprintf(cp, "C-%c", c + '@')); 
	else if (c == '\177') 
		strcpy(cp, "^?"); 
	else 
		ignore(sprintf(cp, "%c", c)); 
} 
 
DescMap(map, pref) 
data_obj	**map; 
char	*pref; 
{ 
	int	c1, 
		c2 = 0, 
		numbetween; 
	char	keydescbuf[10], 
		c1str[10], 
		c2str[10]; 
	data_obj	**prefp; 
 
	for (c1 = 0; c1 < 0200 && c2 < 0200; c1 = c2 + 1) { 
		c2 = c1; 
		if (map[c1] == 0) 
			continue; 
		if (c1 < 0177 && map[c1] == map[c1 + 1]) { 
			c2 = c1 + 1; 
			while (map[c1] == map[c2]) 
				c2++; 
			c2--; 
		} else 
			c2 = c1; 
		numbetween = c2 - c1; 
		PPchar(c1, c1str); 
		PPchar(c2, c2str); 
		if (numbetween == 1) 
			ignore(sprintf(keydescbuf, "%s {%s,%s}", pref, c1str, c2str)); 
		else if (numbetween == 0) 
			ignore(sprintf(keydescbuf, "%s %s", pref, c1str)); 
		else 
			ignore(sprintf(keydescbuf, "%s [%s-%s]", pref, c1str, c2str)); 
		if (prefp = IsPrefix(map[c1])) 
			DescMap(prefp, keydescbuf); 
		else if (DoTell("%-14s%s", keydescbuf, map[c1]->Name) == ABORT) 
			break; 
	} 
} 
 
Beep() 
{ 
	extern int	errormsg; 
 
	message(NullStr); 
	rbell();		/* Ring the bell (or flash) */ 
	errormsg = 0; 
} 
 
char * 
indexs(pattern, string) 
register char	*pattern, 
		*string; 
{ 
	register int	len = strlen(pattern); 
 
	while (*string) { 
		if (*pattern == *string && strncmp(pattern, string, len) == 0) 
			return (string); 
		string++; 
	} 
	return ((char *)0); 
} 
 
Apropos() 
{ 
	register struct funct	*fp; 
	register struct macro	*m; 
	register struct variable	*v; 
	char	*ans; 
 
	ans = ask((char *) 0, "Apropos (keyword) "); 
	if (UseBuffers) 
		TellWBuffers("Apropos", 0); 
	else 
		TellWScreen(0); 
	for (fp = commands; fp->Name; fp++) 
		if (indexs(ans, fp->Name)) 
			ignore(DoTell(fp->Name)); 
 
	for (v = variables; v->Name; v++) 
		if (indexs(ans, v->Name)) 
			ignore(DoTell(v->Name)); 
 
	for (m = macros; m; m = m->m_nextm) 
		if (indexs(ans, m->Name)) 
			ignore(DoTell(m->Name)); 
 
	if (UseBuffers) { 
		ToFirst();		/* Go to the beginning of the file */ 
		NotModified(); 
	} 
	StopTelling(); 
} 
 
 
Extend() 
{ 
	data_obj	*d; 
 
	d = findcom(commands, ": ", NOTHING); 
	if ((int) d == -1) 
		return -1; 
	else if (d) 
		ExecFunc(d, !InJoverc); 
	return 0; 
} 
			 
#define fnumcomp(s1, s2)	(*(s1) != *(s2) ? 0 : numcomp(s1, s2)) 
 
/* Command completion - possible is an array of strings, prompt is 
   the prompt to use, and flags are ... well read jove.h. 
 
   If flags are RET_STATE, and the user hits <return> what they typed 
   so far is in the Minibuf string. */ 
 
complete(possible, prompt, flags) 
char	*possible[]; 
char	*prompt; 
{ 
	char	*cp,	/* Offset into Minibuf */ 
		*begin,	/* Beginning of type-in (not including prompt) */ 
		c; 
	int	minmatch, 
		command, 
		i, 
		lastmatch, 
		numfound, 
		length, 
		exactmatch; 
 
	extern int	(*Getchar)(), 
			NoMacGetc(), 
			Interactive; 
	int	(*HowToRead)() = (Interactive && !InJoverc) 
					? NoMacGetc : Getchar; 
 
	strcpy(Minibuf, prompt); 
	message(Minibuf); 
	Asking = strlen(prompt); 
	begin = Minibuf + Asking; 
	cp = begin; 
	*cp = 0; 
 
	while (c = (*HowToRead)()) { 
		switch (c) { 
		case EOF: 
			return EOF; 
 
		case CTL(U): 
			cp = begin; 
			*cp = 0; 
			break; 
 
		case CTL(G): 
abort: 
			complain("Aborted"); 
 
		case '\n':		/* fhsu: maybe accept anything... */ 
			if (flags & LF_FORCE) { 
				Asking = 0; 
				if (cp == begin) 
					return NULLSTRING; 
				if (InJoverc) 
					complain((char *) 0); 
				return ORIGINAL; 
			} 
 
		case '\r': 
			command = match(possible, begin); 
			if (command >= 0) { 
				Asking = 0; 
				return command; 
			} 
			Asking = 0; 
			if (cp == begin) 
				return NULLSTRING; 
			if (InJoverc) 
				complain((char *) 0); 
			if (flags & RET_STATE) switch (command) { 
				case UNIQUE: 
				case ORIGINAL: 
				case NULLSTRING: 
					return command; 
 
				default: 
					break; 
			} 
			rbell(); 
			break; 
 
		case '\033':		/* Try to complete command */ 
		case ' ': 
			minmatch = 1000; 
			numfound = 0; 
			exactmatch = lastmatch = -1; 
			length = strlen(begin); 
			for (i = 0; possible[i]; i++) 
				if (fnumcomp(possible[i], begin) >= length) { 
					if (numfound) 
						minmatch = min(minmatch, fnumcomp(possible[lastmatch], possible[i])); 
					else 
						minmatch = strlen(possible[i]); 
					lastmatch = i; 
					if (!strcmp(begin, possible[i])) { 
						exactmatch = i; 
						break; 
					} 
					numfound++; 
				} 
 
			if (lastmatch < 0) { 
				rbell(); 
				if (InJoverc) 
					complain((char *) 0); 
				break; 
			} 
			strncpy(begin, possible[lastmatch], minmatch); 
			begin[minmatch] = '\0'; 
			cp = &begin[minmatch]; 
			if (numfound == 1 || exactmatch != -1) { 
				if (c == '\033') { 
					f_mess(Minibuf); 
					break;	/* Don't return, just show */ 
				} 
				Asking = 0; 
				return (numfound == 1) ? lastmatch : 
						exactmatch; 
			} else if (minmatch == length)	/* No difference */ 
				rbell(); 
			break; 
 
		case '?': 
			if (InJoverc) 
				complain((char *) 0); 
			if (do_more(begin, possible, "Help") == ABORT) 
				goto abort; 
			break; 
 
		default: 
			if ((int) (cp = RunEdit(c, begin, cp, HowToRead)) == -1) 
				complain("Aborted"); 
		} 
		Asking = cp - Minibuf;		/* Wonderful! */ 
		message(Minibuf); 
	} 
	/* NOTREACHED */ 
} 
 
match(choices, what) 
register char	**choices, 
		*what; 
{ 
	register int	len; 
	int	i, 
		found = 0, 
		save, 
		exactmatch = -1; 
 
	len = strlen(what); 
	if (len == 0) 
		return NULLSTRING; 
	for (i = 0; choices[i]; i++) { 
		if (*what != *(choices[i])) 
			continue; 
		if (strncmp(what, choices[i], len) == 0) { 
			if (strcmp(what, choices[i]) == 0) 
				exactmatch = i; 
			save = i; 
			found++;	/* Found one. */ 
		} 
	} 
 
	if (found == 0) 
		save = ORIGINAL; 
	else if (found > 1) { 
		if (exactmatch != -1) 
			save = exactmatch; 
		else 
			save = AMBIGUOUS; 
	} 
 
	return save; 
} 
 
Source() 
{ 
	char	*com, 
		buf[256], 
		fname[MAXFILLEN]; 
	ignore(sprintf(buf, "%s/.joverc", getenv("HOME"))); 
	com = ask_file(buf, FuncName(), fname, FIL_ONLY); 
	if (joverc(com) == -1) 
		complain(IOerr("read", com)); 
} 
 
joverc(file) 
char	*file; 
{ 
	return FileComs(file, Extend); 
} 
 
FileComs(file, func) 
char	*file; 
int	(*func)(); 
{ 
	char	buf[LBSIZE]; 
	int	lnum = 0, 
		err = 0; 
	jmp_buf	savejmp; 
 
	if ((io = open(file, 0)) == -1) 
		return -1; 
 
	/* Catch any errors, here, and do the right thing with them, 
	   and then restore the error handle to whoever did a setjmp 
	   last. */ 
 
	copynchar((char *) savejmp, (char *) mainjmp, sizeof savejmp); 
	if (setjmp(mainjmp)) { 
		Buffer	*savebuf = curbuf; 
 
		SetBuf(do_select((Window *) 0, "RC errors")); 
		ins_str(sprint("Source error, file \"%s\", line %d: %s", 
					file, lnum, buf)); 
		LineInsert(); 
		SetUnmodified(curbuf); 
		SetBuf(savebuf); 
		err++; 
	} 
	InJoverc = 1; 
	while (getfline(buf) != EOF) { 
		lnum++; 
		strcat(buf, "\n"); 
		Inputp = buf; 
		(*func)(); 
	} 
 
	copynchar((char *) mainjmp, (char *) savejmp, sizeof mainjmp); 
	Inputp = 0; 
	InJoverc = 0; 
	IOclose(); 
	Asking = 0; 
	return 1; 
} 
 
BufPos() 
{ 
	register Line	*lp = curbuf->b_first; 
	register int	i, 
			dotline, 
			linelen; 
 
	for (i = 1; lp != 0; i++, lp = lp->l_next) { 
		if (lp == curline) 
			dotline = i; 
	} 
	linelen = strlen(linebuf); 
	s_mess("\"%s\" line %d of %d, column %d of %d", 
			filename(curbuf), 
			dotline, 
			i - 1, 
			1 + calc_pos(linebuf, curchar), 
			1 + calc_pos(linebuf, linelen)); 
} 
 
extern char	quots[]; 
 
SetQchars() 
{ 
	char	*chars = ask((char *) 0, FuncName()); 
 
	strncpy(quots, chars, 10);	/* Hope that they were reasonable characters */ 
} 
 
/* Minor modes */ 
 
OvrWrite() 
{ 
	TogMinor(OverWrite); 
} 
 
AutoFill() 
{ 
	TogMinor(Fill); 
} 
 
AutoSave() 
{ 
	TogMinor(Save); 
} 
 
ShowMtch() 
{ 
	TogMinor(ShowMatch); 
} 
 
AutoIndent() 
{ 
	TogMinor(Indent); 
} 
 
/* Major mode(s) */ 
 
char	*Compiler, 
	*CmpPath, 
	*CmpFlags; 
 
Cmode() 
{ 
	UpdModLine++; 
	SetMajor(CMODE); 
	if (!exp_p) SetMinor(ShowMatch); 
	bcopy("/*", CommentBegin, 3); 
	bcopy("*/", CommentEnd, 3); 
	Compiler = ": cc "; 
	CmpPath = "/bin/cc"; 
	CmpFlags = getenv("CFLAGS"); 
} 
 
TextMode() 
{ 
	UpdModLine++; 
	SetMajor(TEXT); 
	SetMinor(Fill); 
	*CommentBegin = '\0'; 
	*CommentEnd = '\0'; 
	Compiler = (char *)0; 
	CmpPath = (char *)0; 
	CmpFlags = (char *)0; 
} 
 
PascalMode() 
{ 
	UpdModLine++; 
	SetMajor(PASCALMODE); 
	if (!exp_p) SetMinor(ShowMatch); 
	bcopy("(*", CommentBegin, 3); 
	bcopy("*)", CommentEnd, 3); 
	Compiler = ": pc "; 
	CmpPath = "/usr/ucb/pc"; 
	CmpFlags = getenv("PFLAGS"); 
} 
 
EuclidMode() 
{ 
	UpdModLine++; 
	SetMajor(EUCLIDMODE); 
	if (!exp_p) SetMinor(ShowMatch); 
	bcopy("{", CommentBegin, 2); 
	bcopy("}", CommentEnd, 2); 
	Compiler = ": cec "; 
	CmpPath = "/usr/local/cec"; 
	CmpFlags = getenv("CEFLAGS"); 
} 
 
AsmMode() 
{ 
	UpdModLine++; 
	SetMajor(ASMMODE); 
	if (!exp_p) SetMinor(ShowMatch); 
	bcopy("/*", CommentBegin, 3); 
	bcopy("*/", CommentEnd, 3); 
	Compiler = ": as "; 
	CmpPath = "/bin/as"; 
	CmpFlags = getenv("ASFLAGS"); 
} 
 
FundMode() 
{ 
	UpdModLine++; 
	SetMajor(FUNDMODE); 
	if (!exp_p) SetMinor(ShowMatch); 
	bcopy("#", CommentBegin, 2); 
	*CommentEnd = '\0'; 
	Compiler = (char *)0; 
	CmpPath = (char *)0; 
	CmpFlags = (char *)0; 
} 
 
ScribeMode() 
{ 
	UpdModLine++; 
	SetMajor(SCRIBEMODE); 
	if (!exp_p) { 
		SetMinor(ShowMatch); 
		SetMinor(Fill); 
	} 
	bcopy("@comment[", CommentBegin, 10); 
	bcopy("]", CommentEnd, 2); 
	Compiler = ": scribe "; 
	CmpPath = "/usr/local/scribe"; 
	CmpFlags = getenv("SCRIBEFLAGS"); 
} 
 
CluMode() 
{ 
	UpdModLine++; 
	SetMajor(CLUMODE); 
	if (!exp_p) SetMinor(ShowMatch); 
	bcopy("%", CommentBegin, 2); 
	*CommentEnd = '\0'; 
	Compiler = ": clu "; 
	CmpPath = "/usr/clu/exe/clu"; 
	CmpFlags = getenv("CLUFLAGS"); 
} 
 
TexMode() 
{ 
	UpdModLine++; 
	SetMajor(TEXMODE); 
	if (!exp_p) { 
		SetMinor(ShowMatch); 
		SetMinor(Fill); 
	} 
	bcopy("%", CommentBegin, 2); 
	*CommentEnd = '\0'; 
	Compiler = ": latex "; 
	CmpPath = "/usr/local/latex"; 
	CmpFlags = getenv("TEXFLAGS"); 
} 
 
resetmode(bp) 
	Buffer *bp; 
{ 
	switch(bp->b_major) { 
		case TEXT:		TextMode(); break; 
		case CMODE:		Cmode(); break; 
		case CLUMODE:		CluMode(); break; 
		case EUCLIDMODE:	EuclidMode(); break; 
		case SCRIBEMODE:	ScribeMode(); break; 
		case TEXMODE:		TexMode(); break; 
		case ASMMODE:		AsmMode(); break; 
		case PASCALMODE:	PascalMode(); break; 
		case FUNDMODE:		FundMode(); break; 
		default:		FundMode(); 
	} 
} 
 
char * 
get_major_string(mode) 
{ 
	char *ret; 
 
	switch(mode) { 
		case TEXT:		ret = "Text "; break; 
		case CMODE:		ret = "C "; break; 
		case CLUMODE:		ret = "Clu "; break; 
		case EUCLIDMODE:	ret = "Euclid "; break; 
		case SCRIBEMODE:	ret = "Scribe "; break; 
		case TEXMODE:		ret = "Tex "; break; 
		case ASMMODE:		ret = "Asm "; break; 
		case PASCALMODE:	ret = "Pascal "; break; 
		case FUNDMODE:		ret = "Fund "; break; 
		default:		ret = "Text "; 
	} 
	return(ret); 
} 
 
 
#ifdef FCOMPLETE 
 
/****************************************************************** 
	New filename completion routines 
******************************************************************/ 
 
char * 
fcomplete(def, prompt, flags) 
char	*def; 
char	*prompt; 
{ 
	char	*cp,	/* Offset into Minibuf */ 
		*begin,	/* Beginning of type-in (not including prompt) */ 
		c, 
		*tcp, 
		buf[MAXFILLEN], 
		**possible; 
	int	minmatch, 
		command, 
		i, 
		lastmatch, 
		numfound, 
		length, 
		exactmatch, 
		completed; 
 
	extern int	(*Getchar)(), 
			NoMacGetc(), 
			Interactive; 
	int	(*HowToRead)() = (Interactive && !InJoverc) 
					? NoMacGetc : Getchar; 
 
	strcpy(Minibuf, prompt); 
	message(Minibuf); 
	Asking = strlen(prompt); 
	begin = Minibuf + Asking; 
	cp = begin; 
	*cp = 0; 
	completed = 0; 
 
	while (c = (*HowToRead)()) { 
		switch (c) { 
		case EOF: 
			complain( (char *) 0); 
 
		case CTL(U): 
			cp = begin; 
			*cp = 0; 
			completed = 0; 
			break; 
 
		case CTL(G): 
abort: 
			complain("Aborted"); 
 
		case CTL(R): 
			if (tcp = def) { 
			while (*tcp) 
				cp = RunEdit(*tcp++, begin, cp, HowToRead); 
			} 
			completed = 0; 
			break; 
 
		case '\r': 
		case '\n': 
			Asking = 0; 
			if (cp == begin) { 
				PathParse(def, begin); 
				return begin; 
			} 
			if (InJoverc) 
				complain((char *) 0); 
			i = is_directory(begin); 
			if (((flags&FIL_ONLY) && i) || 
			    ((flags&DIR_ONLY) && !i)) { 
				rbell(); 
				break; 
			} 
			if (!completed) { 
				PathParse(begin, buf); 
				strcpy(begin, buf); 
			} 
			return begin; 
 
 
		case '\033':		/* Try to complete filename */ 
		case ' ':		/* cannot be called from .joverc */ 
			if (InJoverc) 
				complain((char *) 0); 
			PathParse(begin, buf); 
			strcpy(begin, buf); 
			cp = &begin[strlen(begin)]; 
			completed = 1; 
			if (is_directory(buf)) 
				strcat(buf, "/*"); 
			else 
				strcat(buf, "*"); 
			possible = (char **) glob(buf, 1); 
			if (possible == 0) { 
				rbell(); 
				break; 
			} 
			minmatch = 1000; 
			numfound = 0; 
			exactmatch = lastmatch = -1; 
			length = strlen(begin); 
			for (i = 0; possible[i]; i++) { 
				register char *possib = possible[i]; 
				if (fnumcomp(possib, begin) >= length) { 
					if (numfound) 
						minmatch = min(minmatch, fnumcomp(possible[lastmatch], possib)); 
					else 
						minmatch = strlen(possib); 
					lastmatch = i; 
					if (!strcmp(begin, possib)) { 
						exactmatch = i; 
						break; 
					} 
					numfound++; 
				} 
			} 
			if (lastmatch < 0) { 
				rbell(); 
				blkfree(possible); 
				break; 
			} 
			strncpy(begin, possible[lastmatch], minmatch); 
			begin[minmatch] = '\0'; 
			cp = &begin[minmatch]; 
			strcpy(buf, (numfound == 1) ? possible[lastmatch] : 
						      possible[exactmatch]); 
			blkfree(possible); 
			if (numfound == 1 || exactmatch != -1) { 
				f_mess(Minibuf); 
				break;	/* Don't return, just show */ 
			} else if (minmatch == length)	/* No difference */ 
				rbell(); 
			break; 
 
		case '?': 
			if (InJoverc) 
				complain((char *) 0); 
			PathParse(begin, buf); 
#ifdef DIRLIST 
			do_dirTell(buf, "Directory listing", 1); 
#else 
			if (is_directory(buf)) 
				strcat(buf, "/*"); 
			else 
				strcat(buf, "*"); 
			possible = (char **) glob(buf, 1); 
			if (possible == 0) { 
				rbell(); 
				break; 
			} else { 
				int what = do_more(begin, possible, "Directory listing"); 
				blkfree(possible); 
				if (what == ABORT) { 
					goto abort; 
				} 
			} 
#endif 
			break; 
		case '\\': 
			c = (*HowToRead)(); 
		case '~': case '$': case '.': 
			completed = 0; 
		default: 
			if ((int) (cp = RunEdit(c, begin, cp, HowToRead)) == -1) 
				complain("Aborted"); 
		} 
		Asking = cp - Minibuf;		/* Wonderful! */ 
		message(Minibuf); 
	} 
	/* NOTREACHED */ 
} 
#endif 
 
do_more(str, possible, bufname) 
	register char *str; 
	register char *possible[]; 
	char *bufname; 
{ 
	register int i; 
	register int length = strlen(str); 
	if (UseBuffers) 
		TellWBuffers(bufname, 1); 
	else 
		TellWScreen(0); 
	for (i = 0; possible[i]; i++) { 
		if (fnumcomp(possible[i], str) >= length) { 
			int what = DoTell(possible[i]); 
			if (what == ABORT) 
				return(ABORT); 
			if (what == STOP) 
				break; 
		} 
	} 
	if (UseBuffers) 
		ToFirst(); 
	StopTelling(); 
	return(STOP); 
} 

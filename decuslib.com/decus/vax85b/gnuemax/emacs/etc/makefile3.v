head     1.2;A
access   ; 
symbols  ; 
locks    bbanerje:1.2; strict; 
comment  @# @; 
 
 
1.2 
date     85.03.31.18.37.00;  author bbanerje;  state Exp; 
branches ; 
next     1.1; 
 
1.1 
date     85.03.31.18.35.51;  author bbanerje;  state Exp; 
branches ; 
next     ; 
 
 
desc 
@@ 
 
 
1.2 
log 
@Changed -g flag on compile to -O -s. 
@ 
text 
@all: etags ctags loadst make-docfile 
 
etags: etags.c 
	cc -o etags -O -s -DETAGS etags.c 
 
ctags: etags.c 
	cc -o ctags -O -s -DCTAGS etags.c 
 
loadst: loadst.c 
	cc -o loadst -O -s loadst.c 
 
make-docfile: make-docfile.c 
	cc -o make-docfile -O -s make-docfile.c 
@ 
 
 
1.1 
log 
@Initial revision 
@ 
text 
@d4 1 
a4 1 
	cc -o etags -g -DETAGS etags.c 
d7 1 
a7 1 
	cc -o ctags -g -DCTAGS etags.c 
d10 1 
a10 1 
	cc -o loadst -g loadst.c 
d13 1 
a13 1 
	cc -o make-docfile -g make-docfile.c 
@ 

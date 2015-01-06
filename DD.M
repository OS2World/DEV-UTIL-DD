SRC    =.^\
PSRC   =P:^\
QSRC   =Q:^\
QOBJ   =Q:\OB2^\
OBJ    =OB2^\
 
# DYNA LINK
#CFLAGS =-C -Ses -Q -W3 -Kbrfa-g-o-t-i-c- -Gdmshen- -Mp -Ti -Fd-
#LIBS   = OS2386+DDE4MBSI
# STAT LINK
CFLAGS =-C -Ses -Q -W3 -Kbrfa-g-o-t-i-c- -Gd-mshen- -Mp -Ti -Fd-
LIBS   = OS2386+DDE4MBS
LFLAGS = /NoLogo /Align:4 /Map /LineNumbers /De /NoI /Noe 

DDT    = $(SRC)ddT.c $(SRC)ddT1.c $(SRC)ddT2.c $(SRC)ddT3.c \
                     $(SRC)ddT4.c $(SRC)ddT5.c
 
dd.ddi     : dd.exe 
  dd i /b dd

dd.exe     : dd.m dd.def       \
             $(OBJ)ddm.obj     \
             $(OBJ)dd.obj      \
             $(OBJ)ddi.obj     \
             $(OBJ)ddexec.obj  
 LINK386 @<<dd.lnk
$(LFLAGS)       +
$(OBJ)ddm.obj   +
$(OBJ)dd.obj    +
$(OBJ)ddi.obj   +
$(OBJ)ddexec.obj+
setargv
dd.exe
dd.Map
$(LIBS)
dd.def;
<<

$(OBJ)ddm.obj    : $(SRC)ddi.h $(SRC)ddexec.h $(DDT)
$(OBJ)dd.obj     : $(SRC)dd.h  $(SRC)ddexec.h $(SRC)ddb.h  $(SRC)ddi.h
$(OBJ)ddi.obj    : $(SRC)ddi.h
$(0BJ)ddexec.obj : $(SRC)dd.h  $(SRC)ddexec.h



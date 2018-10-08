$!------------------------------------------------------------------------
$! MAKEFILE.COM
$!------------------------------------------------------------------------
$  set noverify
$! Check your system
$!
$  arch_name=f$getsyi("ARCH_NAME")
$  WRITE SYS$OUTPUT "System is ''arch_name'."
$  WRITE SYS$OUTPUT "-------------------------------------"
$  WRITE SYS$OUTPUT "Compiling sources files ..."
$!
$  WRITE SYS$OUTPUT "Compiling file FSPEC_PT.''arch_name'_MAR ..."
$  mac/nodebug/object=FSPEC_PT FSPEC_PT.'arch_name'_MAR
$!
$  WRITE SYS$OUTPUT "Compiling file VTFM.C ..."
$  cc/nodebug/object=VTFM VTFM
$!
$  WRITE SYS$OUTPUT "Linking VTFM.EXE ..."
$  link/nodebug/notraceback VTFM,FSPEC_PT
$!
$! - Clean directory -----------------------------------------------------
$  WRITE SYS$OUTPUT "Cleaning DIRECTORY ..."
$! - Check if exixst *.obj and *.exe -----
$  IF F$SEARCH("*.OBJ") .EQS. ""
$  THEN
$      WRITE SYS$OUTPUT "->>> There is nothing to delete ..."
$  ELSE
$      WRITE SYS$OUTPUT "->>> Deleting *.OBJ files ..."
$      DELETE *.OBJ;*
$  ENDIF
$  IF F$SEARCH("VTFM.EXE") .EQS. ""
$  THEN
$      WRITE SYS$OUTPUT "->>> There is nothing to PURGE ..."
$  ELSE
$      WRITE SYS$OUTPUT "->>> Rurging VTFM.EXE files ..."
$      PURGE VTFM.EXE
$  ENDIF
$!------------------------------------------------------------------------
$ EXIT
$!------------------------------------------------------------------------

Copyright (c) 1991-2004 VTfm.  All rights reserved.

		Norton Commander Style
	     Video Terminals File Manager
		     VMS-version

Author: Vladimir K. Vershinin, Moscow, Russia
E-mail: vershinin-vk@tochka.ru

History:

1991, July - First release 1.0 with name NCvtf.

	- developed on VAXC v3.0;
	- supports VT-terminal modes with only
	  24x80 rows/columns;
	- uses EDT$EDIT routine for view and edit;
	- works only on VAXVMS.
 .
 .
 .

2004, August - Release 2.0-0 with name VTfm.

	- developed on DECC v6.0;
	- redesigned and reduced source code;
	- supports VT-terminal modes with more
	  then 24x80 rows/columns;
	- uses TPU$TPU routine for view and edit;
	- works on VAX, Alpha and IA64 with OpenVMS.

Currently known restrictions:

	- does not accept node specification in
	  GotoDir, Edit, Copy and RenMov operations
	  (use VTfm command line instead in this cases);
	- does not translate disk device logicals in
	  RenMov operation, so if, for example,
	  logicals XXX: and YYY: point to the SAME
	  physical disk device VTfm RenMov files
	  via copy;
	- does not Copy directory files with their
	  content (make destination directory first
	  and then Copy source directory content);
	- does not RenMov directory files with their
	  content to ANOTHER logical or physical disk
	  device (make destination directory first
	  and then RenMov source directory content);
	- does not open any archive files (use
	  correspondent utility in VTfm command line).

2004, October - Release 2.2-6

	- Copy, RenMov and Delete DIRECTORY TREES;
	- supports LOGICALs when GotoDir, Copy or
	  RenMov (SYS$LOGIN, SYS$MANAGER, SYS$COMMON,
	  SYS$COMMON:[SYSMGR] e.t.c.);
	- recognizes the same disk device with DIFFERENT
	  logical names when RenMov, so if, for example,
	  logicals XXX: and YYY: point to the SAME
	  physical disk VTfm RenMov files WITHOUT copy;
	- works on VAX, Alpha and IA64 with OpenVMS.

Currently known restrictions:

	- supports only LOCAL or CLUSTERWIDE disk
	  devices when GotoDir, Copy, RenMov or Delete
	  (if need to use NODE specification use VTfm
	  command line);
	- does not support "*", "[.", ".]", "[-", ".-"
	  and "..." in DIRECTORY specification when
	  GotoDir, Copy or RenMov;
	- does not support EXTENDED FILE SPECIFICATIONS
	  on ODS-5 volumes;
	- does not open any archive files (use
	  correspondent utility in VTfm command line).

2004, October - Release 2.2-8

	- some changes and additions of function keys
	  when input and edit lines (see also 19FKey):
		^P - extract previous command line
		     (instead of ^E),
		^H - cursor to beg of line,
		^E - cursor to end of line,
		^U - delete chars from beg of line
		     to current position;
	- some minor bug fixes.
	- works on VAX, Alpha and IA64 with OpenVMS.

Currently known restrictions:

	- supports only LOCAL or CLUSTERWIDE disk
	  devices when GotoDir, Copy, RenMov or Delete
	  (if need to use NODE specification use VTfm
	  command line);
	- does not support "*", "[.", ".]", "[-", ".-"
	  and "..." in DIRECTORY specification when
	  GotoDir, Copy or RenMov;
	- does not support EXTENDED FILE SPECIFICATIONS
	  on ODS-5 volumes;
	- does not open any archive files (use
	  correspondent utility in VTfm command line).

2004, November - Release 2.2-9

	- speed up Copy operation more than 3 times
	  (127 blocks size of I/O buffer);
	- change ^P function key (extract Previous
	  command line) to ^V (extract preVious
	  command line) for use VTfm on Console
	  terminal;
	- works on VAX, Alpha and IA64 with OpenVMS.

Currently known restrictions:

	- supports only LOCAL or CLUSTERWIDE disk
	  devices when GotoDir, Copy, RenMov or Delete
	  (if need to use NODE specification use VTfm
	  command line);
	- does not support "*", "[.", ".]", "[-", ".-"
	  and "..." in DIRECTORY specification when
	  GotoDir, Copy or RenMov;
	- does not support EXTENDED FILE SPECIFICATIONS
	  on ODS-5 volumes;
	- does not open any archive files (use
	  correspondent utility in VTfm command line).

Building executable:

For building VTfm executable unzip VTFM.ZIP first in some
directory and then use the following DCL commands:

	$ set default [.vtfm]
	$ @vtfm.com

The result of this operation is VTFM.VAX_EXE for VAX,
VTFM.ALPHA_EXE for ALPHA or VTFM.IA64_EXE for IA64.

VTfm works with Digital VT-series terminals or terminal
emulators which can emulate such terminals (PowerTerm,
for example).

Operation keys:

VTfm operation keys for Digital VT-series terminals are:

	11 GotoVMS - <F11>,
	12 GotoDir - <F12>,
	13 View    - <F13>,
	14 Edit    - <F14>,
	15 Copy    - <Help>,
	16 RenMov  - <Do>,
	17 MkDir   - <F17>,
	18 Delete  - <F18>,
	19 FKey    - <F19>,
	20 Quit    - <F20>.

See correspondent Keyboard Map when use terminal emulator.


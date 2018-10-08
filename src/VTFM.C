/*
**	Copyright (c) 1991-2004 VTfm.  All rights reserved.
**
**			Norton Commander Style
**		     Video Terminals File Manager
**			     VMS-version
**
**	Author: Vladimir K. Vershinin, Moscow, Russia
**	E-mail: vershinin-vk@tochka.ru
**
**	History:
**
**	1991, July - First release 1.0 with name NCvtf.
**
**		- developed on VAXC v3.0;
**		- supports VT-terminal modes with only
**		  24x80 rows/columns;
**		- uses EDT$EDIT routine for view and edit;
**		- works only on VAXVMS.
**	 .
**	 .
**	 .
**
**	2004, August - Release 2.0-0 with name VTfm.
**
**		- developed on DECC v6.0;
**		- redesigned and reduced source code;
**		- supports VT-terminal modes with more
**		  then 24x80 rows/columns;
**		- uses TPU$TPU routine for view and edit;
**		- works on VAX, Alpha and IA64 with OpenVMS.
**
**	Currently known restrictions:
**
**		- does not accept node specification in
**		  GotoDir, Edit, Copy and RenMov operations
**		  (use VTfm command line instead in this cases);
**		- does not translate disk device logicals in
**		  RenMov operation, so if, for example,
**		  logicals XXX: and YYY: point to the SAME
**		  physical disk device VTfm RenMov files
**		  via copy;
**		- does not Copy directory files with their
**		  content (make destination directory first
**		  and then Copy source directory content);
**		- does not RenMov directory files with their
**		  content to ANOTHER logical or physical disk
**		  device (make destination directory first
**		  and then RenMov source directory content);
**		- does not open any archive files (use
**		  correspondent utility in VTfm command line).
**
**	2004, October - Release 2.2-6
**
**		- Copy, RenMov and Delete DIRECTORY TREES;
**		- supports LOGICALs when GotoDir, Copy or
**		  RenMov (SYS$LOGIN, SYS$MANAGER, SYS$COMMON,
**		  SYS$COMMON:[SYSMGR] e.t.c.);
**		- recognizes the same disk device with DIFFERENT
**		  logical names when RenMov, so if, for example,
**		  logicals XXX: and YYY: point to the SAME
**		  physical disk VTfm RenMov files WITHOUT copy;
**		- works on VAX, Alpha and IA64 with OpenVMS.
**
**	Currently known restrictions:
**
**		- supports only LOCAL or CLUSTERWIDE disk
**		  devices when GotoDir, Copy, RenMov or Delete
**		  (if need to use NODE specification use VTfm
**		  command line);
**		- does not support "*", "[.", ".]", "[-", ".-"
**		  and "..." in DIRECTORY specification when
**		  GotoDir, Copy or RenMov;
**		- does not support EXTENDED FILE SPECIFICATIONS
**		  on ODS-5 volumes;
**		- does not open any archive files (use
**		  correspondent utility in VTfm command line).
**
**	2004, October - Release 2.2-8
**
**		- some changes and additions of function keys
**		  when input and edit lines (see also 19FKey):
**			^P - extract previous command line
**			     (instead of ^E),
**			^H - cursor to beg of line,
**			^E - cursor to end of line,
**			^U - delete chars from beg of line
**			     to current position;
**		- some minor bug fixes.
**		- works on VAX, Alpha and IA64 with OpenVMS.
**
**	Currently known restrictions:
**
**		- supports only LOCAL or CLUSTERWIDE disk
**		  devices when GotoDir, Copy, RenMov or Delete
**		  (if need to use NODE specification use VTfm
**		  command line);
**		- does not support "*", "[.", ".]", "[-", ".-"
**		  and "..." in DIRECTORY specification when
**		  GotoDir, Copy or RenMov;
**		- does not support EXTENDED FILE SPECIFICATIONS
**		  on ODS-5 volumes;
**		- does not open any archive files (use
**		  correspondent utility in VTfm command line).
**
**	2004, November - Release 2.2-9
**
**		- speed up Copy operation more than 3 times
**		  (127 blocks size of I/O buffer);
**		- change ^P function key (extract Previous
**		  command line) to ^V (extract preVious
**		  command line) for use VTfm on Console
**		  terminal;
**		- works on VAX, Alpha and IA64 with OpenVMS.
**
**	Currently known restrictions:
**
**		- supports only LOCAL or CLUSTERWIDE disk
**		  devices when GotoDir, Copy, RenMov or Delete
**		  (if need to use NODE specification use VTfm
**		  command line);
**		- does not support "*", "[.", ".]", "[-", ".-"
**		  and "..." in DIRECTORY specification when
**		  GotoDir, Copy or RenMov;
**		- does not support EXTENDED FILE SPECIFICATIONS
**		  on ODS-5 volumes;
**		- does not open any archive files (use
**		  correspondent utility in VTfm command line).
*/
#include stdio
#include stdlib
#include string
#include stat
#include ctype
#include time
#include math

#include errno
#include descrip
#include ssdef
#include smgdef
#include smgmsg
#include strdef
#include rmsdef
#include fab
#include nam
#include xab
#include rab
#include dvidef
#include dcdef
#include tpadef
#include fscndef
#include iodef
#include lnmdef
#include syidef

#define SMG$M_NOBORDER	0

#define LINE_ADV	1
#define WRAP_FLAG	1
#define ERASE_FLAG	0

#define MAX_DIR_FQUA	4096
#define MAX_COMM_QUA	128
#define REC_BUFF_SIZE	65024

#define	CURSOR_TO_HOME	printf("\033[H")
#define	INST_LAT_FONT	printf("\033[62;1\"p\033\033\033(B")
#define PAK_MESS	printf("\n%s",vtfm_bgt[8]+1)

extern int	LIB$CREATE_DIR(),
		LIB$FIND_FILE(),
		LIB$FIND_FILE_END(),
		LIB$GETDVI(),
		LIB$GETSYI(),
		LIB$SET_LOGICAL(),
		LIB$SPAWN(),
		LIB$TABLE_PARSE();

extern int	SMG$CHANGE_RENDITION(),
		SMG$CHANGE_VIRTUAL_DISPLAY(),
		SMG$CREATE_PASTEBOARD(),
		SMG$CREATE_VIRTUAL_DISPLAY(),
		SMG$CREATE_VIRTUAL_KEYBOARD(),
		SMG$DELETE_CHARS(),
		SMG$DELETE_PASTEBOARD(),
		SMG$DELETE_VIRTUAL_DISPLAY(),
		SMG$DELETE_VIRTUAL_KEYBOARD(),
		SMG$ERASE_CHARS(),
		SMG$ERASE_DISPLAY(),
		SMG$ERASE_PASTEBOARD(),
		SMG$FIND_CURSOR_DISPLAY(),
		SMG$GET_DISPLAY_ATTR(),
		SMG$INSERT_CHARS(),
		SMG$LABEL_BORDER(),
		SMG$PASTE_VIRTUAL_DISPLAY(),
		SMG$PUT_CHARS(),
		SMG$PUT_LINE(),
		SMG$READ_KEYSTROKE(),
		SMG$RESTORE_PHYSICAL_SCREEN(),
		SMG$RETURN_CURSOR_POS(),
		SMG$RING_BELL(),
		SMG$SAVE_PHYSICAL_SCREEN(),
		SMG$SCROLL_DISPLAY_AREA(),
		SMG$SET_CURSOR_ABS(),
		SMG$SET_KEYPAD_MODE(),
		SMG$UNPASTE_VIRTUAL_DISPLAY();

extern int	STR$MATCH_WILD();

extern int	SYS$ASSIGN(),
		SYS$CLOSE(),
		SYS$CONNECT(),
		SYS$CREATE(),
		SYS$DASSGN(),
		SYS$ERASE(),
		SYS$FILESCAN(),
		SYS$GETMSG(),
		SYS$OPEN(),
		SYS$QIOW(),
		SYS$READ(),
		SYS$RENAME(),
		SYS$SETDDIR(),
		SYS$TRNLNM(),
		SYS$WRITE();

extern int	TPU$TPU();

globalvalue	TPU$_SUCCESS,
		TPU$_EXITING,
		TPU$_QUITTING;

extern int	FSPEC_ST_TBL,
		FSPEC_KW_TBL;

int	str_centr_pos(),
	probe_dir_cont(),
	probe_file_exist(),
	get_max_fver(),
	get_phys_disk_name(),
	delete_subdir_tree(),
	copmov_subdir_tree(),
	scan_file_spec(),
	copy_proc(),
	translate_lnm(),
	translate_dirlnm(),
	parse_file_spec();

void	get_sysmsg(),
	get_term_row_col(),
	set_term_row_col(),
	start_session(),
	end_session(),
	set_def_dir(),
	chg_oper_dir(),
	cursor_to_br(),
	ring_bell(),
	disp_w_label(),
	cre_s_str_desc(),
	str_tolower(),
	str_toupper(),
	read_dir(),
	paste_dir_w(),
	chg_dir_brack_comma(),
	disp_ext_fspec(),
	disp_dir_spec(),
	disp_dir_cont(),
	file_bar_on(),
	file_bar_off(),
	prev_dir_file(),
	next_dir_file(),
	disp_file_stat(),
	set_file_selp(),
	prev_dir_page(),
	next_dir_page(),
	pf1_key_handler(),
	erase_dir_w(),
	cr_key_handler(),
	refresh_screen(),
	rotate_dir_w(),
	extr_file_ext(),
	find_dir_cpos(),
	before_restore(),
	restore_dir_cont(),
	disp_mess(),
	spawn_proc(),
	cok_bar_on(),
	cok_bar_off(),
	cok_handler(),
	str_shift(),
	input_w_line(),
	accept_wild_spec(),
	un_sel_files(),
	find_files(),
	cre_paste_filew(),
	delete_files(),
	copy_renmov_files(),
	goto_dir(),
	edit_file(),
	view_file(),
	make_dir(),
	fkey_desc(),
	invoke_tpu(),
	comm_handler();

static	char	*vtfm_bgt[]=
	{
	"",							/* [0] */
	"       Norton Commander Style",
	"    Video Terminals File Manager",
	"         VMS-version r2.2-9",
	"",
	"      by Vladimir K. Vershinin",
	"        Moscow, Russia, 2004",
	"",
	" Press any key... ",
	" Reading current directory... ",
	"Bad or inappropriate device",				/* [10] */
	"Invalid directory specification",
	"Directory not found",
	"Device is not ready",
	"Node is unknown or unreachable",
	"No files found",
	"Access to directory denied",
	"Unexpected RMS error %u",
	"Access to file denied",
	">UP-DIR<",
	">SUB-DIR<",						/* [20] */
	"Invalid file specification",
	" DELETING... ",
	"selected file",
	"Reading directories...",
	">ROOT-DIR<",
	".DIR",
	"11",
	"GotoVMS",
	" 12",
	"GotoDir",						/* [30] */
	" 13",
	"View",
	" 14",
	"Edit",
	" 15",
	"Copy",
	" 16",
	"RenMov",
	" 17",
	"MkDir",						/* [40] */
	" 18",
	"Delete",
	" 19",
	"FKey",
	" 20",
	"Quit",
	"*.*;*",
	"Logical name translation error",
	".DIR;32768",
	".exe",							/* [50] */
	".com",
	"RUN ",
	"Error in directory name",
	" EDIT ",
	"block",
	" DELETE ",
	"byte",
	"",
	" QUIT ",
	"Do you really want to quit the VTfm session?",		/* [60] */
	"Wildcard symbols * or % required",
	" GO TO DIRECTORY ",
	" SELECT ",
	" UNSELECT ",
	" FIND ",
	" will be DELETED",
	"Current file",
	"(VTfm)",
	"",
	"TPU$JOURNAL",						/* [70] */
	"SYS$SCRATCH:",
	".TPU$JOURNAL;*",
	"Cannot create a subprocess",
	" EDIT ",
	"Do you wish to CONTINUE operation?",
	"Do you wish to RECOVER this file?",
	" MAKE A DIRECTORY ",
	"Nondirectory file %s already exists",
	"(to return to VTfm simply LOgout)",
	"Directory already exists",				/* [80] */
	" RENAME OR MOVE",
	" CURRENT FILE",
	"SELECTED FILE",
	" TO ",
	" RENAMING OR MOVING... ",
	" COPY",
	"",
	" COPYING...",
	"",
	"Invalid directory name",				/* [90] */
	"",
	" FUNCTION KEYS ",
	" PANELS:                  OTHER:                                    ",
	"  Switch panels   <Tab>    Next dir page                  <NextScr> ",
	"  Rotate panels     ^W     Bottom of dir            <PF1>+<NextScr> ",
	" CURSOR MOVEMENT:          Previous dir page              <PrevScr> ",
	"  Chars        arr L/R     Top of dir               <PF1>+<PrevScr> ",
	"  Lines        arr U/D     Find file(s) by name              <Find> ",
	"  Beg of line       ^H     Select file(s) by name          <Select> ",

								/* [100] */

	"  End of line       ^E     Unselect file(s) by name        <Remove> ",
	" DELETE:                   Select/Unselect curr file      <InsHere> ",
	"  Char      <BackSpace>    File(s) size in blocks/bytes         ^B  ",
	"  Line            <PF4>    Insert curr file name into command   ^F  ",
	"  Beg of line       ^U     Insert curr dir name into command    ^D  ",
	"                           Extract previous command             ^V  ",
	" EXECUTE:      <Return>    Extract next command                 ^N  ",
	" ESCAPE:          <KP0>    Refresh screen                       ^R  ",
	"",
	"",
								/* [110] */
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
	"TPU",
	"/READ_ONLY ",						/* [120] */
	"/RECOVER "
	};

static	char	ext_file_spec[96];

static	char	comm_prompt[48];

static	char	rec_buff[REC_BUFF_SIZE];

static	short	key_pr;

static	int	prev_term_rq,
		prev_term_cq,
		term_row_qua,
		term_col_qua,
		pstb_id,
		keyb_id,
		main_w,
		oper_dir=0,	/* 0 if left or top panel	*/
				/* 1 if right or bottom panel	*/

		fsm_unit=1,	/* 0 if file size in bytes	*/
				/* 1 if file size in blocks	*/
		comm_w,
		comm_w_cq,
		comm_col=1,
		comm_cpos=0,
		comm_stack_qua=0,
		comm_stack_cpos=0;

static	struct
	{
		int	disp_id,
			row_qua,
			col_qua,
			s_row,
			s_col;
	}
		cur_dir_w[4][3];

static	struct	dsc$descriptor_s s_str_desc =
		{0,DSC$K_DTYPE_T,DSC$K_CLASS_S};

static  struct  dsc$descriptor_d d_str_desc =
                {0,DSC$K_DTYPE_T,DSC$K_CLASS_D};

static	struct	stat file_info;

static	struct	tm *file_ctime;

static	struct
	{
		char	comm_str[256];
	}
		spawn_comm[MAX_COMM_QUA];

struct	tag_dir_parms
	{
		char	dir_spec[256],
			find_spec[64],
			sel_spec[64];
		int	dir_fqua,
			ord_fqua,
			sel_fqua,
			dir_cpos,
			dir_wpos;
	};

static	struct	tag_dir_parms cur_dir_parms[2];

static	struct	tag_dir_parms old_dir_parms[2];

struct	tag_dir_cont
	{
		 char	file_spec[96],
			file_selp,	/* ' ' if not selected */
					/* '*' if selected */

			file_type;	/* < 0 if Message: */
					/*     -3 if Root directory */
					/*     -2 if not Root directory */
					/*        and dir ctime not exists */
					/*     -1 if not Root directory */
					/*        and dir ctime exists */
					/*  0 if access denied */
					/*  1 if File */
					/*  2 if Directory */

	unsigned int	file_size;	/* In bytes */
	unsigned long	file_date;	/* In seconds */
	};

static	struct	tag_dir_cont cur_dir_cont[2][MAX_DIR_FQUA];

static	struct	tag_dir_cont old_dir_cont[2][MAX_DIR_FQUA];

static	struct	tag_dir_cont ord_dir_file[MAX_DIR_FQUA];

static	struct
	{
		unsigned short	eqvlnm_buff_len,
				item_code;
			 void	*eqvlnm_buff_addr,
			 	*eqvlnm_len_addr;
			  int	term;
	}
		eqvlnm_item;

static	int	eqv_lnm_len;

main()
{
	start_session();

	do
	{
		SMG$READ_KEYSTROKE(&keyb_id,&key_pr);

		switch (key_pr)
		{
			case	SMG$K_TRM_HT:
					chg_oper_dir();
					break;
			case	SMG$K_TRM_UP:
					prev_dir_file(oper_dir);
					break;
			case	SMG$K_TRM_DOWN:
					next_dir_file(oper_dir);
					break;
			case	SMG$K_TRM_PREV_SCREEN:
					prev_dir_page(oper_dir,0);
					break;
			case	SMG$K_TRM_NEXT_SCREEN:
					next_dir_page(oper_dir,0);
					break;
			case	SMG$K_TRM_INSERT_HERE:
					set_file_selp(oper_dir);
					break;
			case	SMG$K_TRM_SELECT:
					un_sel_files(oper_dir,1);
					break;
			case	SMG$K_TRM_REMOVE:
					un_sel_files(oper_dir,0);
					break;
			case	SMG$K_TRM_FIND:
					find_files(oper_dir);
					break;
			case	SMG$K_TRM_CTRLB:
					fsm_unit++; fsm_unit%=2;
					disp_file_stat(0);
					disp_file_stat(1);
					break;
			case	SMG$K_TRM_CR:
					cr_key_handler(oper_dir);
					break;
			case	SMG$K_TRM_CTRLR:
					refresh_screen();
					break;
			case	SMG$K_TRM_CTRLW:
					rotate_dir_w();
					break;
			case	SMG$K_TRM_PF1:
					pf1_key_handler(oper_dir);
					break;
			case	SMG$K_TRM_F11:
					spawn_proc(oper_dir,0);
					break;
			case	SMG$K_TRM_F12:
					goto_dir(oper_dir);
					break;
			case	SMG$K_TRM_F13:
					view_file(oper_dir);
					break;
			case	SMG$K_TRM_F14:
					edit_file(oper_dir);
					break;
			case	SMG$K_TRM_HELP:
					copy_renmov_files(oper_dir,0);
					break;
			case	SMG$K_TRM_DO:
					copy_renmov_files(oper_dir,1);
					break;
			case	SMG$K_TRM_F17:
					make_dir(oper_dir);
					break;
			case	SMG$K_TRM_F18:
					delete_files(oper_dir);
					break;
			case	SMG$K_TRM_F19:
					fkey_desc(15);
					break;
			case	SMG$K_TRM_F20:
					end_session();
					break;
			default:
				comm_handler(oper_dir);
		}

		SMG$SET_CURSOR_ABS(&comm_w,&1,&comm_col);
	}
	while (1 == 1);
}

void pf1_key_handler(dir_sw)
int	dir_sw;
{
	SMG$READ_KEYSTROKE(&keyb_id,&key_pr);

	switch (key_pr)
	{
		case	SMG$K_TRM_PREV_SCREEN:
				prev_dir_page(dir_sw,1);
				break;
		case	SMG$K_TRM_NEXT_SCREEN:
				next_dir_page(dir_sw,1);
	}
}

int str_centr_pos(area_width,text_str)
char	*text_str;		/* \0-terminated string */
int	area_width;
{
	int	l;

	l=strlen(text_str);

	if (l > area_width) l=area_width;

	l=area_width-l;

	return 1+(l-l%2)/2;
}

void get_sysmsg(cond_code,msg_text)
char	*msg_text;
int	cond_code;
{
	short	l;

	char	msg_buff[256],
		msg_info[4];

	$DESCRIPTOR(msg_desc,msg_buff);

	SYS$GETMSG(cond_code,&l,&msg_desc,1,msg_info);

	msg_buff[0]=toupper(msg_buff[0]);
	msg_buff[l]='\0';

	strcpy(msg_text,msg_buff);
}

void invoke_tpu(tpu_comm_line)
char	*tpu_comm_line;
{
	int	tpu_ret_st,
		x_tpu_ret_st;

	char	tpu_mess_text[256];

	struct	dsc$descriptor_s tpu_comm_desc;

	cre_s_str_desc(tpu_comm_line); tpu_comm_desc=s_str_desc;

	x_tpu_ret_st=TPU$TPU(&tpu_comm_desc);

	tpu_ret_st=x_tpu_ret_st & 0x0FFFFFFF;

	INST_LAT_FONT;

	if ((tpu_ret_st != TPU$_SUCCESS) &&
		(tpu_ret_st != TPU$_EXITING) &&
		(tpu_ret_st != TPU$_QUITTING))
	{
		get_sysmsg(x_tpu_ret_st,tpu_mess_text);

		printf("\n%s\n",tpu_mess_text);
	}

	PAK_MESS;
}

void get_term_row_col()
{
	cre_s_str_desc("SYS$COMMAND");
	LIB$GETDVI(&DVI$_TT_PAGE,0,&s_str_desc,&prev_term_rq);
	LIB$GETDVI(&DVI$_DEVBUFSIZ,0,&s_str_desc,&prev_term_cq);

	term_row_qua=prev_term_rq; term_col_qua=prev_term_cq;

	if (term_row_qua < 24) term_row_qua=24;
	if (term_col_qua < 80) term_col_qua=80;

	term_row_qua=term_row_qua-(term_row_qua%2);
	term_col_qua=term_col_qua-(term_col_qua%2);

		/* Left panel, Dir window */

	cur_dir_w[0][0].row_qua=1;
	cur_dir_w[0][0].col_qua=(term_col_qua/2)-2;
	cur_dir_w[0][0].s_row=1;
	cur_dir_w[0][0].s_col=2;

		/* Left panel, Files spec window */

	cur_dir_w[0][1].row_qua=term_row_qua-6;
	cur_dir_w[0][1].col_qua=(term_col_qua/2)-2;
	cur_dir_w[0][1].s_row=2;
	cur_dir_w[0][1].s_col=2;

		/* Left panel, File info window */

	cur_dir_w[0][2].row_qua=1;
	cur_dir_w[0][2].col_qua=(term_col_qua/2)-2;
	cur_dir_w[0][2].s_row=term_row_qua-3;
	cur_dir_w[0][2].s_col=2;

		/* Right panel, Dir window */

	cur_dir_w[1][0].row_qua=1;
	cur_dir_w[1][0].col_qua=(term_col_qua/2)-2;
	cur_dir_w[1][0].s_row=1;
	cur_dir_w[1][0].s_col=(term_col_qua/2)+2;

		/* Right panel, Files spec window */

	cur_dir_w[1][1].row_qua=term_row_qua-6;
	cur_dir_w[1][1].col_qua=(term_col_qua/2)-2;
	cur_dir_w[1][1].s_row=2;
	cur_dir_w[1][1].s_col=(term_col_qua/2)+2;

		/* Right panel, File info window */

	cur_dir_w[1][2].row_qua=1;
	cur_dir_w[1][2].col_qua=(term_col_qua/2)-2;
	cur_dir_w[1][2].s_row=term_row_qua-3;
	cur_dir_w[1][2].s_col=(term_col_qua/2)+2;

		/* Top panel, Dir window */

	cur_dir_w[2][0].row_qua=1;
	cur_dir_w[2][0].col_qua=term_col_qua-2;
	cur_dir_w[2][0].s_row=1;
	cur_dir_w[2][0].s_col=2;

		/* Top panel, Files spec window */

	cur_dir_w[2][1].row_qua=((term_row_qua-2)/2)-4;
	cur_dir_w[2][1].col_qua=term_col_qua-2;
	cur_dir_w[2][1].s_row=2;
	cur_dir_w[2][1].s_col=2;

		/* Top panel, File info window */

	cur_dir_w[2][2].row_qua=1;
	cur_dir_w[2][2].col_qua=term_col_qua-2;
	cur_dir_w[2][2].s_row=((term_row_qua-2)/2)-1;
	cur_dir_w[2][2].s_col=2;

		/* Bottom panel, Dir window */

	cur_dir_w[3][0].row_qua=1;
	cur_dir_w[3][0].col_qua=term_col_qua-2;
	cur_dir_w[3][0].s_row=((term_row_qua-2)/2)+1;
	cur_dir_w[3][0].s_col=2;

		/* Bottom panel, Files spec window */

	cur_dir_w[3][1].row_qua=((term_row_qua-2)/2)-4;
	cur_dir_w[3][1].col_qua=term_col_qua-2;
	cur_dir_w[3][1].s_row=((term_row_qua-2)/2)+2;
	cur_dir_w[3][1].s_col=2;

		/* Bottom panel, File info window */

	cur_dir_w[3][2].row_qua=1;
	cur_dir_w[3][2].col_qua=term_col_qua-2;
	cur_dir_w[3][2].s_row=term_row_qua-3;
	cur_dir_w[3][2].s_col=2;
}

void set_term_row_col()
{
	short	term_chan,
		term_iosb[4];

	struct	term_chars
		{
		unsigned char	class_type[2];
		unsigned short	page_width;
		unsigned char	basic_chars[3];
		unsigned char	page_length;
		unsigned char	extend_chars[4];
		};

	struct	term_chars term_chars_buff;

	cre_s_str_desc("SYS$COMMAND");
	SYS$ASSIGN(&s_str_desc,&term_chan,0,0);

	SYS$QIOW(0,term_chan,IO$_SENSEMODE,&term_iosb,0,0,
			&term_chars_buff,sizeof(term_chars_buff),0,0,0,0);

	term_chars_buff.page_length=term_row_qua;
	term_chars_buff.page_width=term_col_qua;

	SYS$QIOW(0,term_chan,IO$_SETMODE,&term_iosb,0,0,
			&term_chars_buff,sizeof(term_chars_buff),0,0,0,0);

	SYS$DASSGN(term_chan);

	printf("\033[H\033[J\033[%d*|\033[%d$|",term_row_qua,term_col_qua);
}

void start_session()
{
	int	i,
		j,
		comm_w_sc,
		title_w,
		a_row,
		a_col;

	char	node_name[16];

	INST_LAT_FONT;

	get_term_row_col();
	set_term_row_col();

	for (i=0; i <= MAX_COMM_QUA-1; i++)
			spawn_comm[i].comm_str[0]='\0';

	SMG$CREATE_PASTEBOARD(&pstb_id);

	SMG$CREATE_VIRTUAL_KEYBOARD(&keyb_id);
	SMG$SET_KEYPAD_MODE(&keyb_id,&1);

	SMG$CREATE_VIRTUAL_DISPLAY(&term_row_qua,&term_col_qua,
			&main_w,&SMG$M_NOBORDER,&SMG$M_NORMAL);

	for (i=0; i <= 1; i++)
	{
	for (j=1; j <= 2; j++)
		SMG$CREATE_VIRTUAL_DISPLAY(&cur_dir_w[i][j].row_qua,
						&cur_dir_w[i][j].col_qua,
						&cur_dir_w[i][j].disp_id,
						&SMG$M_BORDER,&SMG$M_NORMAL);

	SMG$CREATE_VIRTUAL_DISPLAY(&cur_dir_w[i][0].row_qua,
					&cur_dir_w[i][0].col_qua,
					&cur_dir_w[i][0].disp_id,
					&SMG$M_NOBORDER,
					&(SMG$M_REVERSE|SMG$M_BOLD));
	}

	LIB$GETSYI(&SYI$_NODENAME,0,&d_str_desc);

	strncpy(node_name,d_str_desc.dsc$a_pointer,d_str_desc.dsc$w_length);
	node_name[d_str_desc.dsc$w_length]='\0';

	strcpy(comm_prompt,vtfm_bgt[68]);
	strcat(comm_prompt,node_name);
	strcat(comm_prompt,"::");
	strcat(comm_prompt,cuserid(NULL));
	strcat(comm_prompt,"> ");
	comm_w_cq=term_col_qua-strlen(comm_prompt);
	comm_w_sc=term_col_qua-comm_w_cq+1;

	SMG$CREATE_VIRTUAL_DISPLAY(&1,&comm_w_cq,
				&comm_w,&SMG$M_NOBORDER,&SMG$M_REVERSE);

	SMG$CREATE_VIRTUAL_DISPLAY(&8,&36,&title_w,
				&SMG$M_BORDER,&SMG$M_REVERSE);

	for (i=0; i <= 7; i++)
	{
		cre_s_str_desc(vtfm_bgt[i]);
		SMG$PUT_LINE(&title_w,&s_str_desc);
	}

	disp_w_label(title_w,vtfm_bgt[8],SMG$K_BOTTOM,2);

	SMG$PASTE_VIRTUAL_DISPLAY(&main_w,&pstb_id,&1,&1);

	a_row=(term_row_qua-8)/2;
	a_col=(term_col_qua-36)/2+1;

	SMG$PASTE_VIRTUAL_DISPLAY(&title_w,&pstb_id,&a_row,&a_col);

	cursor_to_br();

	SMG$READ_KEYSTROKE(&keyb_id,&key_pr);

	disp_w_label(title_w,vtfm_bgt[9],SMG$K_BOTTOM,2);

	cursor_to_br();

	strcpy(cur_dir_parms[0].dir_spec,getenv("PATH"));
	str_toupper(cur_dir_parms[0].dir_spec);
	chg_dir_brack_comma(cur_dir_parms[0].dir_spec);

	strcpy(cur_dir_parms[0].find_spec,vtfm_bgt[47]);
	strcpy(cur_dir_parms[0].sel_spec,vtfm_bgt[47]);

	set_def_dir(cur_dir_parms[0].dir_spec);
	read_dir(0);

	cur_dir_parms[0].dir_cpos=0;

	cur_dir_parms[1]=cur_dir_parms[0];

	for (i=0; i <= cur_dir_parms[0].dir_fqua; i++)
			cur_dir_cont[1][i]=cur_dir_cont[0][i];

	disp_dir_cont(0,1);
	disp_dir_cont(1,1);

	SMG$ERASE_PASTEBOARD(&pstb_id);

	SMG$DELETE_VIRTUAL_DISPLAY(&title_w);

	SMG$UNPASTE_VIRTUAL_DISPLAY(&main_w,&pstb_id);

	a_row=term_row_qua-1;

	SMG$SET_CURSOR_ABS(&main_w,&a_row,&1);
	cre_s_str_desc(comm_prompt);
	SMG$PUT_LINE(&main_w,&s_str_desc);

	SMG$CHANGE_RENDITION(&main_w,&a_row,&1,&1,&term_col_qua,
				&SMG$M_NORMAL,&SMG$M_REVERSE);

	a_col=((term_col_qua-80)/2)+1;

	SMG$CHANGE_RENDITION(&main_w,&term_row_qua,&a_col,&1,&80,
				&SMG$M_NORMAL,&SMG$M_REVERSE);

	for (i=27; i <= 45; i=i+2)
	{
		SMG$SET_CURSOR_ABS(&main_w,&term_row_qua,&a_col);
		cre_s_str_desc(vtfm_bgt[i]);
		SMG$PUT_LINE(&main_w,&s_str_desc,
				&LINE_ADV,&SMG$M_NORMAL,&SMG$M_REVERSE);
		a_col=a_col+strlen(vtfm_bgt[i]);
		SMG$SET_CURSOR_ABS(&main_w,&term_row_qua,&a_col);
		cre_s_str_desc(vtfm_bgt[i+1]);
		SMG$PUT_LINE(&main_w,&s_str_desc);
		a_col=a_col+strlen(vtfm_bgt[i+1]);
	}

	SMG$PASTE_VIRTUAL_DISPLAY(&main_w,&pstb_id,&1,&1);

	SMG$PASTE_VIRTUAL_DISPLAY(&comm_w,&pstb_id,&a_row,&comm_w_sc);

	paste_dir_w();
	file_bar_on(0);

	SMG$SET_CURSOR_ABS(&comm_w,&1,&comm_col);
}

void end_session()
{
	int	cok_disp_id;

	cok_handler(&cok_disp_id,vtfm_bgt[59],
			vtfm_bgt[60],(term_row_qua-6)/2,
			str_centr_pos(term_col_qua,vtfm_bgt[60])-1,1);

	SMG$DELETE_VIRTUAL_DISPLAY(&cok_disp_id);

	if (key_pr == SMG$K_TRM_CR)
	{
		SMG$DELETE_PASTEBOARD(&pstb_id);
		SMG$DELETE_VIRTUAL_KEYBOARD(&keyb_id);

		term_row_qua=prev_term_rq; term_col_qua=prev_term_cq;

		set_term_row_col();

		exit(0);
	}
}

void set_def_dir(dir_spec)
char	*dir_spec;
{
	int	i;

	char	*dir_name,
		dev_name[256];

	$DESCRIPTOR(sys_disk,"SYS$DISK");

	dir_name=strchr(dir_spec,'[');
	i=(int)dir_name-(int)dir_spec;
	strncpy(dev_name,dir_spec,i);
	dev_name[i]='\0';

	cre_s_str_desc(dev_name);
	LIB$SET_LOGICAL(&sys_disk,&s_str_desc);

	cre_s_str_desc(dir_name);
	SYS$SETDDIR(&s_str_desc,0,0);
}

void chg_oper_dir()
{
	file_bar_off(oper_dir);
	oper_dir++; oper_dir%=2;
	set_def_dir(cur_dir_parms[oper_dir].dir_spec);
	file_bar_on(oper_dir);
}

void cursor_to_br()
{
	SMG$SET_CURSOR_ABS(&main_w,&term_row_qua,&term_col_qua);
}

void ring_bell(bell_cnt)
int	bell_cnt;
{
	SMG$RING_BELL(&main_w,&bell_cnt);
}

void paste_dir_w()
{
	int	i,
		j;

	for (i=0; i <= 1; i++)
	{
	for (j=1; j <= 2; j++)
		SMG$PASTE_VIRTUAL_DISPLAY(&cur_dir_w[i][j].disp_id,
					&pstb_id,
					&cur_dir_w[i][j].s_row,
					&cur_dir_w[i][j].s_col);

	SMG$PASTE_VIRTUAL_DISPLAY(&cur_dir_w[i][0].disp_id,
					&pstb_id,
					&cur_dir_w[i][0].s_row,
					&cur_dir_w[i][0].s_col);
	}
}

void disp_w_label(w_disp_id,text_str,at_top_bot,label_sc)
char	*text_str;
int	w_disp_id,
	at_top_bot,
	label_sc;
{
	cre_s_str_desc(text_str);
	SMG$LABEL_BORDER(&w_disp_id,&s_str_desc,&at_top_bot,
				&label_sc,&SMG$M_NORMAL,&SMG$M_REVERSE);
}

void chg_dir_brack_comma(dir_spec)
char	dir_spec[];
{
	int	i=0;

	while (dir_spec[i] != '\0')
	{
		if (dir_spec[i] == '<')
			dir_spec[i]='[';
		else
			if (dir_spec[i] == '>')
				dir_spec[i]=']';
		else
			if (dir_spec[i] == ',')
				dir_spec[i]='.';
		i++;
	}
}

void str_tolower(any_string)
char	any_string[];	/* \0-terminated string */
{
	int	i=0;

	while (any_string[i] != '\0')
	{
		any_string[i]=tolower(any_string[i]);
		i++;
	}
}

void str_toupper(any_string)
char	any_string[];	/* \0-terminated string */
{
	int	i=0;

	while (any_string[i] != '\0')
	{
		any_string[i]=toupper(any_string[i]);
		i++;
	}
}

void cre_s_str_desc(any_string)
char	*any_string;	/* \0-terminated string */
{
	s_str_desc.dsc$w_length=strlen(any_string);
	s_str_desc.dsc$a_pointer=any_string;
}

void read_dir(dir_sw)
int	dir_sw;
{
	int	i,
		rms_ret_st,
		stat_ret_st,
		str_ret_st,
		c_text=0,
		dir_rtp=0,
		dir_fqua=(-1),
		ord_fqua=0;

	char	in_file_spec[256],
		ou_file_spec[256],
		find_spec[64],
		*ou_fsp,
		*s_fext,
		file_ext[64];

	struct	dsc$descriptor_s all_s_str_desc;
	struct	dsc$descriptor_s find_s_str_desc;

	cur_dir_parms[dir_sw].sel_fqua=0;
	cur_dir_cont[dir_sw][0].file_selp=' ';

	ou_fsp=strchr(cur_dir_parms[dir_sw].dir_spec,'[');

	if (strchr(ou_fsp,'.') == NULL)
						/* Root Directory */
	{
		cur_dir_cont[dir_sw][0].file_type=(-3);
		cur_dir_cont[dir_sw][0].file_spec[0]='\0';
		dir_rtp=1;
	}
	else					/* Not Root Directory */
	{
		cur_dir_cont[dir_sw][0].file_type=(-2);
		strcpy(cur_dir_cont[dir_sw][0].file_spec,"..");

						/* Create UP--DIR file spec */

		strcpy(ou_file_spec,cur_dir_parms[dir_sw].dir_spec);
		s_fext=strrchr(ou_file_spec,'.');
		strncpy(s_fext,"]",1);
		ou_file_spec[strlen(cur_dir_parms[dir_sw].dir_spec)-1]='\0';
		strcat(ou_file_spec,vtfm_bgt[26]);

							/* Get UP--DIR date */

		stat_ret_st=stat(ou_file_spec,&file_info);

		if (stat_ret_st == 0)
		{
			cur_dir_cont[dir_sw][0].file_type=(-1);
			cur_dir_cont[dir_sw][0].file_date=file_info.st_ctime;
		}
		dir_fqua=0;
	}

	i=strlen(cur_dir_parms[dir_sw].dir_spec);

	strcat(strcpy(in_file_spec,
			cur_dir_parms[dir_sw].dir_spec),vtfm_bgt[47]);
	cre_s_str_desc(in_file_spec);
	all_s_str_desc=s_str_desc;

	strcpy(find_spec,cur_dir_parms[dir_sw].find_spec);
	str_toupper(find_spec);
	cre_s_str_desc(find_spec);
	find_s_str_desc=s_str_desc;

	do
	{
		rms_ret_st=LIB$FIND_FILE(&all_s_str_desc,
						&d_str_desc,&c_text);

		if (rms_ret_st == RMS$_NORMAL)
		{
			strncpy(ou_file_spec,
				d_str_desc.dsc$a_pointer,
				d_str_desc.dsc$w_length);
			ou_file_spec[d_str_desc.dsc$w_length]='\0';
			ou_fsp=strchr(ou_file_spec,']')+1;

			cre_s_str_desc(ou_fsp);

			str_ret_st=STR$MATCH_WILD(&s_str_desc,
							&find_s_str_desc);
			if (str_ret_st == STR$_MATCH)
			{
				extr_file_ext(ou_fsp,file_ext);

			if (strcmp(file_ext,vtfm_bgt[26]) == 0)
			{
				dir_fqua++;

			strcpy(cur_dir_cont[dir_sw][dir_fqua].file_spec,
								ou_fsp);
			cur_dir_cont[dir_sw][dir_fqua].file_type=0;
			cur_dir_cont[dir_sw][dir_fqua].file_selp=' ';
			}
			else
			{
				ord_fqua++;

				str_tolower(ou_fsp);

			strcpy(ord_dir_file[ord_fqua].file_spec,
								ou_fsp);
			ord_dir_file[ord_fqua].file_type=0;
			ord_dir_file[ord_fqua].file_selp=' ';
			}
			}
		}
		else if (rms_ret_st != RMS$_NMF)
		{
		switch (rms_ret_st)
		{
			case	RMS$_DEV:
				strcat(cur_dir_cont[dir_sw][0].file_spec,
							vtfm_bgt[10]);
					break;
			case	RMS$_DIR:
			case	RMS$_SYN:
				strcat(cur_dir_cont[dir_sw][0].file_spec,
							vtfm_bgt[53]);
					break;
			case	RMS$_DNF:
				strcat(cur_dir_cont[dir_sw][0].file_spec,
							vtfm_bgt[12]);
					break;
			case	RMS$_DNR:
				strcat(cur_dir_cont[dir_sw][0].file_spec,
							vtfm_bgt[13]);
					break;
			case	RMS$_FND:
				strcat(cur_dir_cont[dir_sw][0].file_spec,
							vtfm_bgt[14]);
					break;
			case	RMS$_FNF:
				strcat(cur_dir_cont[dir_sw][0].file_spec,
							vtfm_bgt[15]);
					break;
			case	RMS$_LNE:
				strcat(cur_dir_cont[dir_sw][0].file_spec,
							vtfm_bgt[48]);
					break;
			case	RMS$_PRV:
				strcat(cur_dir_cont[dir_sw][0].file_spec,
							vtfm_bgt[16]);
					break;
			default:
				sprintf(cur_dir_cont[dir_sw][0].file_spec,
						vtfm_bgt[17],rms_ret_st);
		}
			LIB$FIND_FILE_END(&c_text);

			cur_dir_parms[dir_sw].dir_fqua=
			cur_dir_parms[dir_sw].ord_fqua=0;

			return;
		}
	}
	while ((rms_ret_st != RMS$_NMF) &&
		((dir_fqua+ord_fqua) < (MAX_DIR_FQUA-dir_rtp-1)));

	LIB$FIND_FILE_END(&c_text);

	if (dir_fqua+dir_rtp+ord_fqua == 0)
	{
		strcat(cur_dir_cont[dir_sw][0].file_spec,vtfm_bgt[15]);
		cur_dir_parms[dir_sw].dir_fqua=
		cur_dir_parms[dir_sw].ord_fqua=0;

		return;
	}

	if (ord_fqua > 0)
		for (i=1; i <= ord_fqua; i++)
		{
			dir_fqua++;
			cur_dir_cont[dir_sw][dir_fqua]=ord_dir_file[i];
		}

	cur_dir_parms[dir_sw].dir_fqua=dir_fqua;
	cur_dir_parms[dir_sw].ord_fqua=ord_fqua;
}

void disp_ext_fspec(dir_sw,dir_cpos)
int	dir_sw,
	dir_cpos;
{
	ext_file_spec[0]=cur_dir_cont[dir_sw][dir_cpos].file_selp;
	ext_file_spec[1]='\0';
	strcat(ext_file_spec,cur_dir_cont[dir_sw][dir_cpos].file_spec);

	cre_s_str_desc(ext_file_spec);
	SMG$PUT_LINE(&cur_dir_w[dir_sw][1].disp_id,&s_str_desc);
}

void disp_dir_spec(dir_sw)
int	dir_sw;
{
	char	full_spec[256];

	strcat(strcpy(full_spec,
		cur_dir_parms[dir_sw].dir_spec),
			cur_dir_parms[dir_sw].find_spec);

	str_toupper(full_spec);

	cre_s_str_desc(full_spec);
	SMG$PUT_LINE(&cur_dir_w[dir_sw][0].disp_id,&s_str_desc);
}

void disp_dir_cont(dir_sw,disp_dsp_sw)
int	dir_sw,
	disp_dsp_sw;
{
	int	i,
		xpos,
		disp_fqua,
		a_row=1,
		a_col=1;

	cur_dir_parms[dir_sw].dir_wpos=0;

	do
	{
		cur_dir_parms[dir_sw].dir_wpos++;
		xpos=cur_dir_parms[dir_sw].dir_cpos-
				cur_dir_parms[dir_sw].dir_wpos+1;
	}
	while ((xpos > 0) &&
		(cur_dir_parms[dir_sw].dir_wpos <
			 cur_dir_w[dir_sw][1].row_qua));

	if (cur_dir_parms[dir_sw].dir_fqua <
			cur_dir_w[dir_sw][1].row_qua-1)
		disp_fqua=cur_dir_parms[dir_sw].dir_fqua+1;
	else
		disp_fqua=cur_dir_w[dir_sw][1].row_qua;

	if (disp_dsp_sw) disp_dir_spec(dir_sw);

	SMG$SET_CURSOR_ABS(&cur_dir_w[dir_sw][1].disp_id,&a_row,&a_col);

	for (i=1; i <= disp_fqua; i++)
			disp_ext_fspec(dir_sw,xpos+i-1);

	disp_file_stat(dir_sw);
}

void file_bar_on(dir_sw)
int	dir_sw;
{
	int	a_row=1,
		a_col=1;

	SMG$SET_CURSOR_ABS(&cur_dir_w[dir_sw][1].disp_id,
				&cur_dir_parms[dir_sw].dir_wpos,&a_col);

	SMG$CHANGE_RENDITION(&cur_dir_w[dir_sw][1].disp_id,
				&cur_dir_parms[dir_sw].dir_wpos,&a_col,
				&a_row,&cur_dir_w[dir_sw][1].col_qua,
				&SMG$M_NORMAL,&SMG$M_REVERSE);
}

void file_bar_off(dir_sw)
int	dir_sw;
{
	int	a_row=1,
		a_col=1;

	SMG$CHANGE_RENDITION(&cur_dir_w[dir_sw][1].disp_id,
				&cur_dir_parms[dir_sw].dir_wpos,&a_col,
				&a_row,&cur_dir_w[dir_sw][1].col_qua);
}

void prev_dir_file(dir_sw)
int	dir_sw;
{
	int	i,
		a_row=1,
		a_col=1;

	if (cur_dir_parms[dir_sw].dir_wpos > 1)
	{
		file_bar_off(dir_sw);
		cur_dir_parms[dir_sw].dir_cpos--;
		cur_dir_parms[dir_sw].dir_wpos--;
		disp_file_stat(dir_sw);
		file_bar_on(dir_sw);
	}
	else
	{
		if (cur_dir_parms[dir_sw].dir_cpos)
		{
			file_bar_off(dir_sw);
			cur_dir_parms[dir_sw].dir_cpos--;
			i=cur_dir_parms[dir_sw].dir_cpos;

			SMG$SCROLL_DISPLAY_AREA(&cur_dir_w[dir_sw][1].disp_id,
						&a_row,&a_col,
						&cur_dir_w[dir_sw][1].row_qua,
						&cur_dir_w[dir_sw][1].col_qua,
							&SMG$M_DOWN);
			disp_ext_fspec(dir_sw,i);
			disp_file_stat(dir_sw);
			file_bar_on(dir_sw);
		}
		else
			ring_bell(1);
	}
}

void next_dir_file(dir_sw)
int	dir_sw;
{
	int	i,
		a_row,
		a_col=1;

	a_row=cur_dir_w[dir_sw][1].row_qua;

	if (cur_dir_parms[dir_sw].dir_cpos <
			cur_dir_parms[dir_sw].dir_fqua)
	{
		if (cur_dir_parms[dir_sw].dir_wpos < a_row)
		{
			file_bar_off(dir_sw);
			cur_dir_parms[dir_sw].dir_cpos++;
			cur_dir_parms[dir_sw].dir_wpos++;
			disp_file_stat(dir_sw);
			file_bar_on(dir_sw);
		}
		else
		{
			file_bar_off(dir_sw);
			cur_dir_parms[dir_sw].dir_cpos++;
			i=cur_dir_parms[dir_sw].dir_cpos;

			SMG$SCROLL_DISPLAY_AREA(&cur_dir_w[dir_sw][1].disp_id);

			SMG$SET_CURSOR_ABS(&cur_dir_w[dir_sw][1].disp_id,
							&a_row,&a_col);
			disp_ext_fspec(dir_sw,i);
			disp_file_stat(dir_sw);
			file_bar_on(dir_sw);
		}
	}
	else
		ring_bell(1);
}

void disp_file_stat(dir_sw)
int	dir_sw;
{
		 char	fsize_str[512],
			fstat_str[512],
			fspec[256],
			*fs_beg;

		 int	i,
			j,
			k,
			l,
			stat_ret_st;

	unsigned int	fsize;

	i=cur_dir_parms[dir_sw].dir_cpos;
	k=cur_dir_w[dir_sw][2].col_qua;

	sprintf(fstat_str,"%*s",k," ");

	if (cur_dir_cont[dir_sw][i].file_type > (-2))
	{
	if (cur_dir_cont[dir_sw][i].file_type == 0)
	{
		strcpy(fspec,cur_dir_parms[dir_sw].dir_spec);
		strcat(fspec,cur_dir_cont[dir_sw][i].file_spec);

		stat_ret_st=stat(fspec,&file_info);

	if (stat_ret_st == 0)
	{
		cur_dir_cont[dir_sw][i].file_date=file_info.st_ctime;

		if (file_info.st_mode & S_IFDIR)
			cur_dir_cont[dir_sw][i].file_type=2;
		else
		{
			cur_dir_cont[dir_sw][i].file_type=1;
			cur_dir_cont[dir_sw][i].file_size=file_info.st_size;
		}
	}
	}
	if (cur_dir_cont[dir_sw][i].file_type)
	{
		file_ctime=localtime(&cur_dir_cont[dir_sw][i].file_date);

		k=k-17;

	sprintf(fstat_str,"%*d-%02d-%04d %02d:%02d:%02d",k,
				file_ctime->tm_mday,
				file_ctime->tm_mon+1,
				file_ctime->tm_year+1900,
				file_ctime->tm_hour,
				file_ctime->tm_min,
				file_ctime->tm_sec);

		k=k-2;
	}
	}

	switch (cur_dir_cont[dir_sw][i].file_type)
	{
		case	(-3):
				strcpy(fsize_str,vtfm_bgt[25]);
				break;
		case	(-2):
		case	(-1):
				strcpy(fsize_str,vtfm_bgt[19]);
				break;
		case	0:
				strcpy(fsize_str,vtfm_bgt[18]);
				break;
		case	1:
				fsize=cur_dir_cont[dir_sw][i].file_size;

				if (fsm_unit)
				{
					j=fsize; fsize=fsize/512; j%=512;
					if (j) fsize++;
					sprintf(fsize_str,"%u %s",
							fsize,vtfm_bgt[55]);
				}
				else
					sprintf(fsize_str,"%u %s",
							fsize,vtfm_bgt[57]);

				if (fsize != 1) strcat(fsize_str,"s");

				break;
		case	2:
				strcpy(fsize_str,vtfm_bgt[20]);

	}

	l=strlen(fsize_str); k=k-l; j=k; k=k/2; j%=2;
	fs_beg=fstat_str+k+j;
	strncpy(fs_beg,fsize_str,l);

	cre_s_str_desc(fstat_str);
	SMG$PUT_LINE(&cur_dir_w[dir_sw][2].disp_id,&s_str_desc);
}

void set_file_selp(dir_sw)
int	dir_sw;
{
	int	i,
		a_col=1;

	char	selp_char[2]={'\0','\0'};

	i=cur_dir_parms[dir_sw].dir_cpos;

	if (cur_dir_cont[dir_sw][i].file_type >= 0)
	{
		if (cur_dir_cont[dir_sw][i].file_selp == ' ')
		{
			cur_dir_cont[dir_sw][i].file_selp='*';
			cur_dir_parms[dir_sw].sel_fqua++;
		}
		else
		{
			cur_dir_cont[dir_sw][i].file_selp=' ';
			cur_dir_parms[dir_sw].sel_fqua--;
		}

		selp_char[0]=cur_dir_cont[dir_sw][i].file_selp;

		cre_s_str_desc(selp_char);
		SMG$PUT_CHARS(&cur_dir_w[dir_sw][1].disp_id,&s_str_desc,
				&cur_dir_parms[dir_sw].dir_wpos,&a_col,
				&ERASE_FLAG,&SMG$M_NORMAL,&SMG$M_REVERSE);

		next_dir_file(dir_sw);
	}
	else
		ring_bell(1);
}

void prev_dir_page(dir_sw,pf1_sw)
int	dir_sw,
	pf1_sw;
{
	int	i,
		j,
		scr_cnt,
		xpos,
		ypos,
		a_row=1,
		a_col=1;

	if (cur_dir_parms[dir_sw].dir_cpos >=
			cur_dir_parms[dir_sw].dir_wpos)
	{
		file_bar_off(dir_sw);

		scr_cnt=0;

		do
		{
			scr_cnt++;
			xpos=cur_dir_parms[dir_sw].dir_cpos-
				cur_dir_parms[dir_sw].dir_wpos+1-scr_cnt;
		}
		while ((xpos > 0) && (scr_cnt <
				cur_dir_w[dir_sw][1].row_qua));

		SMG$SCROLL_DISPLAY_AREA(&cur_dir_w[dir_sw][1].disp_id,
						&a_row,&a_col,
						&cur_dir_w[dir_sw][1].row_qua,
						&cur_dir_w[dir_sw][1].col_qua,
						&SMG$M_DOWN,&scr_cnt);

		SMG$SET_CURSOR_ABS(&cur_dir_w[dir_sw][1].disp_id,
							&a_row,&a_col);
	pf1_sw++; pf1_sw%=2;

	for (i=1; i <= scr_cnt; i++)
			disp_ext_fspec(dir_sw,xpos*pf1_sw+i-1);

	pf1_sw++; pf1_sw%=2;

	if (pf1_sw)
	{
		cur_dir_parms[dir_sw].dir_wpos=1;
		cur_dir_parms[dir_sw].dir_cpos=0;
		disp_file_stat(dir_sw);
	}
	else
	{
		ypos=xpos+cur_dir_w[dir_sw][1].row_qua-1;

		if (ypos < cur_dir_parms[dir_sw].dir_cpos)
		{
			cur_dir_parms[dir_sw].dir_wpos=
					cur_dir_w[dir_sw][1].row_qua;
			cur_dir_parms[dir_sw].dir_cpos=ypos;
			disp_file_stat(dir_sw);
		}
		else cur_dir_parms[dir_sw].dir_wpos=
				cur_dir_parms[dir_sw].dir_wpos+scr_cnt;
	}
		file_bar_on(dir_sw);
	}
	else if (cur_dir_parms[dir_sw].dir_cpos > 0)
	{
		file_bar_off(dir_sw);
		cur_dir_parms[dir_sw].dir_wpos=1;
		cur_dir_parms[dir_sw].dir_cpos=0;
		disp_file_stat(dir_sw);
		file_bar_on(dir_sw);
	}
	else
		ring_bell(1);
}

void next_dir_page(dir_sw,pf1_sw)
int	dir_sw,
	pf1_sw;
{
	int	i,
		j,
		scr_cnt,
		xpos,
		ypos,
		a_row=1,
		a_col=1;

	if (cur_dir_parms[dir_sw].dir_fqua >
		cur_dir_parms[dir_sw].dir_cpos+
			cur_dir_w[dir_sw][1].row_qua-
				cur_dir_parms[dir_sw].dir_wpos)
	{
		file_bar_off(dir_sw);

		scr_cnt=0;

		do
		{
			scr_cnt++;
			xpos=cur_dir_parms[dir_sw].dir_cpos+
				cur_dir_w[dir_sw][1].row_qua-
				cur_dir_parms[dir_sw].dir_wpos+scr_cnt;
		}
		while ((xpos < cur_dir_parms[dir_sw].dir_fqua) &&
				(scr_cnt < cur_dir_w[dir_sw][1].row_qua));

		SMG$SCROLL_DISPLAY_AREA(&cur_dir_w[dir_sw][1].disp_id,
						&a_row,&a_col,
						&cur_dir_w[dir_sw][1].row_qua,
						&cur_dir_w[dir_sw][1].col_qua,
						&SMG$M_UP,&scr_cnt);

		a_row=cur_dir_w[dir_sw][1].row_qua-scr_cnt+1;

		SMG$SET_CURSOR_ABS(&cur_dir_w[dir_sw][1].disp_id,
							&a_row,&a_col);

	for (i=scr_cnt; i >= 1; i--) disp_ext_fspec(dir_sw,
		xpos+(cur_dir_parms[dir_sw].dir_fqua-xpos)*pf1_sw-i+1);

	if (pf1_sw)
	{
		cur_dir_parms[dir_sw].dir_wpos=
					cur_dir_w[dir_sw][1].row_qua;
		cur_dir_parms[dir_sw].dir_cpos=
					cur_dir_parms[dir_sw].dir_fqua;
		disp_file_stat(dir_sw);
	}
	else
	{
		ypos=xpos-cur_dir_w[dir_sw][1].row_qua+1;

		if (ypos > cur_dir_parms[dir_sw].dir_cpos)
		{
			cur_dir_parms[dir_sw].dir_wpos=1;
			cur_dir_parms[dir_sw].dir_cpos=ypos;
			disp_file_stat(dir_sw);
		}
		else cur_dir_parms[dir_sw].dir_wpos=
				cur_dir_parms[dir_sw].dir_wpos-scr_cnt;
	}
		file_bar_on(dir_sw);
	}
	else if (cur_dir_parms[dir_sw].dir_cpos <
					cur_dir_parms[dir_sw].dir_fqua)
	{
		file_bar_off(dir_sw);
		cur_dir_parms[dir_sw].dir_wpos=
					cur_dir_parms[dir_sw].dir_fqua-
					cur_dir_parms[dir_sw].dir_cpos+
					cur_dir_parms[dir_sw].dir_wpos;
		cur_dir_parms[dir_sw].dir_cpos=
					cur_dir_parms[dir_sw].dir_fqua;
		disp_file_stat(dir_sw);
		file_bar_on(dir_sw);
	}
	else
		ring_bell(1);
}

void erase_dir_w(dir_sw)
int	dir_sw;
{
	int	i;

	for (i=1; i <= 2; i++)
		SMG$ERASE_DISPLAY(&cur_dir_w[dir_sw][i].disp_id);
}

void cr_key_handler(dir_sw)
int	dir_sw;
{
	int	i,
		l,
		file_type,
		a_row=1,
		a_col=1;

	char	*fspec,
		file_spec[256],
		file_ext[64];

	i=(-1);

	do
		i++;
	while (spawn_comm[0].comm_str[i] == ' ');

	if (spawn_comm[0].comm_str[i])
			spawn_proc(dir_sw,1);
	else
	{
		i=cur_dir_parms[dir_sw].dir_cpos;
		file_type=cur_dir_cont[dir_sw][i].file_type;

	switch (file_type)
	{
	case	(-2):
	case	(-1):
	case	   2:
			if (file_type != 2)
		{
			fspec=strrchr(cur_dir_parms[dir_sw].dir_spec,'.')+1;
			strcpy(file_spec,fspec);
			fspec--; strcpy(fspec,"]");
			fspec=strchr(file_spec,']'); *fspec='\0';
		}
		else
		{
			l=strlen(cur_dir_parms[dir_sw].dir_spec);
			cur_dir_parms[dir_sw].dir_spec[l-1]='.';
			strcpy(strchr(cur_dir_cont[dir_sw][i].file_spec,
								'.'),"]");
			strcat(cur_dir_parms[dir_sw].dir_spec,
					cur_dir_cont[dir_sw][i].file_spec);
			str_toupper(cur_dir_parms[dir_sw].dir_spec);
			strcpy(file_spec,"\001");
		}
			set_def_dir(cur_dir_parms[dir_sw].dir_spec);
			strcpy(cur_dir_parms[dir_sw].find_spec,vtfm_bgt[47]);
			cur_dir_parms[dir_sw].sel_fqua=0;
			strcat(file_spec,vtfm_bgt[49]);
			before_restore(dir_sw);
			restore_dir_cont(dir_sw,file_spec);
				return;
	case	1:
		extr_file_ext(cur_dir_cont[dir_sw][i].file_spec,file_ext);

		if (strcmp(file_ext,vtfm_bgt[50]) == 0)
				strcpy(spawn_comm[0].comm_str,vtfm_bgt[52]);
		else
			if (strcmp(file_ext,vtfm_bgt[51]) == 0)
					strcpy(spawn_comm[0].comm_str,"@");
		else
			break;

		strcat(spawn_comm[0].comm_str,
				cur_dir_parms[dir_sw].dir_spec);
		strcat(spawn_comm[0].comm_str,
				cur_dir_cont[dir_sw][i].file_spec);
		spawn_proc(dir_sw,1);
			return;
	}
		ring_bell(1);
	}
}

void refresh_screen()
{
	int	save_disp_id,
		curs_disp_id,
		x_row,
		x_col;

	SMG$FIND_CURSOR_DISPLAY(&pstb_id,&curs_disp_id);

	SMG$RETURN_CURSOR_POS(&curs_disp_id,&x_row,&x_col);

	SMG$ERASE_PASTEBOARD(&pstb_id);

	SMG$SAVE_PHYSICAL_SCREEN(&pstb_id,&save_disp_id);

	SMG$RESTORE_PHYSICAL_SCREEN(&pstb_id,&save_disp_id);

	SMG$DELETE_VIRTUAL_DISPLAY(&save_disp_id);

	SMG$SET_CURSOR_ABS(&curs_disp_id,&x_row,&x_col);
}

void rotate_dir_w()
{
	int	i,
		j,
		k;

		file_bar_off(oper_dir);

		for (i=0; i <= 1; i++)
			for (j=0; j <= 2; j++)
		SMG$UNPASTE_VIRTUAL_DISPLAY(&cur_dir_w[i][j].disp_id,
								&pstb_id);

		SMG$SET_CURSOR_ABS(&comm_w,&1,&comm_col);

		for (i=0; i <= 1; i++)
			for (j=0; j <= 2; j++)
		{
			k=cur_dir_w[i][j].row_qua;
				cur_dir_w[i][j].row_qua=
					cur_dir_w[i+2][j].row_qua;
				cur_dir_w[i+2][j].row_qua=k;
			k=cur_dir_w[i][j].col_qua;
				cur_dir_w[i][j].col_qua=
					cur_dir_w[i+2][j].col_qua;
				cur_dir_w[i+2][j].col_qua=k;
			k=cur_dir_w[i][j].s_row;
				cur_dir_w[i][j].s_row=
					cur_dir_w[i+2][j].s_row;
				cur_dir_w[i+2][j].s_row=k;
			k=cur_dir_w[i][j].s_col;
				cur_dir_w[i][j].s_col=
					cur_dir_w[i+2][j].s_col;
				cur_dir_w[i+2][j].s_col=k;
		}

	for (i=0; i <= 1; i++)
	{
	for (j=1; j <= 2; j++)
		SMG$CHANGE_VIRTUAL_DISPLAY(&cur_dir_w[i][j].disp_id,
						&cur_dir_w[i][j].row_qua,
						&cur_dir_w[i][j].col_qua,
						&SMG$M_BORDER,&SMG$M_NORMAL);

	SMG$CHANGE_VIRTUAL_DISPLAY(&cur_dir_w[i][0].disp_id,
					&cur_dir_w[i][0].row_qua,
					&cur_dir_w[i][0].col_qua,
					&SMG$M_NOBORDER,
					&(SMG$M_REVERSE|SMG$M_BOLD));
	}

		disp_dir_cont(0,1);
		disp_dir_cont(1,1);

		paste_dir_w();

		file_bar_on(oper_dir);
}

void extr_file_ext(fspec,fext)
char	*fspec,
	fext[];
{
	int	l;

	char	*fext_p;

	fext_p=strchr(fspec,'.');
	l=(int)strchr(fext_p,';')-(int)fext_p;
	strncpy(fext,fext_p,l);
	fext[l]='\0';
}

void find_dir_cpos(dir_sw,fspec)
char	*fspec;
int	dir_sw;
{
	int	i,
		j,
		s_cpos=(-1),
		f_cpos,
		fver_nbr,
		fver_nbr_i;

	char	file_name[96],
		fname[96],
		file_ext[64],
		*fver_str;

	cur_dir_parms[dir_sw].dir_cpos=0;

	if (cur_dir_parms[dir_sw].dir_fqua)
	{
		i=cur_dir_parms[dir_sw].dir_fqua-
				cur_dir_parms[dir_sw].ord_fqua+1;

		extr_file_ext(fspec,file_ext);

		if (strcmp(file_ext,vtfm_bgt[26]) == 0)
		{
			if (cur_dir_cont[dir_sw][0].file_type)
			{
				i--;

				if (i)
				{
					s_cpos=1;
					f_cpos=i;
				}
			}
			else if (i)
			{
				s_cpos=0;
				f_cpos=i-1;
			}
		}
		else
		{
			if (cur_dir_parms[dir_sw].ord_fqua)
			{
				s_cpos=i;
				f_cpos=cur_dir_parms[dir_sw].dir_fqua;
			}
			else
				cur_dir_parms[dir_sw].dir_cpos=
					cur_dir_parms[dir_sw].dir_fqua;
		}

		if (s_cpos < 0) return;

		strcpy(file_name,fspec); str_toupper(file_name);
		fver_str=strchr(file_name,';'); *fver_str='\0';
		fver_str++; fver_nbr=atoi(fver_str);

		while (s_cpos < f_cpos)
		{
			i=f_cpos-s_cpos+1; j=i; i=i/2; j%=2;
			i=s_cpos-1+i+j;

			strcpy(fname,cur_dir_cont[dir_sw][i].file_spec);
			str_toupper(fname); fver_str=strchr(fname,';');
			*fver_str='\0';

			if (strcmp(file_name,fname) < 0)
						f_cpos=i;
			else if (strcmp(file_name,fname) > 0)
						s_cpos=i+1;
			else
			{
				fver_str++; fver_nbr_i=atoi(fver_str);

				if (fver_nbr_i <= fver_nbr)
							f_cpos=i;
				else
					s_cpos=i+1;
			}
		}

		strcpy(fname,cur_dir_cont[dir_sw][f_cpos].file_spec);
		str_toupper(fname); fver_str=strchr(fname,';');
		*fver_str='\0';

		if (strcmp(file_name,fname) < 0)
		{
			if (f_cpos)
				cur_dir_parms[dir_sw].dir_cpos=f_cpos-1;
		}
		else
			cur_dir_parms[dir_sw].dir_cpos=f_cpos;
	}
}

void before_restore(dir_sw)
int	dir_sw;
{
	file_bar_off(dir_sw);
	erase_dir_w(dir_sw);
	disp_dir_spec(dir_sw);
}

void restore_dir_cont(dir_sw,file_spec)
char	*file_spec;
int	dir_sw;
{
	int	i,
		j,
		k,
		l,
		m,
		alt_dir,
		mess_disp_id;

	SMG$SET_CURSOR_ABS(&comm_w,&1,&comm_col);

	disp_mess(&mess_disp_id,"",vtfm_bgt[24],(term_row_qua-4)/2,
				str_centr_pos(term_col_qua,vtfm_bgt[24])-1);

	alt_dir=(dir_sw+1)%2; l=strlen(file_spec);

	for (i=0; i <= 1; i++)
	{
		old_dir_parms[i]=cur_dir_parms[i];
		str_toupper(old_dir_parms[i].find_spec);

		for (j=0; j <= cur_dir_parms[i].dir_fqua; j++)
				old_dir_cont[i][j]=cur_dir_cont[i][j];
	}

	read_dir(dir_sw);

	if (l)
		find_dir_cpos(dir_sw,file_spec);
	else
	{
		i=old_dir_parms[dir_sw].dir_cpos;

		if (old_dir_cont[dir_sw][i].file_type >= 0)
			find_dir_cpos(dir_sw,
				old_dir_cont[dir_sw][i].file_spec);
	}

	if (strcmp(old_dir_parms[dir_sw].dir_spec,
			old_dir_parms[alt_dir].dir_spec) ||
		strcmp(old_dir_parms[dir_sw].find_spec,
			old_dir_parms[alt_dir].find_spec))
						read_dir(alt_dir);
	else
	{
		cur_dir_parms[alt_dir].dir_fqua=
				cur_dir_parms[dir_sw].dir_fqua;
		cur_dir_parms[alt_dir].ord_fqua=
				cur_dir_parms[dir_sw].ord_fqua;

		for (i=0; i <= cur_dir_parms[dir_sw].dir_fqua; i++)
			cur_dir_cont[alt_dir][i]=cur_dir_cont[dir_sw][i];
	}

	i=old_dir_parms[alt_dir].dir_cpos;

	if (old_dir_cont[alt_dir][i].file_type >= 0)
		find_dir_cpos(alt_dir,old_dir_cont[alt_dir][i].file_spec);

	for (i=0; i <= 1; i++)
		if (old_dir_parms[i].sel_fqua)
	{
		k=cur_dir_parms[i].dir_cpos;

		for (j=0; j <= old_dir_parms[i].dir_fqua; j++)
			if (old_dir_cont[i][j].file_selp == '*')
			{
				find_dir_cpos(i,old_dir_cont[i][j].file_spec);
				m=cur_dir_parms[i].dir_cpos;

				if (strcmp(cur_dir_cont[i][m].file_spec,
					old_dir_cont[i][j].file_spec) == 0)
				{
					cur_dir_cont[i][m].file_selp='*';
					cur_dir_parms[i].sel_fqua++;
				}
			}

		cur_dir_parms[i].dir_cpos=k;
	}

	SMG$DELETE_VIRTUAL_DISPLAY(&mess_disp_id);

	file_bar_off(dir_sw);

	for (i=0; i <= 1; i++)
	{
		erase_dir_w(i);
		disp_dir_cont(i,0);
	}

	file_bar_on(dir_sw);

	SMG$SET_KEYPAD_MODE(&keyb_id,&1);
}

void disp_mess(mess_did_pp,title_str,text_str,s_row,s_col)
char	*title_str,
	*text_str;
int	*mess_did_pp,
	s_row,
	s_col;
{
	int	l,
		mess_cq,
		a_col,
		curs_disp_id,
		x_row,
		x_col;

	struct
	{
		char	entry_str[96];
	}
		entry_list[2];

	if (strlen(text_str))
	{
		SMG$FIND_CURSOR_DISPLAY(&pstb_id,&curs_disp_id);

		SMG$RETURN_CURSOR_POS(&curs_disp_id,&x_row,&x_col);

		mess_cq=strlen(title_str);

		if (mess_cq > (term_col_qua-4))
				mess_cq=term_col_qua-4;

		strncpy(entry_list[0].entry_str,title_str,mess_cq);
		entry_list[0].entry_str[mess_cq]='\0';

		l=strlen(text_str);

		if (l > (term_col_qua-4)) l=term_col_qua-4;

		if (l > mess_cq) mess_cq=l;

		strncpy(entry_list[1].entry_str,text_str,l);
		entry_list[1].entry_str[l]='\0';

		mess_cq=mess_cq+2;

		SMG$CREATE_VIRTUAL_DISPLAY(&3,&mess_cq,
				mess_did_pp,&SMG$M_BORDER,&SMG$M_REVERSE);

		disp_w_label(*mess_did_pp,
			entry_list[0].entry_str,SMG$K_BOTTOM,2);

		a_col=str_centr_pos(mess_cq,entry_list[1].entry_str);

		SMG$SET_CURSOR_ABS(mess_did_pp,&2,&a_col);
		cre_s_str_desc(entry_list[1].entry_str);
		SMG$PUT_LINE(mess_did_pp,&s_str_desc);

		SMG$PASTE_VIRTUAL_DISPLAY(mess_did_pp,
					&pstb_id,&s_row,&s_col);

		SMG$SET_CURSOR_ABS(&curs_disp_id,&x_row,&x_col);
	}
}

void spawn_proc(dir_sw,spawn_str_sw)
int	dir_sw,
	spawn_str_sw;	/* 0 - GotoVMS, 1 - comm_str */
{
	int	i,
		j,
		save_disp_id,
		spn_ret_st;

	char	spawn_comm_str[256]={'$','\0'};

	if (spawn_str_sw)
	{
		SMG$ERASE_DISPLAY(&comm_w);
		SMG$SET_CURSOR_ABS(&comm_w,&1,&1);
	}

	SMG$ERASE_PASTEBOARD(&pstb_id);

	SMG$SAVE_PHYSICAL_SCREEN(&pstb_id,&save_disp_id);

	CURSOR_TO_HOME;

	if (spawn_str_sw)
	{
		strcat(spawn_comm_str,spawn_comm[0].comm_str);

		if (strcmp(spawn_comm[0].comm_str,
					spawn_comm[1].comm_str))
		{
			for (i=MAX_COMM_QUA-2; i >= 0; i--)
					spawn_comm[i+1]=spawn_comm[i];

			if (comm_stack_qua < MAX_COMM_QUA-1)
						comm_stack_qua++;
		}

		spawn_comm[0].comm_str[0]='\0';
		comm_cpos=0; comm_col=1; comm_stack_cpos=0;

			i=strlen(spawn_comm_str);

		do
			i--;
		while (spawn_comm_str[i] == ' ');

		spawn_comm_str[i+1]='\0';

			i=0;

		do
			i++;
		while (spawn_comm_str[i] == ' ');

			j=0;

		do
		{
			j++;
			spawn_comm_str[j]=spawn_comm_str[i+j-1];
		}
		while (spawn_comm_str[i+j-1] != '\0');

		str_toupper(spawn_comm_str);

		printf("%s%s%s\n",comm_prompt,spawn_comm_str+1,"...");

		cre_s_str_desc(spawn_comm_str);
		spn_ret_st=LIB$SPAWN(&s_str_desc);
	}
	else
	{
		printf("%s %s%s\n",vtfm_bgt[28],vtfm_bgt[79],"...");

		cre_s_str_desc(comm_prompt);
		spn_ret_st=LIB$SPAWN(0,0,0,0,0,0,0,0,0,0,&s_str_desc);
	}

	INST_LAT_FONT;

	if (spn_ret_st != SS$_NORMAL)
			printf("%s\n",vtfm_bgt[73]);

	PAK_MESS;

	SMG$READ_KEYSTROKE(&keyb_id,&key_pr);

	set_term_row_col();

	SMG$RESTORE_PHYSICAL_SCREEN(&pstb_id,&save_disp_id);

	SMG$DELETE_VIRTUAL_DISPLAY(&save_disp_id);

	if (spn_ret_st == SS$_NORMAL)
			restore_dir_cont(dir_sw,"");
}

void cok_bar_on(cok_did_pp,bar_col,bar_width)
int	*cok_did_pp,
	bar_col,
	bar_width;
{
	SMG$SET_CURSOR_ABS(cok_did_pp,&4,&bar_col);
	SMG$CHANGE_RENDITION(cok_did_pp,&4,&bar_col,
			&1,&bar_width,&SMG$M_NORMAL,&SMG$M_REVERSE);
}

void cok_bar_off(cok_did_pp,bar_col,bar_width)
int	*cok_did_pp,
	bar_col,
	bar_width;
{
	SMG$CHANGE_RENDITION(cok_did_pp,&4,&bar_col,&1,&bar_width);
}

void cok_handler(cok_did_pp,title_str,text_str,s_row,s_col,menu_sw)
char	*title_str,
	*text_str;
int	*cok_did_pp,
	s_row,
	s_col,
	menu_sw;
{
	int	i,
		j,
		l,
		cok_cq,
		menu_idx=0,
		bar_col,
		bar_width=8,
		curs_disp_id,
		x_row,
		x_col;

	char	*cok_menu[]=
		{
			" Cancel   Okay ",
			" Yes   No "
		};

	struct
	{
		char	entry_str[96];
	}
		entry_list[3];

	if (strlen(text_str))
	{
		SMG$FIND_CURSOR_DISPLAY(&pstb_id,&curs_disp_id);

		SMG$RETURN_CURSOR_POS(&curs_disp_id,&x_row,&x_col);

		if (menu_sw)
		{
			menu_idx=1;
			bar_width=5;
		}

		cok_cq=strlen(cok_menu[menu_idx]);
		strcpy(entry_list[2].entry_str,cok_menu[menu_idx]);

		l=strlen(title_str);
		if (l > (term_col_qua-4)) l=term_col_qua-4;
		if (l > cok_cq) cok_cq=l;
		strncpy(entry_list[0].entry_str,title_str,l);
		entry_list[0].entry_str[l]='\0';

		l=strlen(text_str);
		if (l > (term_col_qua-4)) l=term_col_qua-4;
		if (l > cok_cq) cok_cq=l;
		strncpy(entry_list[1].entry_str,text_str,l);
		entry_list[1].entry_str[l]='\0';

		cok_cq=cok_cq+2;

		SMG$CREATE_VIRTUAL_DISPLAY(&5,&cok_cq,
				cok_did_pp,&SMG$M_BORDER,&SMG$M_REVERSE);

		for (i=0; i <= 2; i++) if (i)
		{
			j=2*i;

			bar_col=str_centr_pos(cok_cq,
					entry_list[i].entry_str);

			SMG$SET_CURSOR_ABS(cok_did_pp,&j,&bar_col);
			cre_s_str_desc(entry_list[i].entry_str);
			SMG$PUT_LINE(cok_did_pp,&s_str_desc);
		}
		else
			disp_w_label(*cok_did_pp,
				entry_list[0].entry_str,SMG$K_TOP,
					str_centr_pos(cok_cq+2,
						entry_list[0].entry_str));

		SMG$PASTE_VIRTUAL_DISPLAY(cok_did_pp,
					&pstb_id,&s_row,&s_col);

		cok_bar_on(cok_did_pp,bar_col,bar_width);

		SMG$SET_CURSOR_ABS(&curs_disp_id,&x_row,&x_col);

		l=strlen(cok_menu[menu_idx]);

		do
		{
			SMG$READ_KEYSTROKE(&keyb_id,&key_pr);

		switch (key_pr)
		{
			case	SMG$K_TRM_CTRLR:
					refresh_screen();
					break;
			case 	SMG$K_TRM_LEFT:
			case 	SMG$K_TRM_RIGHT:
			case 	SMG$K_TRM_BS:
			case 	SMG$K_TRM_UP:
			case 	SMG$K_TRM_HT:
			case 	SMG$K_TRM_DOWN:
					cok_bar_off(cok_did_pp,
						bar_col,bar_width);

				if (menu_sw)
				{
					if (bar_width == 5)
					{
						bar_col=bar_col+l-4;
						bar_width=4;
					}
					else
					{
						bar_col=bar_col-l+4;
						bar_width=5;
					}
				}
				else
				{
					if (bar_width == 8)
					{
						bar_col=bar_col+l-6;
						bar_width=6;
					}
					else
					{
						bar_col=bar_col-l+6;
						bar_width=8;
					}
				}
	
				cok_bar_on(cok_did_pp,bar_col,bar_width);
						break;
			case	SMG$K_TRM_CR:
				if (menu_sw)
				{
					if (bar_width == 4)
						key_pr=(-SMG$K_TRM_CR);
				}
				else
				{
					if (bar_width == 8)
						key_pr=(-SMG$K_TRM_CR);
				}
						return;
			case	SMG$K_TRM_KP0:
						return;
		}
			SMG$SET_CURSOR_ABS(&curs_disp_id,&x_row,&x_col);
		}
		while (1 == 1);
	}
}

void str_shift(text_str)
char	text_str[];	/* \0-terminated string */
{
	int	i,
		l,
		m=0;

	l=strlen(text_str);

	for (i=0; i <= l; i++)
		if (text_str[i] != ' ')
		{
			if (i > m) text_str[m]=text_str[i];
			m++;
		}
}

void input_w_line(disp_id,line_nbr,text_str,txt_nbr_sw)
char	text_str[];	/* \0-terminated string */
int	disp_id,
	line_nbr,
	txt_nbr_sw;
{
	int	i,
		l,
		c_col=1,
		e_col=0,
		curs_disp_id,
		x_row,
		x_col;

	char	ins_char[2]={'\0','\0'};

	SMG$GET_DISPLAY_ATTR(&disp_id,&l,&e_col);

	if (e_col > 0)
	{
		SMG$FIND_CURSOR_DISPLAY(&pstb_id,&curs_disp_id);

		SMG$RETURN_CURSOR_POS(&curs_disp_id,&x_row,&x_col);

	l=strlen(text_str);

	if (l >= e_col) text_str[e_col-1]='\0';

	l=strlen(text_str);

	SMG$DELETE_CHARS(&disp_id,&e_col,&line_nbr,&c_col);
	SMG$SET_CURSOR_ABS(&disp_id,&line_nbr,&c_col);
	cre_s_str_desc(text_str);
	SMG$PUT_LINE(&disp_id,&s_str_desc);

	c_col=l+1; 

	SMG$SET_CURSOR_ABS(&disp_id,&line_nbr,&c_col);

	do
	{
		SMG$READ_KEYSTROKE(&keyb_id,&key_pr);

		switch (key_pr)
		{
		case	SMG$K_TRM_CTRLR:
				refresh_screen();
				break;
		case	SMG$K_TRM_LEFT:
				if (c_col == 1)
					ring_bell(1);
				else
				{
					c_col--;
					SMG$SET_CURSOR_ABS(&disp_id,
							&line_nbr,&c_col);
				}
					break;
		case	SMG$K_TRM_RIGHT:
				if (text_str[c_col-1] == '\0')
							ring_bell(1);
				else
				{
					c_col++;
					SMG$SET_CURSOR_ABS(&disp_id,
							&line_nbr,&c_col);
				}
					break;
		case	SMG$K_TRM_CTRLH:
				if (c_col == 1)
					ring_bell(1);
				else
				{
					c_col=1;
					SMG$SET_CURSOR_ABS(&disp_id,
							&line_nbr,&c_col);
				}
					break;
		case	SMG$K_TRM_CTRLE:
				if (text_str[c_col-1] == '\0')
							ring_bell(1);
				else
				{
					c_col=strlen(text_str)+1;
					SMG$SET_CURSOR_ABS(&disp_id,
							&line_nbr,&c_col);
				}
					break;
		case	SMG$K_TRM_CTRLU:
				if (c_col == 1)
					ring_bell(1);
				else
				{
					l=strlen(text_str);

					for (i=c_col-1; i <= l; i++)
						text_str[i-(c_col-1)]=
								text_str[i];

					l=c_col-1; c_col=1;

					SMG$DELETE_CHARS(&disp_id,&l,
							&line_nbr,&c_col);
				}
					break;
		case	SMG$K_TRM_PF4:
				text_str[0]='\0'; c_col=1;

				SMG$DELETE_CHARS(&disp_id,
						&e_col,&line_nbr,&c_col);

					break;
		case	127:				/* DEL-key */
				if (c_col == 1)
					ring_bell(1);
				else
				{
					l=strlen(text_str);

					for (i=c_col-1; i <= l; i++)
						text_str[i-1]=text_str[i];

					c_col--;

					SMG$DELETE_CHARS(&disp_id,&1,
							&line_nbr,&c_col);
				}
					break;
		case	SMG$K_TRM_CR:
		case	SMG$K_TRM_KP0:
				SMG$SET_CURSOR_ABS(&curs_disp_id,
							&x_row,&x_col);
					return;
		default:
			if (((txt_nbr_sw == 0) &&
			       (((key_pr > 31) && (key_pr < 127)) ||
				((key_pr > 160) && (key_pr < 164)) ||
				 (key_pr == 165) ||
				((key_pr > 166) && (key_pr < 172)) ||
				((key_pr > 175) && (key_pr < 180)) ||
				((key_pr > 180) && (key_pr < 184)) ||
				((key_pr > 184) && (key_pr < 190)) ||
				((key_pr > 190) && (key_pr < 208)) ||
				((key_pr > 208) && (key_pr < 222)) ||
				((key_pr > 222) && (key_pr < 240)) ||
				((key_pr > 240) && (key_pr < 254)))) ||
			    ((txt_nbr_sw != 0) &&
				((key_pr == 32) ||
				((key_pr > 47) && (key_pr < 58)))))
			{
				l=strlen(text_str);

				if (l < e_col-1)
				{
					for (i=l; i >= c_col-1; i--)
						text_str[i+1]=text_str[i];

				text_str[c_col-1]=key_pr;

				ins_char[0]=key_pr;
				cre_s_str_desc(ins_char);
				SMG$INSERT_CHARS(&disp_id,
					&s_str_desc,&line_nbr,&c_col);
							
					c_col++;
				}
				else
					ring_bell(1);
			}
		}
	}
	while (1 == 1);
	}
}

void accept_wild_spec(bgt_did_pp,wild_did_pp,title_str,wild_spec)
char	*title_str,
	*wild_spec;
int	*bgt_did_pp,
	*wild_did_pp;
{
	int	a_row,
		a_col,
		parse_ret_st,
		mess_disp_id;

	SMG$CREATE_VIRTUAL_DISPLAY(&3,&50,
			bgt_did_pp,&SMG$M_BORDER,&SMG$M_REVERSE);

	disp_w_label(*bgt_did_pp,title_str,SMG$K_TOP,
				str_centr_pos(52,title_str));

	SMG$CREATE_VIRTUAL_DISPLAY(&1,&48,
				wild_did_pp,&SMG$M_NOBORDER,&SMG$M_REVERSE);

	a_row=(term_row_qua-4)/2; a_col=(term_col_qua-50)/2+1;

	SMG$PASTE_VIRTUAL_DISPLAY(bgt_did_pp,&pstb_id,&a_row,&a_col);

	a_row++; a_col++;

	SMG$PASTE_VIRTUAL_DISPLAY(wild_did_pp,&pstb_id,&a_row,&a_col);

	SMG$SET_CURSOR_ABS(&comm_w,&1,&comm_col);

	do
	{
		input_w_line(*wild_did_pp,1,wild_spec,0);
		str_shift(wild_spec);

		if ((key_pr == SMG$K_TRM_KP0) ||
			((key_pr == SMG$K_TRM_CR) &&
			(strlen(wild_spec) == 0)))
						return;
		else
		{
			SMG$SET_CURSOR_ABS(wild_did_pp,&1,&1);
			cre_s_str_desc(wild_spec);
			SMG$PUT_LINE(wild_did_pp,&s_str_desc);

			SMG$SET_CURSOR_ABS(&comm_w,&1,&comm_col);

			parse_ret_st=parse_file_spec(wild_spec);

			if ((parse_ret_st != SS$_NORMAL) ||
				((parse_ret_st == SS$_NORMAL) &&
				(strpbrk(wild_spec,":[]<>") != NULL)))
			{
				disp_mess(&mess_disp_id,
					vtfm_bgt[8],vtfm_bgt[21],
					(term_row_qua-4)/2+3,
					str_centr_pos(term_col_qua,
							vtfm_bgt[21])-1);

				SMG$READ_KEYSTROKE(&keyb_id,&key_pr);

				SMG$DELETE_VIRTUAL_DISPLAY(&mess_disp_id);

				SMG$SET_CURSOR_ABS(&comm_w,&1,&comm_col);
			}
			else
			{
				if (strpbrk(wild_spec,"*%") == NULL)
			{
				disp_mess(&mess_disp_id,vtfm_bgt[8],
					vtfm_bgt[61],(term_row_qua-4)/2+3,
					str_centr_pos(term_col_qua,
							vtfm_bgt[61])-1);

				SMG$READ_KEYSTROKE(&keyb_id,&key_pr);

				SMG$DELETE_VIRTUAL_DISPLAY(&mess_disp_id);

				SMG$SET_CURSOR_ABS(&comm_w,&1,&comm_col);
			}
			else
				return;
			}
		}
	}
	while (1 == 1);
}

void un_sel_files(dir_sw,sel_sw)
int	dir_sw,
	sel_sw;
{
	int	i,
		bgt_disp_id,
		wild_disp_id,
		str_ret_st;

	char	sel_spec[64],
		file_spec[256];

	struct	dsc$descriptor_s sel_s_str_desc;

	strcpy(sel_spec,cur_dir_parms[dir_sw].sel_spec);

	if (sel_sw)
		accept_wild_spec(&bgt_disp_id,
			&wild_disp_id,vtfm_bgt[63],sel_spec);
	else
		accept_wild_spec(&bgt_disp_id,
			&wild_disp_id,vtfm_bgt[64],sel_spec);

	if (strlen(sel_spec) && (key_pr == SMG$K_TRM_CR))
	{
		strcpy(cur_dir_parms[dir_sw].sel_spec,sel_spec);
		str_toupper(sel_spec);
		cre_s_str_desc(sel_spec);
		sel_s_str_desc=s_str_desc;

	for (i=0; i <= cur_dir_parms[dir_sw].dir_fqua; i++)
			if (cur_dir_cont[dir_sw][i].file_type >= 0)
	{
		strcpy(file_spec,cur_dir_cont[dir_sw][i].file_spec);
		str_toupper(file_spec);
		cre_s_str_desc(file_spec);

		str_ret_st=STR$MATCH_WILD(&s_str_desc,&sel_s_str_desc);

		if (str_ret_st == STR$_MATCH)
		{
			if (sel_sw)
			{
				if (cur_dir_cont[dir_sw][i].file_selp != '*')
				{
					cur_dir_cont[dir_sw][i].file_selp='*';
					cur_dir_parms[dir_sw].sel_fqua++;
				}
			}
			else
			{
				if (cur_dir_cont[dir_sw][i].file_selp == '*')
				{
					cur_dir_cont[dir_sw][i].file_selp=' ';
					cur_dir_parms[dir_sw].sel_fqua--;
				}
			}
		}
	}
		SMG$DELETE_VIRTUAL_DISPLAY(&wild_disp_id);
		SMG$DELETE_VIRTUAL_DISPLAY(&bgt_disp_id);

		file_bar_off(dir_sw);
		erase_dir_w(dir_sw);
		disp_dir_cont(dir_sw,0);
		file_bar_on(dir_sw);
	}
	else
	{
		SMG$DELETE_VIRTUAL_DISPLAY(&wild_disp_id);
		SMG$DELETE_VIRTUAL_DISPLAY(&bgt_disp_id);
	}
}

void find_files(dir_sw)
int	dir_sw;
{
	int	bgt_disp_id,
		wild_disp_id;

	char	find_spec[64];

	strcpy(find_spec,cur_dir_parms[dir_sw].find_spec);

	accept_wild_spec(&bgt_disp_id,
			&wild_disp_id,vtfm_bgt[65],find_spec);

	SMG$DELETE_VIRTUAL_DISPLAY(&wild_disp_id);
	SMG$DELETE_VIRTUAL_DISPLAY(&bgt_disp_id);

	if (strlen(find_spec) && (key_pr == SMG$K_TRM_CR))
	{
		strcpy(cur_dir_parms[dir_sw].find_spec,find_spec);
		cur_dir_parms[dir_sw].sel_fqua=0;
		strcpy(find_spec,"\001");
		strcat(find_spec,vtfm_bgt[49]);
		before_restore(dir_sw);
		restore_dir_cont(dir_sw,find_spec);
	}
}

void cre_paste_filew(bgt_did_pp,file_did_pp,title_str)
char	*title_str;
int	*bgt_did_pp,
	*file_did_pp;
{
	int	a_row=3,
		a_col=76;

	SMG$CREATE_VIRTUAL_DISPLAY(&a_row,&a_col,
			bgt_did_pp,&SMG$M_BORDER,&SMG$M_REVERSE);

	disp_w_label(*bgt_did_pp,title_str,SMG$K_TOP,
				str_centr_pos(78,title_str));

	a_row=1; a_col=74;

	SMG$CREATE_VIRTUAL_DISPLAY(&a_row,&a_col,
				file_did_pp,&SMG$M_NOBORDER,&SMG$M_REVERSE);

	a_row=(term_row_qua-4)/2; a_col=(term_col_qua-76)/2+1;

	SMG$PASTE_VIRTUAL_DISPLAY(bgt_did_pp,&pstb_id,&a_row,&a_col);

	a_row++; a_col++;

	SMG$PASTE_VIRTUAL_DISPLAY(file_did_pp,&pstb_id,&a_row,&a_col);

	SMG$SET_CURSOR_ABS(&comm_w,&1,&comm_col);
}

int probe_dir_cont(dir_spec)
char	*dir_spec;
{
	int	l,
		c_text=0,
		rms_ret_st;

	l=strlen(dir_spec);

	strcat(dir_spec,vtfm_bgt[47]);
	cre_s_str_desc(dir_spec);

	rms_ret_st=LIB$FIND_FILE(&s_str_desc,&d_str_desc,&c_text);

	LIB$FIND_FILE_END(&c_text);

	dir_spec[l]='\0';

	return rms_ret_st;
}

int delete_subdir_tree(bgt_did_pp,file_did_pp,subdir_spec)
char	*subdir_spec;
int	*bgt_did_pp,
	*file_did_pp;
{
	int	i,
		err_sw,
		c_text=0,
		stat_ret_st,
		rms_ret_st,
		sd_rms_ret_st,
		dsd_ret_st,
		cok_disp_id,
		mess_disp_id;

	char	sysmsg_text[256],
		file_spec[256],
		find_spec[256],
		*brack_p;

	struct	dsc$descriptor_s all_s_str_desc;
	struct	stat x_file_info;
	struct	FAB src_fab;

	strcat(strcpy(find_spec,subdir_spec),vtfm_bgt[47]);
	cre_s_str_desc(find_spec);
	all_s_str_desc=s_str_desc;

	i=0;

	do
	{
		dsd_ret_st=0;

		rms_ret_st=LIB$FIND_FILE(&all_s_str_desc,&d_str_desc,&c_text);

	switch (rms_ret_st)
	{
		case	RMS$_NORMAL:
				i++; err_sw=0;

			if (i == 1)
			{
			        SMG$SET_CURSOR_ABS(bgt_did_pp,&1,&1);

				cre_s_str_desc(subdir_spec);
				SMG$PUT_LINE(bgt_did_pp,&s_str_desc);

				SMG$ERASE_CHARS(file_did_pp,&74,&1,&1);
        			SMG$SET_CURSOR_ABS(&comm_w,&1,&comm_col);
			}
				strncpy(file_spec,
					d_str_desc.dsc$a_pointer,
					d_str_desc.dsc$w_length);
				file_spec[d_str_desc.dsc$w_length]='\0';

				stat_ret_st=stat(file_spec,&file_info);

		if (stat_ret_st == 0)
		{
			if (file_info.st_mode & S_IFDIR)
			{
				strcpy(sysmsg_text,strchr(file_spec,']')+1);
				strcpy(strchr(sysmsg_text,'.'),"]");
				strcpy(strchr(subdir_spec,']'),".");
				strcat(subdir_spec,sysmsg_text);

				x_file_info=file_info;

				dsd_ret_st=delete_subdir_tree(bgt_did_pp,
						file_did_pp,subdir_spec);

				file_info=x_file_info;

				brack_p=strchr(subdir_spec,']');

				do
					brack_p--;
				while (*brack_p != '.');

				strcpy(brack_p,"]");

			if (dsd_ret_st != (-1))
			{
			        SMG$SET_CURSOR_ABS(bgt_did_pp,&1,&1);

				cre_s_str_desc(subdir_spec);
				SMG$PUT_LINE(bgt_did_pp,&s_str_desc);

				if ((dsd_ret_st) && (dsd_ret_st != RMS$_DNF))
				{
					sd_rms_ret_st=dsd_ret_st;
					err_sw=1;
				}
			}
                        }

				if ((err_sw == 0) && (dsd_ret_st != (-1)))
			{
				cre_s_str_desc(strchr(file_spec,']')+1);
				SMG$PUT_LINE(file_did_pp,&s_str_desc);

	                	SMG$SET_CURSOR_ABS(&comm_w,&1,&comm_col);

				src_fab=cc$rms_fab;
				src_fab.fab$l_fna=file_spec;
				src_fab.fab$b_fns=strlen(file_spec);
				src_fab.fab$w_ifi=0;

				sd_rms_ret_st=SYS$ERASE(&src_fab);

				if (sd_rms_ret_st != RMS$_NORMAL)
				{
					err_sw=(-2);

					if (sd_rms_ret_st == RMS$_MKD)
						sd_rms_ret_st=
							src_fab.fab$l_stv;
				}
			}
		}
		else
		{
			err_sw=1;

			if (stat_ret_st == (-2))
					sd_rms_ret_st=RMS$_PRV;
			else
				sd_rms_ret_st=vaxc$errno;
		}

		if (dsd_ret_st != (-1))
		{
			dsd_ret_st=0;

			if (err_sw == 1)
		{
			cre_s_str_desc(strchr(file_spec,']')+1);
			SMG$PUT_LINE(file_did_pp,&s_str_desc);

                	SMG$SET_CURSOR_ABS(&comm_w,&1,&comm_col);
		}

		if (err_sw)
		{
			get_sysmsg(sd_rms_ret_st,sysmsg_text);

			disp_mess(&mess_disp_id,
				vtfm_bgt[8],sysmsg_text,
				(term_row_qua-4)/2+3,
				str_centr_pos(term_col_qua,sysmsg_text)-1);

			SMG$READ_KEYSTROKE(&keyb_id,&key_pr);

			SMG$DELETE_VIRTUAL_DISPLAY(&mess_disp_id);

			SMG$SET_CURSOR_ABS(&comm_w,&1,&comm_col);

			cok_handler(&cok_disp_id,"",
				vtfm_bgt[75],(term_row_qua-4)/2+3,
				str_centr_pos(term_col_qua,vtfm_bgt[75])-1,1);

			SMG$DELETE_VIRTUAL_DISPLAY(&cok_disp_id);

			if (key_pr != SMG$K_TRM_CR) dsd_ret_st=(-1);
		}
		}
				break;
		case	RMS$_FNF:
		case	RMS$_NMF:
				break;
		default:
				dsd_ret_st=rms_ret_st;
	}
	}
	while ((dsd_ret_st == 0) &&
		(rms_ret_st != RMS$_FNF) && (rms_ret_st != RMS$_NMF));

	LIB$FIND_FILE_END(&c_text);

	return dsd_ret_st;
}

void delete_files(dir_sw)
int	dir_sw;
{
	int	i,
		sel_fqua,
		del_fqua=0,
		err_sw,
		stat_ret_st,
		rms_ret_st,
		dsd_ret_st,
		cok_disp_id,
		bgt_disp_id,
		file_disp_id,
		mess_disp_id;

	char	sysmsg_text[256],
		file_spec[256],
		subdir_spec[256];

	struct	stat x_file_info;
	struct	FAB src_fab;

	i=cur_dir_parms[dir_sw].dir_cpos;

	if (cur_dir_parms[dir_sw].sel_fqua)
	{
		sprintf(sysmsg_text,"%u %s",
			cur_dir_parms[dir_sw].sel_fqua,vtfm_bgt[23]);

		if (cur_dir_parms[dir_sw].sel_fqua > 1)
					strcat(sysmsg_text,"s");

		strcat(sysmsg_text,vtfm_bgt[66]);
	}
	else
		if (cur_dir_cont[dir_sw][i].file_type >= 0)
			sprintf(sysmsg_text,"%s%s",vtfm_bgt[67],vtfm_bgt[66]);
	else
	{
		ring_bell(1);
		return;
	}

	cok_handler(&cok_disp_id,vtfm_bgt[56],sysmsg_text,
					(term_row_qua-6)/2,
					str_centr_pos(term_col_qua,
					sysmsg_text)-1,0);

	if (key_pr == SMG$K_TRM_CR)
	{
		if (cur_dir_parms[dir_sw].sel_fqua == 0)
			cur_dir_cont[dir_sw][i].file_selp='*';

		sel_fqua=cur_dir_parms[dir_sw].sel_fqua;

		i=(-1);

		do
			i++;
		while (cur_dir_cont[dir_sw][i].file_selp != '*');

		SMG$DELETE_VIRTUAL_DISPLAY(&cok_disp_id);

		cre_paste_filew(&bgt_disp_id,&file_disp_id,vtfm_bgt[22]);

		do
		{
			dsd_ret_st=0;

			if (cur_dir_cont[dir_sw][i].file_selp == '*')
		{
			sel_fqua--; err_sw=0;

			strcpy(file_spec,cur_dir_parms[dir_sw].dir_spec);
			strcat(file_spec,cur_dir_cont[dir_sw][i].file_spec);

			stat_ret_st=stat(file_spec,&file_info);

		if (stat_ret_st == 0)
		{
			if (file_info.st_mode & S_IFDIR)
			{
				strcpy(subdir_spec,"[.");
				strcpy(sysmsg_text,
					cur_dir_cont[dir_sw][i].file_spec);
				strcpy(strchr(sysmsg_text,'.'),"]");
				strcat(subdir_spec,sysmsg_text);

				x_file_info=file_info;

				dsd_ret_st=delete_subdir_tree(&bgt_disp_id,
						&file_disp_id,subdir_spec);

				file_info=x_file_info;

			if (dsd_ret_st != (-1))
			{
				SMG$ERASE_CHARS(&bgt_disp_id,&76,&1,&1);
			        SMG$SET_CURSOR_ABS(&comm_w,&1,&comm_col);

				if ((dsd_ret_st) && (dsd_ret_st != RMS$_DNF))
				{
					rms_ret_st=dsd_ret_st;
					err_sw=1;
				}
			}
			}

			if ((err_sw == 0) && (dsd_ret_st != (-1)))
		{
			cre_s_str_desc(cur_dir_cont[dir_sw][i].file_spec);
			SMG$PUT_LINE(&file_disp_id,&s_str_desc);

                	SMG$SET_CURSOR_ABS(&comm_w,&1,&comm_col);

				src_fab=cc$rms_fab;
				src_fab.fab$l_fna=file_spec;
				src_fab.fab$b_fns=strlen(file_spec);
				src_fab.fab$w_ifi=0;

				rms_ret_st=SYS$ERASE(&src_fab);

				if (rms_ret_st != RMS$_NORMAL)
				{
					err_sw=(-2);

					if (rms_ret_st == RMS$_MKD)
						rms_ret_st=
							src_fab.fab$l_stv;
				}
		}
		}
		else
		{
			err_sw=1;

			if (stat_ret_st == (-2))
					rms_ret_st=RMS$_PRV;
			else
				rms_ret_st=vaxc$errno;
		}

		if (dsd_ret_st != (-1))
		{
			dsd_ret_st=0;

			if (err_sw == 1)
		{
			cre_s_str_desc(cur_dir_cont[dir_sw][i].file_spec);
			SMG$PUT_LINE(&file_disp_id,&s_str_desc);

                	SMG$SET_CURSOR_ABS(&comm_w,&1,&comm_col);
		}

		if (err_sw)
		{
			if (cur_dir_parms[dir_sw].sel_fqua == 0)
				cur_dir_cont[dir_sw][i].file_selp=' ';

				get_sysmsg(rms_ret_st,sysmsg_text);

				disp_mess(&mess_disp_id,
					vtfm_bgt[8],sysmsg_text,
					(term_row_qua-4)/2+3,
					str_centr_pos(term_col_qua,
							sysmsg_text)-1);

				SMG$READ_KEYSTROKE(&keyb_id,&key_pr);

				SMG$DELETE_VIRTUAL_DISPLAY(&mess_disp_id);

				SMG$SET_CURSOR_ABS(&comm_w,&1,&comm_col);

			if (sel_fqua > 0)
			{
				cok_handler(&cok_disp_id,"",
					vtfm_bgt[75],(term_row_qua-4)/2+3,
					str_centr_pos(term_col_qua,
							vtfm_bgt[75])-1,1);

				SMG$DELETE_VIRTUAL_DISPLAY(&cok_disp_id);

				if (key_pr != SMG$K_TRM_CR) sel_fqua=0;
			}
		}
		else
		{
			cur_dir_cont[dir_sw][i].file_selp=' ';
			del_fqua++;

			if (cur_dir_parms[dir_sw].sel_fqua)
				cur_dir_parms[dir_sw].sel_fqua--;
		}
		}
		}
			i++;
		}
		while ((dsd_ret_st == 0) && (sel_fqua > 0));

		SMG$DELETE_VIRTUAL_DISPLAY(&file_disp_id);
		SMG$DELETE_VIRTUAL_DISPLAY(&bgt_disp_id);

		if (del_fqua)
			restore_dir_cont(dir_sw,"");
	}
	else
		SMG$DELETE_VIRTUAL_DISPLAY(&cok_disp_id);
}

int scan_file_spec(dir_sw,file_spec,f_name,f_ext,f_ver,dfs_sw)
char	*file_spec,
	*f_name,
	*f_ext,
	*f_ver;
int	dir_sw,
	dfs_sw;
{
	int	i,
		l,
		err_sw,
		parse_ret_st,
		sys_ret_st,
		rms_ret_st,
		mess_disp_id;

	char	sysmsg_text[256],
		x_file_spec[256],
		*brack_p,
		*semi_p;

	struct	fspec_item_name
		{
			char	name[256];
		};

	struct	fscndef	fspec_ival[6];

	struct	fspec_item_name fspec_item[5];

	strcpy(x_file_spec,file_spec);

	err_sw=0;

	parse_ret_st=parse_file_spec(x_file_spec);

	if ((parse_ret_st == SS$_NORMAL) &&
		(strchr(x_file_spec,'%') == NULL) &&
		(strstr(x_file_spec,"::") == NULL) &&
		(strstr(x_file_spec,"[.") == NULL) &&
		(strstr(x_file_spec,"<.") == NULL) &&
		(strstr(x_file_spec,".]") == NULL) &&
		(strstr(x_file_spec,".>") == NULL) &&
		(strstr(x_file_spec,"[-") == NULL) &&
		(strstr(x_file_spec,"<-") == NULL) &&
		(strstr(x_file_spec,".-") == NULL) &&
		(strstr(x_file_spec,"..") == NULL))
	{
		chg_dir_brack_comma(x_file_spec);
		cre_s_str_desc(x_file_spec);

		for (i=0; i <=4; i++)
			fspec_item[i].name[0]='\0';

		fspec_ival[0].fscn$w_item_code=FSCN$_DEVICE;
		fspec_ival[1].fscn$w_item_code=FSCN$_DIRECTORY;
		fspec_ival[2].fscn$w_item_code=FSCN$_NAME;
		fspec_ival[3].fscn$w_item_code=FSCN$_TYPE;
		fspec_ival[4].fscn$w_item_code=FSCN$_VERSION;

		fspec_ival[5].fscn$w_length=0;
		fspec_ival[5].fscn$w_item_code=0;

		SYS$FILESCAN(&s_str_desc,&fspec_ival,0,0,0);

		for (i=0; i<=4; i++)
			if (fspec_ival[i].fscn$l_addr)
		{
			brack_p=(char *)fspec_ival[i].fscn$l_addr;
			l=fspec_ival[i].fscn$w_length;
			strncpy(fspec_item[i].name,brack_p,l);
			fspec_item[i].name[l]='\0';

			if (i < 2)
			{
				if (strchr(fspec_item[i].name,'*'))
								err_sw=1;
			}
			else
			{
				if ((strchr(fspec_item[i].name,'*')) &&
					(strspn(fspec_item[i].name,".*;") < l))
								err_sw=1;
				if ((i == 4) &&
					(strchr(fspec_item[4].name,
							'*') == NULL))
				{
					l=atoi(fspec_item[4].name);
					if (abs(l) > 32767) err_sw=1;
				}
			}
		}

		if ((err_sw == 0) && (dfs_sw == 0) &&
			(((strlen(fspec_item[0].name)+
			strlen(fspec_item[1].name)) &&
			(strlen(fspec_item[2].name)+
			strlen(fspec_item[3].name)+
			strlen(fspec_item[4].name))) ||
			(((strlen(fspec_item[0].name)+
			strlen(fspec_item[1].name)) == 0) &&
			(strlen(fspec_item[3].name)+
			strlen(fspec_item[4].name))))) err_sw=1;

		x_file_spec[0]='\0';

		if ((err_sw == 0) &&
			(strlen(fspec_item[1].name) == 0) &&
			((strlen(fspec_item[0].name)) ||
			((strlen(fspec_item[0].name)+
			strlen(fspec_item[3].name)+
			strlen(fspec_item[4].name)) == 0)))
		{
			if (strlen(fspec_item[0].name))
			{
				strcpy(x_file_spec,fspec_item[0].name);
				l=strlen(x_file_spec);
				x_file_spec[l-1]='\0';
			}
			else
				strcpy(x_file_spec,fspec_item[2].name);

			sys_ret_st=translate_dirlnm(x_file_spec);

			if ((sys_ret_st == SS$_NORMAL) &&
				((strstr(x_file_spec,"::")) ||
				(strstr(x_file_spec,"..")) ||
				(strpbrk(x_file_spec,"-%*")))) err_sw=1;
		}
	}
	else
		err_sw=1;

	if (err_sw)
	{
		if (dfs_sw)
			disp_mess(&mess_disp_id,
				vtfm_bgt[8],vtfm_bgt[21],
				(term_row_qua-4)/2+3,
				str_centr_pos(term_col_qua,vtfm_bgt[21])-1);
		else
			disp_mess(&mess_disp_id,
				vtfm_bgt[8],vtfm_bgt[11],
				(term_row_qua-4)/2+3,
				str_centr_pos(term_col_qua,vtfm_bgt[11])-1);

		SMG$READ_KEYSTROKE(&keyb_id,&key_pr);

		SMG$DELETE_VIRTUAL_DISPLAY(&mess_disp_id);

		SMG$SET_CURSOR_ABS(&comm_w,&1,&comm_col);
	}
	else
	{
		if ((strlen(fspec_item[1].name) == 0) &&
			((strlen(fspec_item[0].name)) ||
			((strlen(fspec_item[0].name)+
			strlen(fspec_item[3].name)+
			strlen(fspec_item[4].name)) == 0)))
		{
			if (strlen(fspec_item[0].name))
		{
			if (sys_ret_st == SS$_NORMAL)
			{
				if (strchr(x_file_spec,':'))
				{
					strcpy(sysmsg_text,fspec_item[0].name);
					strcpy(fspec_item[0].name,x_file_spec);
					strcpy(x_file_spec,sysmsg_text);
				}
				else
				{
					strcpy(fspec_item[1].name,x_file_spec);
					strcpy(fspec_item[0].name,"");
				}
			}
		}
		else
		{
			if (sys_ret_st == SS$_NORMAL)
			{
				if (strchr(x_file_spec,':'))
				{
					strcpy(sysmsg_text,fspec_item[2].name);
					strcpy(fspec_item[0].name,x_file_spec);
					strcat(strcpy(x_file_spec,
							sysmsg_text),":");
				}
				else
					strcpy(fspec_item[1].name,x_file_spec);

				strcpy(fspec_item[2].name,"");
			}
			else
				if (dfs_sw == 0)
				{
					err_sw=1;
					rms_ret_st=RMS$_DIR;
				}
		}
		}

		if (err_sw == 0)
		{
			if (strlen(fspec_item[0].name))
		{
			sys_ret_st=get_phys_disk_name(fspec_item[0].name,
								sysmsg_text);

			if (sys_ret_st != SS$_NORMAL)
			{
				err_sw=1;
				rms_ret_st=sys_ret_st;
			}
		}

		if (err_sw == 0)
		{
			strcpy(sysmsg_text,fspec_item[0].name);
			strcat(sysmsg_text,fspec_item[1].name);

			rms_ret_st=probe_dir_cont(sysmsg_text);

			if ((rms_ret_st != RMS$_NORMAL) &&
				(rms_ret_st != RMS$_FNF) &&
					(rms_ret_st != RMS$_PRV)) err_sw=1;
			else
			{
				brack_p=strchr(fspec_item[0].name,'[');

				if ((brack_p) &&
					(strstr(fspec_item[0].name,
							".]") == NULL))
				{
					strcpy(fspec_item[1].name,brack_p);
					*brack_p='\0';
				}
				else
					if ((brack_p) ||
						((brack_p == NULL) &&
						(strchr(x_file_spec,':'))))
					strcpy(fspec_item[0].name,x_file_spec);

				str_toupper(fspec_item[0].name);

			if (strcmp(fspec_item[0].name,
						"SYS$DISK:") == 0)
			{
				semi_p=strchr(fspec_item[0].name,':');
				*semi_p='\0';

			sys_ret_st=translate_dirlnm(fspec_item[0].name);

				if (sys_ret_st != SS$_NORMAL)
				{
					err_sw=1;
					rms_ret_st=RMS$_LNE;
				}
			}

			if (err_sw == 0)
			{
				brack_p=strchr(cur_dir_parms[dir_sw].dir_spec,
									'[');

			if ((strlen(fspec_item[1].name) == 0) ||
				(strcmp(fspec_item[1].name,"[]") == 0))
					strcpy(fspec_item[1].name,brack_p);

			if (strlen(fspec_item[0].name) == 0)
			{
				*brack_p='\0';
				strcpy(fspec_item[0].name,
					cur_dir_parms[dir_sw].dir_spec);
				*brack_p='[';
			}

			if (strstr(fspec_item[1].name,"[."))
			{
				strcpy(x_file_spec,brack_p);
				brack_p=strchr(x_file_spec,']');
				semi_p=strchr(fspec_item[1].name,'.');
				strcpy(brack_p,semi_p);
				strcpy(fspec_item[1].name,x_file_spec);
			}
			}
			}
		}
		}

		if (err_sw)
		{
			get_sysmsg(rms_ret_st,sysmsg_text);

			disp_mess(&mess_disp_id,
				vtfm_bgt[8],sysmsg_text,
				(term_row_qua-4)/2+3,
				str_centr_pos(term_col_qua,sysmsg_text)-1);

			SMG$READ_KEYSTROKE(&keyb_id,&key_pr);

			SMG$DELETE_VIRTUAL_DISPLAY(&mess_disp_id);

			SMG$SET_CURSOR_ABS(&comm_w,&1,&comm_col);
		}
	}

	if (err_sw == 0)
	{
		strcpy(file_spec,fspec_item[0].name);
		strcat(file_spec,fspec_item[1].name);

		if (dfs_sw)
		{
			strcpy(f_name,fspec_item[2].name);
			strcpy(f_ext,fspec_item[3].name);
			strcpy(f_ver,fspec_item[4].name);

			if (strlen(f_name) == 0) strcpy(f_name,"*");
			if (strlen(f_ext) == 0) strcpy(f_ext,".*");
			if (strlen(f_ver) == 0) strcpy(f_ver,";*");
		}
	}

	return err_sw;
}

int translate_lnm(log_name)
char	*log_name;			/* Logical name on input */
{					/* Eqv string on output  */
	int	sys_ret_st;

	char	eqv_lnm[256];

	$DESCRIPTOR(lnmtbl_desc,"LNM$PROCESS_TABLE");

	eqvlnm_item.eqvlnm_buff_len=sizeof(eqv_lnm);
	eqvlnm_item.item_code=LNM$_STRING;
	eqvlnm_item.eqvlnm_buff_addr=eqv_lnm;
	eqvlnm_item.eqvlnm_len_addr=&eqv_lnm_len;
	eqvlnm_item.term=0;

	cre_s_str_desc(log_name);

	sys_ret_st=SYS$TRNLNM(&LNM$M_CASE_BLIND,
				&lnmtbl_desc,&s_str_desc,0,&eqvlnm_item);

	if (sys_ret_st == SS$_NORMAL)
	{
		eqv_lnm[eqv_lnm_len]='\0';
		strcpy(log_name,eqv_lnm);
	}

	return sys_ret_st;
}

int translate_dirlnm(dir_spec)
char	*dir_spec;			/* Dir logical name on input   */
{					/* Dir specification on output */
	int	sys_ret_st;

	char	eqv_dirlnm[256],
		x_eqv_dirlnm[256];

	$DESCRIPTOR(lnmtbl_desc,"LNM$FILE_DEV");

	eqvlnm_item.eqvlnm_buff_len=sizeof(eqv_dirlnm);
	eqvlnm_item.item_code=LNM$_STRING;
	eqvlnm_item.eqvlnm_buff_addr=eqv_dirlnm;
	eqvlnm_item.eqvlnm_len_addr=&eqv_lnm_len;
	eqvlnm_item.term=0;

	strcpy(eqv_dirlnm,dir_spec);

	do
	{
		strcpy(x_eqv_dirlnm,eqv_dirlnm);
		cre_s_str_desc(x_eqv_dirlnm);

		sys_ret_st=SYS$TRNLNM(&LNM$M_CASE_BLIND,
				&lnmtbl_desc,&s_str_desc,0,&eqvlnm_item);

		if (sys_ret_st == SS$_NORMAL)
				eqv_dirlnm[eqv_lnm_len]='\0';
	}
	while ((sys_ret_st == SS$_NORMAL) && 
			(eqv_dirlnm[eqv_lnm_len-1] != ':') &&
			(eqv_dirlnm[eqv_lnm_len-1] != ']') &&
			(eqv_dirlnm[eqv_lnm_len-1] != '>'));

	if (sys_ret_st == SS$_NORMAL)
	{
		strcpy(dir_spec,eqv_dirlnm);
		chg_dir_brack_comma(dir_spec);
	}

	return sys_ret_st;
}

void goto_dir(dir_sw)
int	dir_sw;
{
	int	err_sw,
		bgt_disp_id,
		dir_disp_id;

	char	dir_spec[256],
		dir_spec_w[96],
		f_name[64],
		f_ext[64],
		f_ver[64];

	strcpy(dir_spec_w,cur_dir_parms[dir_sw].dir_spec);

	cre_paste_filew(&bgt_disp_id,&dir_disp_id,vtfm_bgt[62]);

	do
	{
		input_w_line(dir_disp_id,1,dir_spec_w,0);
		str_shift(dir_spec_w);

		if ((key_pr == SMG$K_TRM_KP0) ||
			((key_pr == SMG$K_TRM_CR) &&
				(strlen(dir_spec_w) == 0)))
		{
			SMG$DELETE_VIRTUAL_DISPLAY(&dir_disp_id);
			SMG$DELETE_VIRTUAL_DISPLAY(&bgt_disp_id);
					return;
		}
		else
		{
			SMG$SET_CURSOR_ABS(&dir_disp_id,&1,&1);
			cre_s_str_desc(dir_spec_w);
			SMG$PUT_LINE(&dir_disp_id,&s_str_desc);

			SMG$SET_CURSOR_ABS(&comm_w,&1,&comm_col);

			strcpy(dir_spec,dir_spec_w);

			err_sw=scan_file_spec(dir_sw,dir_spec,
						f_name,f_ext,f_ver,0);

			if (err_sw == 0)
			{
				str_toupper(dir_spec);
				strcpy(cur_dir_parms[dir_sw].dir_spec,
								dir_spec);

				SMG$DELETE_VIRTUAL_DISPLAY(&dir_disp_id);
				SMG$DELETE_VIRTUAL_DISPLAY(&bgt_disp_id);

				set_def_dir(cur_dir_parms[dir_sw].dir_spec);
				strcpy(cur_dir_parms[dir_sw].find_spec,
								vtfm_bgt[47]);
				cur_dir_parms[dir_sw].sel_fqua=0;
				strcpy(dir_spec,"\001");
				strcat(dir_spec,vtfm_bgt[49]);
				before_restore(dir_sw);
				restore_dir_cont(dir_sw,dir_spec);
					return;
			}
		}
	}
	while (1 == 1);
}

int get_max_fver(file_spec)
char	*file_spec;
{
	int	c_text=0,
		rms_ret_st,
		max_fver;

	char	in_fspec[256],
		ou_fspec[256],
		*semi_p;


	strcpy(in_fspec,file_spec); semi_p=strchr(in_fspec,';');

	if (semi_p)
		strcpy(semi_p,";*");
	else
		strcat(in_fspec,";*");

	cre_s_str_desc(in_fspec);

	rms_ret_st=LIB$FIND_FILE(&s_str_desc,&d_str_desc,&c_text);

	LIB$FIND_FILE_END(&c_text);

	if (rms_ret_st == RMS$_NORMAL)
	{
		strncpy(ou_fspec,d_str_desc.dsc$a_pointer,
					d_str_desc.dsc$w_length);
		ou_fspec[d_str_desc.dsc$w_length]='\0';
		semi_p=strchr(ou_fspec,';')+1;
		max_fver=atoi(semi_p);
	}
	else
		max_fver=0;

	return max_fver;
}

void edit_file(dir_sw)
int	dir_sw;
{
	int	i,
		err_sw,
		rec_sw,
		parse_ret_st,
		rms_ret_st,
		sys_ret_st,
		bgt_disp_id,
		file_disp_id,
		save_disp_id,
		mess_disp_id,
		cok_disp_id,
		bef_edit_max_fver,
		aft_edit_max_fver;

	char	file_spec_w[96]={'\0'},
		file_spec[96],
		jou_spec[256],
		sysmsg_text[256],
		tpu_comm_line[256],
		*semi_p;

	i=cur_dir_parms[dir_sw].dir_cpos;

	if (cur_dir_cont[dir_sw][i].file_type == 1)
		strcpy(file_spec_w,cur_dir_cont[dir_sw][i].file_spec);

	cre_paste_filew(&bgt_disp_id,&file_disp_id,vtfm_bgt[74]);

	do
	{
		input_w_line(file_disp_id,1,file_spec_w,0);
		str_shift(file_spec_w);

		if ((key_pr == SMG$K_TRM_KP0) ||
			((key_pr == SMG$K_TRM_CR) &&
				(strlen(file_spec_w) == 0)))
		{
			SMG$DELETE_VIRTUAL_DISPLAY(&file_disp_id);
			SMG$DELETE_VIRTUAL_DISPLAY(&bgt_disp_id);
					return;
		}
		else
		{
			SMG$SET_CURSOR_ABS(&file_disp_id,&1,&1);
			cre_s_str_desc(file_spec_w);
			SMG$PUT_LINE(&file_disp_id,&s_str_desc);

			SMG$SET_CURSOR_ABS(&comm_w,&1,&comm_col);

			err_sw=0;

			parse_ret_st=parse_file_spec(file_spec_w);

			if ((parse_ret_st == SS$_NORMAL) &&
				(strpbrk(file_spec_w,":[>*%") == NULL))
			{
				strcpy(file_spec,file_spec_w);

				semi_p=strchr(file_spec,';');

				if (semi_p)
				{
					semi_p++;
					i=atoi(semi_p);
					if (abs(i) > 32767) err_sw=1;
				}
			}
			else
				err_sw=1;

			if (err_sw)
			{
				disp_mess(&mess_disp_id,
					vtfm_bgt[8],vtfm_bgt[21],
					(term_row_qua-4)/2+3,
					str_centr_pos(term_col_qua,
							vtfm_bgt[21])-1);

				SMG$READ_KEYSTROKE(&keyb_id,&key_pr);

				SMG$DELETE_VIRTUAL_DISPLAY(&mess_disp_id);

				SMG$SET_CURSOR_ABS(&comm_w,&1,&comm_col);
			}
			else
			{
				rec_sw=0;

				strcpy(jou_spec,vtfm_bgt[70]);

				sys_ret_st=translate_lnm(jou_spec);

				if (sys_ret_st != SS$_NORMAL)
						strcpy(jou_spec,vtfm_bgt[71]);

				i=strlen(jou_spec);

				if ((jou_spec[i-1] != ':') &&
					(jou_spec[i-1] != ']'))
						strcat(jou_spec,":");

				strcpy(sysmsg_text,file_spec);

				semi_p=strchr(sysmsg_text,';');

				if (semi_p) *semi_p='\0';

				semi_p=strchr(sysmsg_text,'.');

				if (semi_p)
					*semi_p='_';
				else
					strcat(sysmsg_text,"_");

				strcat(strncat(jou_spec,
					sysmsg_text,39),vtfm_bgt[72]);

				rms_ret_st=probe_file_exist(jou_spec);

				if (rms_ret_st == RMS$_NORMAL) rec_sw=1;

				if (rec_sw)
			{
				cok_handler(&cok_disp_id,"",
					vtfm_bgt[76],(term_row_qua-4)/2+3,
					str_centr_pos(term_col_qua,
							vtfm_bgt[76])-1,1);

				SMG$DELETE_VIRTUAL_DISPLAY(&cok_disp_id);

				if (key_pr != SMG$K_TRM_CR) rec_sw=0;
			}

			if (key_pr != SMG$K_TRM_KP0)
			{
				SMG$DELETE_VIRTUAL_DISPLAY(&file_disp_id);
				SMG$DELETE_VIRTUAL_DISPLAY(&bgt_disp_id);

				bef_edit_max_fver=get_max_fver(file_spec);

				strcpy(tpu_comm_line,vtfm_bgt[119]);

				if (rec_sw)
					strcat(tpu_comm_line,vtfm_bgt[121]);
				else
					strcat(tpu_comm_line," ");

				strcat(tpu_comm_line,file_spec);

				SMG$ERASE_PASTEBOARD(&pstb_id);

				SMG$SAVE_PHYSICAL_SCREEN(&pstb_id,
							&save_disp_id);
				CURSOR_TO_HOME;

				printf("%s%s\n",vtfm_bgt[34],"...");

					invoke_tpu(tpu_comm_line);

				SMG$READ_KEYSTROKE(&keyb_id,&key_pr);

				set_term_row_col();

				SMG$RESTORE_PHYSICAL_SCREEN(&pstb_id,
							&save_disp_id);

				SMG$DELETE_VIRTUAL_DISPLAY(&save_disp_id);

				aft_edit_max_fver=get_max_fver(file_spec);

				semi_p=strchr(file_spec,';');

				if (semi_p)
					strcpy(semi_p,";32768");
				else
					strcat(file_spec,";32768");

				if (aft_edit_max_fver != bef_edit_max_fver)
						restore_dir_cont(dir_sw,
								file_spec);
				else
					restore_dir_cont(dir_sw,"");

				return;
			}
			}
		}
	}
	while (1 == 1);
}

void view_file(dir_sw)
int	dir_sw;
{
	int	i,
		save_disp_id,
		bef_edit_max_fver,
		aft_edit_max_fver;

	char	tpu_comm_line[256],
		file_spec[96],
		*semi_p;

	i=cur_dir_parms[dir_sw].dir_cpos;

	if (cur_dir_cont[dir_sw][i].file_type == 1)
	{
		strcpy(file_spec,cur_dir_cont[dir_sw][i].file_spec);

		bef_edit_max_fver=get_max_fver(file_spec);

		strcpy(tpu_comm_line,vtfm_bgt[119]);
		strcat(tpu_comm_line,vtfm_bgt[120]);
		strcat(tpu_comm_line,cur_dir_parms[dir_sw].dir_spec);
		strcat(tpu_comm_line,file_spec);

		SMG$ERASE_PASTEBOARD(&pstb_id);

		SMG$SAVE_PHYSICAL_SCREEN(&pstb_id,&save_disp_id);

		CURSOR_TO_HOME;

		printf("%s%s\n",vtfm_bgt[32],"...");

			invoke_tpu(tpu_comm_line);

		SMG$READ_KEYSTROKE(&keyb_id,&key_pr);

		set_term_row_col();

		SMG$RESTORE_PHYSICAL_SCREEN(&pstb_id,&save_disp_id);

		SMG$DELETE_VIRTUAL_DISPLAY(&save_disp_id);

		aft_edit_max_fver=get_max_fver(file_spec);

		semi_p=strchr(file_spec,';');

		if (semi_p)
			strcpy(semi_p,";32768");
		else
			strcat(file_spec,";32768");

		if (aft_edit_max_fver != bef_edit_max_fver)
				restore_dir_cont(dir_sw,
						file_spec);
		else
			restore_dir_cont(dir_sw,"");
	}
	else
		ring_bell(1);
}

int probe_file_exist(file_spec)
char	*file_spec;
{
	int	c_text=0,
		rms_ret_st;

	cre_s_str_desc(file_spec);

	rms_ret_st=LIB$FIND_FILE(&s_str_desc,&d_str_desc,&c_text);

	LIB$FIND_FILE_END(&c_text);

	return rms_ret_st;
}

void make_dir(dir_sw)
int	dir_sw;
{
	int	a_row,
		a_col,
		err_sw,
		rms_ret_st,
		parse_ret_st,
		stat_ret_st,
		bgt_disp_id,
		dir_disp_id,
		mess_disp_id;

	char	dir_name_w[64]={'\0'},
		sysmsg_text[256],
		dir_spec[256];

	struct	stat file_info;

	SMG$CREATE_VIRTUAL_DISPLAY(&3,&42,&bgt_disp_id,
					&SMG$M_BORDER,&SMG$M_REVERSE);

	disp_w_label(bgt_disp_id,vtfm_bgt[77],SMG$K_TOP,
					str_centr_pos(44,vtfm_bgt[77]));

	SMG$CREATE_VIRTUAL_DISPLAY(&1,&40,&dir_disp_id,
					&SMG$M_NOBORDER,&SMG$M_REVERSE);

	a_row=(term_row_qua-4)/2; a_col=(term_col_qua-42)/2+1;

	SMG$PASTE_VIRTUAL_DISPLAY(&bgt_disp_id,&pstb_id,&a_row,&a_col);

	a_row++; a_col++;

	SMG$PASTE_VIRTUAL_DISPLAY(&dir_disp_id,&pstb_id,&a_row,&a_col);

	SMG$SET_CURSOR_ABS(&comm_w,&1,&comm_col);

	do
	{
		input_w_line(dir_disp_id,1,dir_name_w,0);
		str_shift(dir_name_w);

		if ((key_pr == SMG$K_TRM_KP0) ||
			((key_pr == SMG$K_TRM_CR) &&
				(strlen(dir_name_w) == 0)))
		{
			SMG$DELETE_VIRTUAL_DISPLAY(&dir_disp_id);
			SMG$DELETE_VIRTUAL_DISPLAY(&bgt_disp_id);
					return;
		}
		else
		{
			SMG$SET_CURSOR_ABS(&dir_disp_id,&1,&1);
			cre_s_str_desc(dir_name_w);
			SMG$PUT_LINE(&dir_disp_id,&s_str_desc);

			SMG$SET_CURSOR_ABS(&comm_w,&1,&comm_col);

			parse_ret_st=parse_file_spec(dir_name_w);

			if ((parse_ret_st != SS$_NORMAL) ||
				(strpbrk(dir_name_w,":[]<>.;%*") != NULL))
			{
				disp_mess(&mess_disp_id,
					vtfm_bgt[8],vtfm_bgt[90],
					(term_row_qua-4)/2+3,
					str_centr_pos(term_col_qua,
						vtfm_bgt[90])-1);

				SMG$READ_KEYSTROKE(&keyb_id,&key_pr);

				SMG$DELETE_VIRTUAL_DISPLAY(&mess_disp_id);

				SMG$SET_CURSOR_ABS(&comm_w,&1,&comm_col);
			}
			else
			{
				err_sw=0;		

				strcpy(dir_spec,dir_name_w);
				strcat(dir_spec,vtfm_bgt[26]);
				strcat(dir_spec,";1");
				str_toupper(dir_spec);

				rms_ret_st=probe_file_exist(dir_spec);

				if ((rms_ret_st != RMS$_NORMAL) &&
					(rms_ret_st != RMS$_FNF))
								err_sw=1;
				else
					if (rms_ret_st == RMS$_NORMAL)
			{
				stat_ret_st=stat(dir_spec,&file_info);

				if (stat_ret_st == 0)
				{
					err_sw=(-1);

					if (file_info.st_mode & S_IFDIR)
							strcpy(sysmsg_text,
								vtfm_bgt[80]);
					else
						sprintf(sysmsg_text,
							vtfm_bgt[78],dir_spec);
				}
				else
				{
					err_sw=1;

					if (stat_ret_st == (-2))
						rms_ret_st=RMS$_PRV;
					else
						rms_ret_st=vaxc$errno;
				}
			}
			else
			{
				strcpy(dir_spec,"[.");
				strcat(strcat(dir_spec,dir_name_w),"]");
				str_toupper(dir_spec);

				cre_s_str_desc(dir_spec);

				rms_ret_st=LIB$CREATE_DIR(&s_str_desc,&0,
							&0xFFFF,&0xBA08);

				if (rms_ret_st != SS$_CREATED) err_sw=1;
			}

			if (err_sw)
			{
				if (err_sw == 1)
					get_sysmsg(rms_ret_st,sysmsg_text);

				disp_mess(&mess_disp_id,
					vtfm_bgt[8],sysmsg_text,
					(term_row_qua-4)/2+3,
					str_centr_pos(term_col_qua,
							sysmsg_text)-1);

				SMG$READ_KEYSTROKE(&keyb_id,&key_pr);

				SMG$DELETE_VIRTUAL_DISPLAY(&mess_disp_id);

				SMG$SET_CURSOR_ABS(&comm_w,&1,&comm_col);
			}
			else
			{
				SMG$DELETE_VIRTUAL_DISPLAY(&dir_disp_id);
				SMG$DELETE_VIRTUAL_DISPLAY(&bgt_disp_id);

				strcpy(cur_dir_parms[dir_sw].find_spec,
							vtfm_bgt[47]);
				strcpy(dir_spec,dir_name_w);
				str_toupper(dir_spec);
				strcat(dir_spec,vtfm_bgt[49]);
				before_restore(dir_sw);
				restore_dir_cont(dir_sw,dir_spec);
					return;
			}
			}
		}
	}
	while (1 == 1);
}

void comm_handler(dir_sw)
int	dir_sw;
{
	int	i,
		j,
		k,
		m;

	char	file_spec[256]={'\0','\0'},
		*str_p;

	switch (key_pr)
	{
		case	SMG$K_TRM_CTRLH:
				if (comm_cpos == 0)
					ring_bell(1);
				else
				{
					if (comm_cpos > (comm_col-1))
					{
					SMG$SET_CURSOR_ABS(&comm_w,&1,&1);

					cre_s_str_desc(spawn_comm[0].comm_str);
					SMG$PUT_LINE(&comm_w,&s_str_desc);
					}

					SMG$SET_CURSOR_ABS(&comm_w,&1,&1);

					comm_cpos=0; comm_col=1;
				}
					break;
		case	SMG$K_TRM_CTRLE:
				if (spawn_comm[0].comm_str[comm_cpos] == '\0')
					ring_bell(1);
				else
				{
					i=strlen(spawn_comm[0].comm_str);

					if ((i-comm_cpos) <
						(comm_w_cq-(comm_col-1)))
							comm_col=comm_col+
								(i-comm_cpos);
					else
					{
					str_p=spawn_comm[0].comm_str+
							(i-(comm_w_cq-1));

					SMG$SET_CURSOR_ABS(&comm_w,&1,&1);

					cre_s_str_desc(str_p);
					SMG$PUT_LINE(&comm_w,&s_str_desc);

						comm_col=comm_w_cq;
					}

					SMG$SET_CURSOR_ABS(&comm_w,
								&1,&comm_col);

					comm_cpos=i;
				}
					break;
		case 	SMG$K_TRM_CTRLU:
				if (comm_cpos == 0)
					ring_bell(1);
				else
				{
					i=strlen(spawn_comm[0].comm_str);

				for (j=comm_cpos; j <= i; j++)
					spawn_comm[0].comm_str[j-comm_cpos]=
						spawn_comm[0].comm_str[j];

					SMG$SET_CURSOR_ABS(&comm_w,&1,&1);

					cre_s_str_desc(spawn_comm[0].comm_str);
					SMG$PUT_LINE(&comm_w,&s_str_desc);

					comm_cpos=0; comm_col=1;
				}
					break;
		case	SMG$K_TRM_PF4:
				spawn_comm[0].comm_str[0]='\0';
				comm_cpos=0; comm_col=1; comm_stack_cpos=0;

				SMG$ERASE_DISPLAY(&comm_w);
				SMG$SET_CURSOR_ABS(&comm_w,&1,&1);
					break;
		case	SMG$K_TRM_CTRLD:
		case	SMG$K_TRM_CTRLF:
				k=strlen(spawn_comm[0].comm_str);

			if (key_pr == SMG$K_TRM_CTRLD)
				strcpy(file_spec,
					cur_dir_parms[dir_sw].dir_spec);
			else
			{
				i=cur_dir_parms[dir_sw].dir_cpos;
				strcpy(file_spec,
					cur_dir_cont[dir_sw][i].file_spec);
				strcat(file_spec," ");
			}
				m=strlen(file_spec);


		if ((k+m <= 255) && ((key_pr == SMG$K_TRM_CTRLD) ||
				(cur_dir_cont[dir_sw][i].file_type >= 0)))
		{
			for (j=k; j >= comm_cpos; j--)
				spawn_comm[0].comm_str[j+m]=
					spawn_comm[0].comm_str[j];

			str_p=spawn_comm[0].comm_str+comm_cpos;
			strncpy(str_p,file_spec,m);
			comm_cpos=comm_cpos+m;

			if (comm_col+m > comm_w_cq)
			{
				j=comm_cpos-comm_w_cq+1;
				str_p=spawn_comm[0].comm_str+j;

				SMG$SET_CURSOR_ABS(&comm_w,&1,&1);

				cre_s_str_desc(str_p);
				SMG$PUT_LINE(&comm_w,&s_str_desc);
				SMG$SET_CURSOR_ABS(&comm_w,&1,&comm_w_cq);

				comm_col=comm_w_cq;
			}
			else
			{
				cre_s_str_desc(file_spec);
				SMG$INSERT_CHARS(&comm_w,
						&s_str_desc,&1,&comm_col);

				comm_col=comm_col+m;
			}
		}
		else
			ring_bell(1);
					break;
		case	SMG$K_TRM_LEFT:
				if (comm_cpos > 0)
			{
				comm_cpos--;

			if (comm_col > 1)
			{
				comm_col--;
				SMG$SET_CURSOR_ABS(&comm_w,&1,&comm_col);
			}
			else
			{
				file_spec[0]=
					spawn_comm[0].comm_str[comm_cpos];

				cre_s_str_desc(file_spec);
				SMG$INSERT_CHARS(&comm_w,&s_str_desc,&1,&1);
			}
			}
			else
				ring_bell(1);
					break;
		case	SMG$K_TRM_RIGHT:
				if (spawn_comm[0].comm_str[comm_cpos] != '\0')
			{
				comm_cpos++;

			if (comm_col < comm_w_cq)
			{
				comm_col++;
				SMG$SET_CURSOR_ABS(&comm_w,&1,&comm_col);
			}
			else
			{
				file_spec[0]=
					spawn_comm[0].comm_str[comm_cpos];

				SMG$DELETE_CHARS(&comm_w,&1,&1,&1);

				cre_s_str_desc(file_spec);
				SMG$INSERT_CHARS(&comm_w,
						&s_str_desc,&1,&comm_w_cq);
			}
			}
			else
				ring_bell(1);
					break;
		case	127:				/* DEL-key */
				if (comm_cpos > 0)
			{
				i=comm_cpos;

				do
				{
					spawn_comm[0].comm_str[i-1]=
						spawn_comm[0].comm_str[i];
					i++;
				}
				while (spawn_comm[0].comm_str[i-1] != '\0');

				comm_cpos--;

			if ((comm_col > 2) || (comm_cpos == 0))
			{
				comm_col--;

				SMG$DELETE_CHARS(&comm_w,&1,&1,&comm_col);

				i=comm_w_cq-comm_col+1;

			if (strlen(&spawn_comm[0].comm_str[comm_cpos]) >= i)
			{
				file_spec[0]=
					spawn_comm[0].comm_str[comm_cpos+i-1];

				cre_s_str_desc(file_spec);
				SMG$INSERT_CHARS(&comm_w,
						&s_str_desc,&1,&comm_w_cq);

				SMG$SET_CURSOR_ABS(&comm_w,&1,&comm_col);
			}
			}
			else
			{
				file_spec[0]=
					spawn_comm[0].comm_str[comm_cpos-1];

				cre_s_str_desc(file_spec);
				SMG$PUT_CHARS(&comm_w,&s_str_desc,&1,&1);
			}
			}
			else
				ring_bell(1);
					break;
		case	SMG$K_TRM_CTRLV:
		case	SMG$K_TRM_CTRLN:
				switch (key_pr)
			{
				case	SMG$K_TRM_CTRLV:
						comm_stack_cpos++;

			if (comm_stack_cpos > comm_stack_qua)
			{
				spawn_comm[0].comm_str[0]='\0';
				comm_stack_cpos=comm_stack_qua+1;
				ring_bell(1);
			}
			else
				spawn_comm[0]=spawn_comm[comm_stack_cpos];
					break;

				case	SMG$K_TRM_CTRLN:
						comm_stack_cpos--;

			if (comm_stack_cpos < 1)
			{
				spawn_comm[0].comm_str[0]='\0';
				comm_stack_cpos=0;
				ring_bell(1);
			}
			else
				spawn_comm[0]=spawn_comm[comm_stack_cpos];
			}

				SMG$SET_CURSOR_ABS(&comm_w,&1,&1);

				cre_s_str_desc(spawn_comm[0].comm_str);
				SMG$PUT_LINE(&comm_w,&s_str_desc);
				SMG$SET_CURSOR_ABS(&comm_w,&1,&1);

				comm_cpos=0; comm_col=1;
					break;
		default:
			if (((key_pr > 31) && (key_pr < 127)) ||
				((key_pr > 160) && (key_pr < 164)) ||
				 (key_pr == 165) ||
				((key_pr > 166) && (key_pr < 172)) ||
				((key_pr > 175) && (key_pr < 180)) ||
				((key_pr > 180) && (key_pr < 184)) ||
				((key_pr > 184) && (key_pr < 190)) ||
				((key_pr > 190) && (key_pr < 208)) ||
				((key_pr > 208) && (key_pr < 222)) ||
				((key_pr > 222) && (key_pr < 240)) ||
				((key_pr > 240) && (key_pr < 254)))
			{
				k=strlen(spawn_comm[0].comm_str);

			if (k+1 <= 255)
			{
				for (j=k; j >= comm_cpos; j--)
					spawn_comm[0].comm_str[j+1]=
						spawn_comm[0].comm_str[j];

				comm_cpos++;

				spawn_comm[0].comm_str[comm_cpos-1]=key_pr;
				file_spec[0]=key_pr;
				cre_s_str_desc(file_spec);

				if (comm_col < comm_w_cq)
				{
					i=comm_col;
					comm_col++;
				}
				else
				{
					SMG$DELETE_CHARS(&comm_w,&1,&1,&1);

					i=comm_w_cq-1;
				}

				SMG$INSERT_CHARS(&comm_w,&s_str_desc,&1,&i);
			}
			else
				ring_bell(1);
			}
	}
}

int copy_proc(in_file,out_file,diag_disp_id,line_nbr)
char	*in_file,
	*out_file;
int	diag_disp_id,
	line_nbr;
{
	char	fills_str[96]={'\0'};

	int	i,
		diag_col=2,
		rms_ret_st;

	double	x,
		y,
		z,
		fills_per_io,
		disp_fills_qua=0;

	struct	FAB	in_fab,
			out_fab;

	struct	XABFHC	in_xabfhc,
			out_xabfhc;

	struct	XABSUM	in_xabsum,
			out_xabsum;

	struct	RAB	in_rab,
			out_rab;

	for (i=1; i <= 74; i++)
			strcat(fills_str,"%");

	x=74; y=REC_BUFF_SIZE; z=file_info.st_size;

	if (file_info.st_size)
			fills_per_io=x*y/z;

	in_fab=cc$rms_fab;
	in_rab=cc$rms_rab;
	in_xabfhc=cc$rms_xabfhc;
	in_xabsum=cc$rms_xabsum;

	in_fab.fab$l_fna=in_file;
	in_fab.fab$b_fns=strlen(in_file);
	in_fab.fab$b_fac=FAB$M_BIO | FAB$M_GET;
	in_fab.fab$l_xab=(&in_xabfhc);

	in_rab.rab$l_fab=(&in_fab);
	in_rab.rab$l_bkt=0;
	in_rab.rab$l_ubf=rec_buff;
	in_rab.rab$w_usz=REC_BUFF_SIZE;

	in_xabfhc.xab$l_nxt=(&in_xabsum);

	rms_ret_st=SYS$OPEN(&in_fab);

	if (rms_ret_st == RMS$_NORMAL)
	{
		SYS$CONNECT(&in_rab);

	out_fab=in_fab;
	out_rab=in_rab;
	out_xabfhc=in_xabfhc;
	out_xabsum=in_xabsum;

	out_fab.fab$l_fna=out_file;
	out_fab.fab$b_fns=strlen(out_file);
	out_fab.fab$w_ifi=cc$rms_fab.fab$w_ifi;
	out_fab.fab$b_fac=FAB$M_BIO | FAB$M_PUT;
	out_fab.fab$l_xab=(&out_xabfhc);

	out_rab.rab$l_fab=(&out_fab);
	out_rab.rab$l_bkt=0;
	out_rab.rab$l_rbf=rec_buff;
	out_rab.rab$w_isi=cc$rms_rab.rab$w_isi;

	out_xabfhc.xab$l_nxt=(&out_xabsum);

	rms_ret_st=SYS$CREATE(&out_fab);

	if (rms_ret_st == RMS$_NORMAL)
	{
		SYS$CONNECT(&out_rab);

	do
	{
		rms_ret_st=SYS$READ(&in_rab);

	if (rms_ret_st == RMS$_NORMAL)
	{
		out_rab.rab$w_rsz=in_rab.rab$w_rsz;

		rms_ret_st=SYS$WRITE(&out_rab);

		if (rms_ret_st == RMS$_NORMAL)
		{
			disp_fills_qua=disp_fills_qua+fills_per_io;

			if ((disp_fills_qua >= 1) && (diag_col < 76))
			{
				i=floor(disp_fills_qua);
				disp_fills_qua=disp_fills_qua-i;

				if (i > 76-diag_col)
						i=76-diag_col;

				fills_str[i]='\0';

				cre_s_str_desc(fills_str);
				SMG$PUT_CHARS(&diag_disp_id,
					&s_str_desc,&line_nbr,&diag_col);

				fills_str[i]='%'; diag_col=diag_col+i;

				SMG$SET_CURSOR_ABS(&comm_w,&1,&comm_col);
			}
		}
	}
	}
	while (rms_ret_st == RMS$_NORMAL);

		SYS$CLOSE(&out_fab);

		if (rms_ret_st == RMS$_EOF)
		{
			if (diag_col < 76)
			{
				fills_str[76-diag_col]='\0';

				cre_s_str_desc(fills_str);
				SMG$PUT_CHARS(&diag_disp_id,
					&s_str_desc,&line_nbr,&diag_col);

				SMG$SET_CURSOR_ABS(&comm_w,&1,&comm_col);
			}

			rms_ret_st=0;
		}
	}
		SYS$CLOSE(&in_fab);
	}

	return rms_ret_st;
}

int copmov_subdir_tree(bgt_did_pp,file_did_pp,
				src_subdir_spec,dest_subdir_spec,corm_sw)
char	*src_subdir_spec,
	*dest_subdir_spec;
int	*bgt_did_pp,
	*file_did_pp,
	corm_sw;	/* 0 - Copy files, Other - Move files via Copy */
{
	int	i,
		err_sw,
		c_text=0,
		stat_ret_st,
		rms_ret_st,
		sd_rms_ret_st,
		cmsd_ret_st,
		cok_disp_id,
		mess_disp_id;

	char	sysmsg_text[256],
		file_spec[256],
		find_spec[256],
		fspec[96],
		*brack_p,
		*semi_p;

	struct	dsc$descriptor_s all_s_str_desc;
	struct	stat x_file_info;
	struct	FAB 	src_fab,
			dest_fab;

	strcat(strcpy(find_spec,src_subdir_spec),vtfm_bgt[47]);
	cre_s_str_desc(find_spec);
	all_s_str_desc=s_str_desc;

	SMG$ERASE_CHARS(file_did_pp,&74,&1,&1);
	SMG$ERASE_CHARS(bgt_did_pp,&76,&3,&1);
	SMG$SET_CURSOR_ABS(&comm_w,&1,&comm_col);

	i=0;

	do
	{
		cmsd_ret_st=0;

		rms_ret_st=LIB$FIND_FILE(&all_s_str_desc,&d_str_desc,&c_text);

	switch (rms_ret_st)
	{
		case	RMS$_NORMAL:
				i++; err_sw=0;

			if (i == 1)
			{
			        SMG$SET_CURSOR_ABS(bgt_did_pp,&1,&1);

				cre_s_str_desc(src_subdir_spec);
				SMG$PUT_LINE(bgt_did_pp,&s_str_desc);

				SMG$ERASE_CHARS(file_did_pp,&76,&1,&1);
        			SMG$SET_CURSOR_ABS(&comm_w,&1,&comm_col);
			}
				strncpy(file_spec,
					d_str_desc.dsc$a_pointer,
					d_str_desc.dsc$w_length);
				file_spec[d_str_desc.dsc$w_length]='\0';

				stat_ret_st=stat(file_spec,&file_info);

		if (stat_ret_st == 0)
		{
			if (file_info.st_mode & S_IFDIR)
			{
				strcpy(sysmsg_text,dest_subdir_spec);
				strcat(sysmsg_text,strchr(file_spec,']')+1);

				brack_p=strchr(sysmsg_text,']');
				strcpy(strchr(brack_p,'.'),vtfm_bgt[26]);
				strcat(sysmsg_text,";1");

				sd_rms_ret_st=probe_file_exist(sysmsg_text);

				if ((sd_rms_ret_st != RMS$_NORMAL) &&
					(sd_rms_ret_st != RMS$_FNF))
								err_sw=1;
				else
					if (sd_rms_ret_st == RMS$_NORMAL)
			{
				x_file_info=file_info;

				stat_ret_st=stat(sysmsg_text,&file_info);

				if (stat_ret_st == 0)
				{
					if (file_info.st_mode & S_IFDIR)
					{
						brack_p=strchr(sysmsg_text,
									']');
						*brack_p='.'; brack_p++;
						semi_p=strchr(brack_p,'.');
						strcpy(semi_p,"]");
						strcpy(dest_subdir_spec,
								sysmsg_text);

						file_info=x_file_info;
					}
					else
					{
						err_sw=(-1);

						strcpy(fspec,
							strchr(sysmsg_text,
								']')+1);
						sprintf(sysmsg_text,
							vtfm_bgt[78],fspec);
					}
				}
				else
				{
					err_sw=1;

					if (stat_ret_st == (-2))
						sd_rms_ret_st=RMS$_PRV;
					else
						sd_rms_ret_st=vaxc$errno;
				}
			}
			else
			{
				brack_p=strchr(sysmsg_text,']');
				*brack_p='.'; brack_p++;
				semi_p=strchr(brack_p,'.');
				strcpy(semi_p,"]");
				strcpy(dest_subdir_spec,sysmsg_text);

				cre_s_str_desc(dest_subdir_spec);

				sd_rms_ret_st=LIB$CREATE_DIR(&s_str_desc,&0,
							&0xFFFF,&0xBA08);

				if (sd_rms_ret_st != SS$_CREATED)
				{
					err_sw=1;

					brack_p=strchr(dest_subdir_spec,']');

					do
						brack_p--;
					while (*brack_p != '.');

					strcpy(brack_p,"]");
				}
			}

			if (err_sw == 0)
			{
				strcpy(sysmsg_text,strchr(file_spec,']')+1);
				strcpy(strchr(sysmsg_text,'.'),"]");
				strcpy(strchr(src_subdir_spec,']'),".");
				strcat(src_subdir_spec,sysmsg_text);

				x_file_info=file_info;

				cmsd_ret_st=copmov_subdir_tree(bgt_did_pp,
						file_did_pp,src_subdir_spec,
						dest_subdir_spec,corm_sw);

				file_info=x_file_info;

				brack_p=strchr(src_subdir_spec,']');

				do
					brack_p--;
				while (*brack_p != '.');

				strcpy(brack_p,"]");

				brack_p=strchr(dest_subdir_spec,']');

				do
					brack_p--;
				while (*brack_p != '.');

				strcpy(brack_p,"]");

			if (cmsd_ret_st != (-1))
			{
			        SMG$SET_CURSOR_ABS(bgt_did_pp,&1,&1);

				cre_s_str_desc(src_subdir_spec);
				SMG$PUT_LINE(bgt_did_pp,&s_str_desc);

				if ((cmsd_ret_st) && (cmsd_ret_st != RMS$_DNF))
				{
					sd_rms_ret_st=cmsd_ret_st;
					err_sw=1;
				}
			}
                        }
			}

			if ((err_sw == 0) && (cmsd_ret_st != (-1)))
			{
				if ((corm_sw) ||
					(!(file_info.st_mode & S_IFDIR)))
			{
				cre_s_str_desc(strchr(file_spec,']')+1);
				SMG$PUT_LINE(file_did_pp,&s_str_desc);

                		SMG$SET_CURSOR_ABS(&comm_w,&1,&comm_col);
			}

				if (!(file_info.st_mode & S_IFDIR))
			{
				strcat(dest_subdir_spec,
						strchr(file_spec,']')+1);

				err_sw=copy_proc(file_spec,
						dest_subdir_spec,
							*bgt_did_pp,3);

				strcpy(strchr(dest_subdir_spec,']')+1,"");
			}

			if (err_sw)
			{
				sd_rms_ret_st=err_sw;
				err_sw=(-2);
			}
			else
			{
				if (!(file_info.st_mode & S_IFDIR))
			{
				SMG$ERASE_CHARS(bgt_did_pp,&76,&3,&1);
        			SMG$SET_CURSOR_ABS(&comm_w,&1,&comm_col);
			}

				if (corm_sw)
			{
				src_fab=cc$rms_fab;
				src_fab.fab$l_fna=file_spec;
				src_fab.fab$b_fns=strlen(file_spec);
				src_fab.fab$w_ifi=0;

				sd_rms_ret_st=SYS$ERASE(&src_fab);

				if (sd_rms_ret_st != RMS$_NORMAL)
				{
					err_sw=(-2);

					if (sd_rms_ret_st == RMS$_MKD)
						sd_rms_ret_st=
							src_fab.fab$l_stv;
				}
			}
			}
			}
		}
		else
		{
			err_sw=1;

			if (stat_ret_st == (-2))
					sd_rms_ret_st=RMS$_PRV;
			else
				sd_rms_ret_st=vaxc$errno;
		}

		if (cmsd_ret_st != (-1))
		{
			cmsd_ret_st=0;

			if (abs(err_sw) == 1)
		{
			cre_s_str_desc(strchr(file_spec,']')+1);
			SMG$PUT_LINE(file_did_pp,&s_str_desc);

                	SMG$SET_CURSOR_ABS(&comm_w,&1,&comm_col);
		}

		if (err_sw)
		{
			if (err_sw != (-1))
				get_sysmsg(sd_rms_ret_st,sysmsg_text);

			disp_mess(&mess_disp_id,
				vtfm_bgt[8],sysmsg_text,
				(term_row_qua-4)/2+3,
				str_centr_pos(term_col_qua,sysmsg_text)-1);

			SMG$READ_KEYSTROKE(&keyb_id,&key_pr);

			SMG$DELETE_VIRTUAL_DISPLAY(&mess_disp_id);

			SMG$SET_CURSOR_ABS(&comm_w,&1,&comm_col);

			cok_handler(&cok_disp_id,"",
				vtfm_bgt[75],(term_row_qua-4)/2+3,
				str_centr_pos(term_col_qua,vtfm_bgt[75])-1,1);

			SMG$DELETE_VIRTUAL_DISPLAY(&cok_disp_id);

			SMG$ERASE_CHARS(bgt_did_pp,&76,&3,&1);
       			SMG$SET_CURSOR_ABS(&comm_w,&1,&comm_col);

			if (key_pr != SMG$K_TRM_CR) cmsd_ret_st=(-1);
		}
		}
				break;
		case	RMS$_FNF:
		case	RMS$_NMF:
				break;
		default:
				cmsd_ret_st=rms_ret_st;
	}
	}
	while ((cmsd_ret_st == 0) &&
		(rms_ret_st != RMS$_FNF) && (rms_ret_st != RMS$_NMF));

	LIB$FIND_FILE_END(&c_text);

	return cmsd_ret_st;
}

int get_phys_disk_name(disk_name,phys_disk_name)
char	*disk_name,
	phys_disk_name[];
{
	int	dev_class,
		sys_ret_st;

	cre_s_str_desc(disk_name);

	sys_ret_st=LIB$GETDVI(&DVI$_DEVCLASS,
				0,&s_str_desc,&dev_class);

	if (sys_ret_st == SS$_NORMAL)
	{
		if (dev_class == DC$_DISK)
		{
			LIB$GETDVI(&DVI$_ALLDEVNAM,
					0,&s_str_desc,0,&d_str_desc);

			strncpy(phys_disk_name,
				d_str_desc.dsc$a_pointer,
					d_str_desc.dsc$w_length);

			phys_disk_name[d_str_desc.dsc$w_length]='\0';
		}
		else
			sys_ret_st=RMS$_DEV;
	}
	else
		if (sys_ret_st == SS$_NOSUCHDEV)
					sys_ret_st=RMS$_DNR;

	return sys_ret_st;
}

void copy_renmov_files(dir_sw,corm_sw)
int	dir_sw,
	corm_sw;	/* 0 - Copy files, 1 - RenMov files */
{
	int	i,
		alt_dir,
		err_sw,
		sel_fqua,
		corm_fqua=0,
		stat_ret_st,
		sys_ret_st,
		rms_ret_st,
		cmsd_ret_st,
		bgt_disp_id,
		file_disp_id,
		cok_disp_id,
		mess_disp_id;

	char	dir_spec_w[96],
		src_phys_disk_name[96],
		dest_phys_disk_name[96],
		src_subdir_spec[256],
		dest_subdir_spec[256],
		file_spec[256],
		sysmsg_text[256],
		src_exp_str[256],
		src_res_str[256],
		dest_exp_str[256],
		dest_res_str[256],
		f_name[64],
		f_ext[64],
		f_ver[64],
		fspec[96],
		*brack_p,
		*semi_p;

	struct	dsc$descriptor_s inp_file_desc;
	struct	dsc$descriptor_s out_file_desc;
	struct	stat x_file_info;

	struct	FAB 	src_fab,
			dest_fab;

	struct	NAM 	src_nam,
			dest_nam;

	i=cur_dir_parms[dir_sw].dir_cpos;

	if (cur_dir_parms[dir_sw].sel_fqua)
	{
		if (corm_sw)
			sprintf(sysmsg_text,"%s %u %s",vtfm_bgt[81],
				cur_dir_parms[dir_sw].sel_fqua,vtfm_bgt[83]);
		else
			sprintf(sysmsg_text,"%s %u %s",vtfm_bgt[86],
				cur_dir_parms[dir_sw].sel_fqua,vtfm_bgt[83]);

		if (cur_dir_parms[dir_sw].sel_fqua > 1)
					strcat(sysmsg_text,"S");

		strcat(sysmsg_text,vtfm_bgt[84]);
	}
	else
		if (cur_dir_cont[dir_sw][i].file_type >= 0)
	{
		if (corm_sw)
			sprintf(sysmsg_text,"%s%s%s",vtfm_bgt[81],
					vtfm_bgt[82],vtfm_bgt[84]);
		else
			sprintf(sysmsg_text,"%s%s%s",vtfm_bgt[86],
					vtfm_bgt[82],vtfm_bgt[84]);
	}
	else
	{
		ring_bell(1);
		return;
	}

	alt_dir=(dir_sw+1)%2;
	strcpy(dir_spec_w,cur_dir_parms[alt_dir].dir_spec);
	strcat(dir_spec_w,vtfm_bgt[47]);

	cre_paste_filew(&bgt_disp_id,&file_disp_id,sysmsg_text);

	do
	{
		input_w_line(file_disp_id,1,dir_spec_w,0);
		str_shift(dir_spec_w);

		if ((key_pr == SMG$K_TRM_KP0) ||
			((key_pr == SMG$K_TRM_CR) &&
				(strlen(dir_spec_w) == 0)))
	{
		SMG$DELETE_VIRTUAL_DISPLAY(&file_disp_id);
		SMG$DELETE_VIRTUAL_DISPLAY(&bgt_disp_id);
				return;
	}
	else
	{
		SMG$SET_CURSOR_ABS(&file_disp_id,&1,&1);
		cre_s_str_desc(dir_spec_w);
		SMG$PUT_LINE(&file_disp_id,&s_str_desc);

		SMG$SET_CURSOR_ABS(&comm_w,&1,&comm_col);

		strcpy(dest_subdir_spec,dir_spec_w);

		err_sw=scan_file_spec(dir_sw,dest_subdir_spec,
						f_name,f_ext,f_ver,1);

	if (err_sw == 0)
	{
		brack_p=strchr(cur_dir_parms[dir_sw].dir_spec,'[');
		*brack_p='\0';

		semi_p=strchr(dest_subdir_spec,'[');
		*semi_p='\0';

		sys_ret_st=get_phys_disk_name(cur_dir_parms[dir_sw].dir_spec,
							src_phys_disk_name);

		sys_ret_st=get_phys_disk_name(dest_subdir_spec,
							dest_phys_disk_name);

					/* Diff dev - RenMov via Copy */
		if ((corm_sw) &&
			(strcmp(src_phys_disk_name,dest_phys_disk_name)))
								corm_sw=(-1);

		*brack_p='['; *semi_p='[';

		if (cur_dir_parms[dir_sw].sel_fqua == 0)
			cur_dir_cont[dir_sw][i].file_selp='*';

		sel_fqua=cur_dir_parms[dir_sw].sel_fqua;

		i=(-1);

		do
			i++;
		while (cur_dir_cont[dir_sw][i].file_selp != '*');

		if (corm_sw)
			disp_w_label(bgt_disp_id,vtfm_bgt[85],SMG$K_TOP,
					str_centr_pos(78,vtfm_bgt[85]));
		else
			disp_w_label(bgt_disp_id,vtfm_bgt[88],SMG$K_TOP,
					str_centr_pos(78,vtfm_bgt[88]));
	do
	{
		cmsd_ret_st=0;

		if (cur_dir_cont[dir_sw][i].file_selp == '*')
		{
			sel_fqua--; err_sw=0;

			strcpy(file_spec,cur_dir_parms[dir_sw].dir_spec);
			strcat(file_spec,cur_dir_cont[dir_sw][i].file_spec);

			stat_ret_st=stat(file_spec,&file_info);

		if (stat_ret_st == 0)
		{
			if ((corm_sw != 1) &&
				(file_info.st_mode & S_IFDIR))
		{
				strcpy(sysmsg_text,dest_subdir_spec);

			if (strchr(f_name,'*'))
		{
			brack_p=strchr(cur_dir_cont[dir_sw][i].file_spec,'.');
			*brack_p='\0';
			strcat(sysmsg_text,cur_dir_cont[dir_sw][i].file_spec);
		      	*brack_p='.';
		}
		else
			strcat(sysmsg_text,f_name);

				strcat(strcat(sysmsg_text,vtfm_bgt[26]),";1");

				rms_ret_st=probe_file_exist(sysmsg_text);

				if ((rms_ret_st != RMS$_NORMAL) &&
					(rms_ret_st != RMS$_FNF))
								err_sw=1;
				else
					if (rms_ret_st == RMS$_NORMAL)
			{
				x_file_info=file_info;

				stat_ret_st=stat(sysmsg_text,&file_info);

				if (stat_ret_st == 0)
				{
					if (file_info.st_mode & S_IFDIR)
					{

						brack_p=strchr(sysmsg_text,
									']');
						*brack_p='.'; brack_p++;
						semi_p=strchr(brack_p,'.');
						strcpy(semi_p,"]");
						strcpy(dest_subdir_spec,
								sysmsg_text);

						file_info=x_file_info;
					}
					else
					{
						err_sw=(-1);

						strcpy(fspec,
							strchr(sysmsg_text,
								']')+1);
						sprintf(sysmsg_text,
							vtfm_bgt[78],fspec);
					}
				}
				else
				{
					err_sw=1;

					if (stat_ret_st == (-2))
						rms_ret_st=RMS$_PRV;
					else
						rms_ret_st=vaxc$errno;
				}
			}
			else
			{
				brack_p=strchr(sysmsg_text,']');
				*brack_p='.'; brack_p++;
				semi_p=strchr(brack_p,'.');
				strcpy(semi_p,"]");
				strcpy(dest_subdir_spec,sysmsg_text);

				cre_s_str_desc(dest_subdir_spec);

				rms_ret_st=LIB$CREATE_DIR(&s_str_desc,&0,
							&0xFFFF,&0xBA08);

				if (rms_ret_st != SS$_CREATED)
				{
					err_sw=1;

					brack_p=strchr(dest_subdir_spec,']');

					do
						brack_p--;
					while (*brack_p != '.');

					strcpy(brack_p,"]");
				}
				else
					corm_fqua++;
			}

			if (err_sw == 0)
			{
				strcpy(src_subdir_spec,"[.");
				strcpy(sysmsg_text,
					cur_dir_cont[dir_sw][i].file_spec);
				strcpy(strchr(sysmsg_text,'.'),"]");
				strcat(src_subdir_spec,sysmsg_text);

				x_file_info=file_info;

				cmsd_ret_st=copmov_subdir_tree(&bgt_disp_id,
						&file_disp_id,src_subdir_spec,
						dest_subdir_spec,corm_sw);

				file_info=x_file_info;

				brack_p=strchr(dest_subdir_spec,']');

				do
					brack_p--;
				while (*brack_p != '.');

				strcpy(brack_p,"]");

			if (cmsd_ret_st != (-1))
			{
				SMG$ERASE_CHARS(&bgt_disp_id,&76,&1,&1);
			        SMG$SET_CURSOR_ABS(&comm_w,&1,&comm_col);

				if ((cmsd_ret_st) && (cmsd_ret_st != RMS$_DNF))
				{
					rms_ret_st=cmsd_ret_st;
					err_sw=1;
				}
			}
			}
		}
		else
		{
			if (strchr(f_name,'*'))
		{
			brack_p=strchr(cur_dir_cont[dir_sw][i].file_spec,'.');
			*brack_p='\0';
			strcat(dest_subdir_spec,
				cur_dir_cont[dir_sw][i].file_spec);
			*brack_p='.';
		}
		else
			strcat(dest_subdir_spec,f_name);

			if (strchr(f_ext,'*'))
		{
			brack_p=strchr(cur_dir_cont[dir_sw][i].file_spec,'.');
			semi_p=strchr(brack_p,';'); *semi_p='\0';
			strcat(dest_subdir_spec,brack_p); *semi_p=';';
		}
		else
			strcat(dest_subdir_spec,f_ext);

			if (strchr(f_ver,'*'))
		{
			semi_p=strchr(cur_dir_cont[dir_sw][i].file_spec,';');
			strcat(dest_subdir_spec,semi_p);
		}
		else
			strcat(dest_subdir_spec,f_ver);

			str_toupper(dest_subdir_spec);
		}

		if ((err_sw == 0) && (cmsd_ret_st != (-1)))
		{
				if ((corm_sw) ||
					(!(file_info.st_mode & S_IFDIR)))
			{
				cre_s_str_desc(strchr(file_spec,']')+1);
				SMG$PUT_LINE(&file_disp_id,&s_str_desc);

                		SMG$SET_CURSOR_ABS(&comm_w,&1,&comm_col);
			}

				if ((corm_sw != 1) &&
					(!(file_info.st_mode & S_IFDIR)))
			{
				err_sw=copy_proc(file_spec,dest_subdir_spec,
								bgt_disp_id,3);
				if (err_sw)
				{
					rms_ret_st=err_sw;
					err_sw=(-2);
				}
				else
				{
					SMG$ERASE_CHARS(&bgt_disp_id,
								&76,&3,&1);
        				SMG$SET_CURSOR_ABS(&comm_w,
								&1,&comm_col);
					corm_fqua++;
				}
			}
			else
				if (corm_sw == 1)
			{
				src_fab=cc$rms_fab;
				src_fab.fab$l_fna=file_spec;
				src_fab.fab$b_fns=strlen(file_spec);
				src_fab.fab$w_ifi=0;
				src_fab.fab$l_nam=&src_nam;

				src_nam=cc$rms_nam;
				src_nam.nam$l_esa=src_exp_str;
				src_nam.nam$b_ess=NAM$C_MAXRSS;
				src_nam.nam$l_rsa=src_res_str;
				src_nam.nam$b_rss=NAM$C_MAXRSS;

				dest_fab=cc$rms_fab;
				dest_fab.fab$l_fna=dest_subdir_spec;
				dest_fab.fab$b_fns=strlen(dest_subdir_spec);
				dest_fab.fab$w_ifi=0;
				dest_fab.fab$l_nam=&dest_nam;

				dest_nam=cc$rms_nam;
				dest_nam.nam$l_esa=dest_exp_str;
				dest_nam.nam$b_ess=NAM$C_MAXRSS;
				dest_nam.nam$l_rsa=dest_res_str;
				dest_nam.nam$b_rss=NAM$C_MAXRSS;

				rms_ret_st=SYS$RENAME(&src_fab,0,0,&dest_fab);

				if (rms_ret_st != RMS$_NORMAL)
				{
					err_sw=(-2);

					if ((rms_ret_st == RMS$_ENT) ||
						(rms_ret_st == RMS$_RMV))
							rms_ret_st=
							src_fab.fab$l_stv;
				}
				else
					corm_fqua++;
			}

				if ((err_sw == 0) && (corm_sw == (-1)))
			{
				src_fab=cc$rms_fab;
				src_fab.fab$l_fna=file_spec;
				src_fab.fab$b_fns=strlen(file_spec);
				src_fab.fab$w_ifi=0;

				rms_ret_st=SYS$ERASE(&src_fab);

				if (rms_ret_st != RMS$_NORMAL)
				{
					err_sw=(-2);

					if (rms_ret_st == RMS$_MKD)
						rms_ret_st=
							src_fab.fab$l_stv;
				}
			}
		}
			strcpy(strchr(dest_subdir_spec,']')+1,"");
		}
		else
		{
			err_sw=1;

			if (stat_ret_st == (-2))
					rms_ret_st=RMS$_PRV;
			else
				rms_ret_st=vaxc$errno;
		}

		if (cmsd_ret_st != (-1))
		{
			cmsd_ret_st=0;

			if (abs(err_sw) == 1)
		{
			cre_s_str_desc(strchr(file_spec,']')+1);
			SMG$PUT_LINE(&file_disp_id,&s_str_desc);

                	SMG$SET_CURSOR_ABS(&comm_w,&1,&comm_col);
		}

		if (err_sw)
		{
			if (cur_dir_parms[dir_sw].sel_fqua == 0)
				cur_dir_cont[dir_sw][i].file_selp=' ';

			if (err_sw != (-1))
				get_sysmsg(rms_ret_st,sysmsg_text);

			disp_mess(&mess_disp_id,
				vtfm_bgt[8],sysmsg_text,
				(term_row_qua-4)/2+3,
				str_centr_pos(term_col_qua,sysmsg_text)-1);

			SMG$READ_KEYSTROKE(&keyb_id,&key_pr);

			SMG$DELETE_VIRTUAL_DISPLAY(&mess_disp_id);

			SMG$SET_CURSOR_ABS(&comm_w,&1,&comm_col);

			if (sel_fqua > 0)
			{
				cok_handler(&cok_disp_id,"",
					vtfm_bgt[75],(term_row_qua-4)/2+3,
					str_centr_pos(term_col_qua,
							vtfm_bgt[75])-1,1);

				SMG$DELETE_VIRTUAL_DISPLAY(&cok_disp_id);

				SMG$ERASE_CHARS(&bgt_disp_id,&76,&3,&1);
        			SMG$SET_CURSOR_ABS(&comm_w,&1,&comm_col);

				if (key_pr != SMG$K_TRM_CR) sel_fqua=0;
			}
		}
		else
		{
			cur_dir_cont[dir_sw][i].file_selp=' ';

			if (cur_dir_parms[dir_sw].sel_fqua)
				cur_dir_parms[dir_sw].sel_fqua--;
		}
		}
		}
			i++;
	}
	while ((cmsd_ret_st == 0) && (sel_fqua > 0));

		SMG$DELETE_VIRTUAL_DISPLAY(&file_disp_id);
		SMG$DELETE_VIRTUAL_DISPLAY(&bgt_disp_id);

		if (corm_fqua)
			restore_dir_cont(dir_sw,"");

		return;
	}
	}
	}
	while (1 == 1);
}

void fkey_desc(fkey_win_rqua)
int	fkey_win_rqua;
{
	int	i,
		a_row,
		a_col,
		fkey_disp_id,
		pak_disp_id;


	SMG$CREATE_VIRTUAL_DISPLAY(&fkey_win_rqua,&68,&fkey_disp_id,
				&SMG$M_BORDER,&SMG$M_REVERSE);

	SMG$CREATE_VIRTUAL_DISPLAY(&1,&18,&pak_disp_id,
				&SMG$M_NOBORDER,&SMG$M_NORMAL);

	SMG$SET_CURSOR_ABS(&fkey_disp_id,&2,&1);

	for (i=93; i <= 107; i++)
	{
		cre_s_str_desc(vtfm_bgt[i]);
		SMG$PUT_LINE(&fkey_disp_id,&s_str_desc);
	}

	disp_w_label(fkey_disp_id,vtfm_bgt[92],SMG$K_TOP,
				str_centr_pos(70,vtfm_bgt[92]));

	cre_s_str_desc(vtfm_bgt[8]);
	SMG$PUT_LINE(&pak_disp_id,&s_str_desc);

	a_row=((term_row_qua-
		((fkey_win_rqua+2)-(fkey_win_rqua%2)))/2)-(1-(fkey_win_rqua%2));

	a_col=(term_col_qua-68)/2+1;

	SMG$PASTE_VIRTUAL_DISPLAY(&fkey_disp_id,&pstb_id,&a_row,&a_col);

	a_row=a_row+fkey_win_rqua;

	SMG$PASTE_VIRTUAL_DISPLAY(&pak_disp_id,&pstb_id,&a_row,&a_col);

	SMG$SET_CURSOR_ABS(&comm_w,&1,&comm_col);

	SMG$READ_KEYSTROKE(&keyb_id,&key_pr);

	SMG$DELETE_VIRTUAL_DISPLAY(&pak_disp_id);

	SMG$DELETE_VIRTUAL_DISPLAY(&fkey_disp_id);
}

int parse_file_spec(file_spec)
char	*file_spec;
{
	int	tpa_ret_st;

	struct	tpadef	tpa_arg_block;

	tpa_arg_block.tpa$l_count=TPA$K_COUNT0;
	tpa_arg_block.tpa$l_stringcnt=strlen(file_spec);
	tpa_arg_block.tpa$l_stringptr=(int)file_spec;

	tpa_ret_st=LIB$TABLE_PARSE(&tpa_arg_block,
				&FSPEC_ST_TBL,&FSPEC_KW_TBL);

	return tpa_ret_st;
}

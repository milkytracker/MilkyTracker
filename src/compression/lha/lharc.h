
/*----------------------------------------------------------------------*/
/*			OTHER DIFINITIONS				*/
/*----------------------------------------------------------------------*/

#define FILENAME_LENGTH	1024

/*----------------------------------------------------------------------*/
/*			LHarc FILE DEFINITIONS				*/
/*----------------------------------------------------------------------*/
#define METHOD_TYPE_STRAGE	5
#define LZHUFF0_METHOD		"-lh0-"
#define LZHUFF1_METHOD		"-lh1-"
#define LZHUFF2_METHOD		"-lh2-"
#define LZHUFF3_METHOD		"-lh3-"
#define LZHUFF4_METHOD		"-lh4-"
#define LZHUFF5_METHOD		"-lh5-"
#define LARC4_METHOD		"-lz4-"
#define LARC5_METHOD		"-lz5-"
#define LZHDIRS_METHOD		"-lhd-"

#define I_HEADER_SIZE			0
#define I_HEADER_CHECKSUM		1
#define I_METHOD				2
#define I_PACKED_SIZE			7
#define I_ORIGINAL_SIZE			11
#define I_LAST_MODIFIED_STAMP	15
#define I_ATTRIBUTE				19
#define I_HEADER_LEVEL			20
#define I_NAME_LENGTH			21
#define I_NAME					22

#define I_CRC					22 /* + name_length */
#define I_EXTEND_TYPE			24 /* + name_length */
#define I_MINOR_VERSION			25 /* + name_length */
#define I_UNIX_LAST_MODIFIED_STAMP	26 /* + name_length */
#define I_UNIX_MODE				30 /* + name_length */
#define I_UNIX_UID				32 /* + name_length */
#define I_UNIX_GID				34 /* + name_length */
#define I_UNIX_EXTEND_BOTTOM	36 /* + name_length */

#define I_GENERIC_HEADER_BOTTOM		I_EXTEND_TYPE



#define EXTEND_GENERIC		0
#define EXTEND_UNIX		'U'
#define EXTEND_MSDOS		'M'
#define EXTEND_MACOS		'm'
#define EXTEND_OS9		'9'
#define EXTEND_OS2		'2'
#define EXTEND_OS68K		'K'
#define EXTEND_OS386		'3'		/* OS-9000??? */
#define EXTEND_HUMAN		'H'
#define EXTEND_CPM		'C'
#define EXTEND_FLEX		'F'
#define EXTEND_RUNSER		'R'
/*	this OS type is not official */
#define EXTEND_TOWNSOS		'T'
#define EXTEND_XOSK		'X'
/*------------------------------*/

#define GENERIC_ATTRIBUTE			0x20
#define GENERIC_DIRECTORY_ATTRIBUTE		0x10
#define HEADER_LEVEL0				0x00
#define HEADER_LEVEL1				0x01
#define HEADER_LEVEL2				0x02

#define CURRENT_UNIX_MINOR_VERSION		0x00

#define DELIM ('/')
#define DELIM2 (0xff)
#define DELIMSTR "/"




#define OSK_RW_RW_RW			0000033
#define OSK_FILE_REGULAR		0000000
#define OSK_DIRECTORY_PERM		0000200
#define OSK_SHARED_PERM			0000100
#define OSK_OTHER_EXEC_PERM		0000040
#define OSK_OTHER_WRITE_PERM	0000020
#define OSK_OTHER_READ_PERM		0000010
#define OSK_OWNER_EXEC_PERM		0000004
#define OSK_OWNER_WRITE_PERM	0000002
#define OSK_OWNER_READ_PERM		0000001

#define UNIX_FILE_TYPEMASK		0170000
#define UNIX_FILE_REGULAR		0100000
#define UNIX_FILE_DIRECTORY		0040000
#define UNIX_SETUID				0004000
#define UNIX_SETGID				0002000
#define UNIX_STYCKYBIT			0001000
#define UNIX_OWNER_READ_PERM	0000400
#define UNIX_OWNER_WRITE_PERM	0000200
#define UNIX_OWNER_EXEC_PERM	0000100
#define UNIX_GROUP_READ_PERM	0000040
#define UNIX_GROUP_WRITE_PERM	0000020
#define UNIX_GROUP_EXEC_PERM	0000010
#define UNIX_OTHER_READ_PERM	0000004
#define UNIX_OTHER_WRITE_PERM	0000002
#define UNIX_OTHER_EXEC_PERM	0000001
#define UNIX_RW_RW_RW			0000666

#define LZHEADER_STRAGE			4096


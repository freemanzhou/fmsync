



#define		CURRENT_VERSION			576



// the top 4 bits of the AppInfo flags are reserved for indexes into the 16 categories 

// which are saved in the prefs structure, so 'new' databases should have the top 4 bits always

// 0 out

#define 	INFOFLAGS_LOCKPWONEXIT		0x0001

#define		INFOFLAGS_VIEWONLY			0x0002

#define		INFOFLAGS_CATPRIV_LOCKED	0x0004		// the category and private record flags are 'locked' by another app for this db

#define		INFOFLAGS_STRUCTLOCK		0x0008		// structure locked - takes precedence over

													// INFOFLAGS_VIEWONLY if set

#define		INFOFLAGS_PRIVATEDB			0x0010		// instead of password, use the normal Palm Privacy settings



// only first 8 bits should be used

#define		FLDTYPE_STRING			0x0001		// strlen + 1 (for null) bytes of data

#define		FLDTYPE_BOOLEAN			0x0002		// strlen + 1 (for null) bytes of data

#define		FLDTYPE_DATE			0x0004		// strlen + 1 (for null) bytes of data

#define		FLDTYPE_INT			0x0008		// strlen + 1 (for null) bytes of data

#define		FLDTYPE_FLOAT			0x0010		// strlen + 1 (for null) bytes of data

#define		FLDTYPE_TIME			0x0020		// strlen + 1 (for null) bytes of data

#define		FLDTYPE_LIST			0x0040		// strlen + 1 (for null) bytes of data

#define		FLDTYPE_AUTODATE		0x0041		// strlen + 1 (for null) bytes of data

#define		FLDTYPE_AUTOTIME		0x0042		// strlen + 1 (for null) bytes of data

#define		FLDTYPE_AUTOINC			0x0043		// strlen + 1 (for null) bytes of data

#define		FLDTYPE_BINARY			0x0044		// variable length - always a 'hidden' field type

#define		FLDTYPE_RELLIST			0x0045		// not yet implemented 

#define		FLDTYPE_CALC			0x0046		// not yet implemented

// these are the top 8 bits of the fld type , which are reserved for options currently

#define		FLDOPT_PROTECTED		0x8000		// shown but not edited	

#define		FLDOPT_HIDDEN			0x4000		// not visible to end user

#define		FLDOPT_CLEAR			0x00ff		// mask for easy removal of above settings



// App Prefs stored here

#define		PREFFLAGS_CAPITALIZE	0x0001

// CLICKSORT is no longer used

#define		PREFFLAGS_CLICKSORT		0x0002

#define		PREFFLAGS_EDITINPLACE	0x0004

#define		PREFFLAGS_NONFINDABLE	0x0008

#define		PREFFLAGS_VIEWONLYMODE	0x0010

#define		PREFFLAGS_VIEWFONT1		0x0020

#define		PREFFLAGS_VIEWFONT2		0x0040

#define		PREFFLAGS_VIEWFONT3		0x0080

#define		PREFFLAGS_DATAFONT1		0x0100

#define		PREFFLAGS_DATAFONT2		0x0200

#define		PREFFLAGS_DATAFONT3		0x0400



#define		MAX_EXTRACHARDATA_LENGTH			10

#define		MAX_TOTAL_POPUP_PER_LIST_LENGTH			2000

#define		MAX_POPUP_ITEMS					100

#define		MAX_POPUP_ITEM_LENGTH				25

#define 	MAX_DB_NAME_LENGTH 				30

#define 	MAX_FIELDS					50

#define 	MAX_FIELD_NAME_LENGTH				20

#define 	MAIN_TABLE_ROWS					10

#define		NEWDB_FIELD_ROWS				10

#define		MAX_DATA_LENGTH					500

#define		NORMAL_DATA_LENGTH				50

#define		MAX_TITLE_LENGTH				50

#define		SHOWITEM_ROWS					11

#define		SHOWITEM_TABLE_HEIGHT				122

#define		SHOWDB_TABLE_ROWS				11

#define		MAX_FIND_STRING					16

#define		MAX_CATEGORY_NAME_LENGTH			15

#define		MAX_CATEGORIES					16



typedef struct 

{

	char	fieldNames[MAX_FIELDS][MAX_FIELD_NAME_LENGTH+1];	// holds the field names

	int	fieldTypes[MAX_FIELDS];		

	int	numFields;

	int	version;			

	int	showDBColumnWidths[MAX_FIELDS];

	int 	showDataWidth;

	int	sort1Field;

	int	sort2Field;

	int	sort3Field;

	int	findField;

	int	filterField;

	char	findString[MAX_FIND_STRING];

	char	filterString[MAX_FIND_STRING];

	int	flags;

	int	firstColumnToShow;

	long	fieldExtraData[MAX_FIELDS];			// increment counters, calculated field info etc

	long	fieldExtraData2[MAX_FIELDS];			// increment counters, calculated field info etc

	char*	popupLists;				// any 'binary' data will follow this in the appinfo.

							// if there is a popup list it will end in 2 nulls

							// if there is no popuplist, there will be 4 nulls here



							// binary data can then follow this popup list data and will be of the following format

							// 2 bytes of length data for each length segment, followed by 4 bytes for the creator code of saving app, followed by the data 

							// continue this until a final byte sequence of "EndJFileData"

							// NOTE: That this final byte sequence of "EndJFileData" should make it possible to easily locate the end of the

							// of the AppInfo structure, while only have a VERY low percentage of finding 'EndJFileData' in a true binary data field



} JBaseAppInfoType;



/********************************************************************************************



The popupLists in the above structure is a actually just a chunk of memory with a

continuous sequence of null terminated strings that holds the contents of the

items in the popup lists for this database.  Lists are seperated with the string

"popopX", where X is a,b,c.... for field item 1,2,3... etc.  The list must be terminated

with two null characters in a row, and must be shorter than 4000 chars in length.  For fields over number 26 (ie. popupz

in the list, begin again at field 27 with upper case chars, such as popupA = field 27, popubB = field28 etc).





Example:



"popupb\0First 2\0Second 2\0popupd\0First 4\0Second4\0Third 4\0\0"



The above string would translate to the following popup lists in JFile:



For the second field (ie field 'b'):



First 2

Second 2



For the fourth field (ie field 'd'):



First 4

Second 4

Third 4



Note that the app info structure should not be saving a pointer to the field at all, 

it should save the string above as an example - this means that when creating the .pdb

file it is necessary to compute the size of the required appinfo block as the program

runs if you are handling conversion TO JFile .pdb format.  If no popup lists are to be 

defined, just save 4 '\0' characters into the space alloted for the popupLists.   Then do a

similar process for and binary data stored at end of AppInfo struct.



The records themselves are stored in the following format:



2 bytes per field in the record containing the length of the used field

followed by the field data itself for each field.



ie.  



2 bytes - 00 03 - length of 3 as seen below

2 bytes - 00 05 - length of 5 as seen below

2 bytes - 00 01 - length of 1 as seen below

field one data - 'Hi' (ending null makes length of 3 as above)

field two data - 'test' (ending null makes length of 4 as above).

field three data - "" (null field)



So the first 6 bytes of the record would be:



00 03 00 05 00 01



followed by the following in the record:



"Hi\0test\0\0" (if viewed as a string)



Creating a simple database in JFile, then doing a HotSync, and viewing the resulting .pdb file

should make the above much clearer.



The records themselves are also null terminated sequences of strings, one null terminated

string per field.  All field types are saved as their string representations - so an integer

field type is recorded as its ascii representation (Ie. 45 would be stored as the string "45").

The only field type of concern in this respect should be the Boolean field type which saves

its data as either the string "1" or the string "0".



*************************************************************************************************/





#define JBaseAppType		'JFil'    // type for application.  must be 4 chars, mixed case.

#define JBaseDBType		'JfDb'    // type for application database.  must be 4 chars, mixed case.






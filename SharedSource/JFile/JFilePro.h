//
// JFile header file with relavant .pdb/database structure
//

#define		CURRENT_VERSION			576				// used in the 'version' variable of the AppInfo structure below
													// to make sure this is a 'compatible' JFile Pro database




// the top 4 bits of the AppInfo flags are reserved for indexes into the 16 categories 
// which are saved in the prefs structure, so 'new' databases should have the top 4 bits always
// 0 out
#define 	INFOFLAGS_LOCKPWONEXIT		0x0001		// will close the database on application exit to help preserve
													// 		privacy settings
#define		INFOFLAGS_VIEWONLY			0x0002		// view only databases are not non-editable while this bit is set
#define		INFOFLAGS_CATPRIV_LOCKED	0x0004		// no longer in use
#define		INFOFLAGS_STRUCTLOCK		0x0008		// structure locked - takes precedence over INFOFLAGS_VIEWONLY if set
													//  	this bit will dis-allow any changes to the field ordering, naming, field types, etc
#define		INFOFLAGS_PRIVATEDB			0x0010		// instead of password, use the normal Palm Privacy settings
													// 		if set, the database will only show up in JFile if the Palm unit is showing 'private' information
													// 		as determined by the security applet of the Palm device.


// only first 8 bits should be used
const UInt16 FLDTYPE_STRING			=0x0001;
const UInt16 FLDTYPE_BOOLEAN			=0x0002;
const UInt16 FLDTYPE_DATE			=0x0004;
const UInt16 FLDTYPE_INT				=0x0008;
const UInt16 FLDTYPE_FLOAT			=0x0010;
const UInt16 FLDTYPE_TIME			=0x0020;
const UInt16 FLDTYPE_LIST			=0x0040;
const UInt16 FLDTYPE_AUTODATE		=0x0041;
const UInt16 FLDTYPE_AUTOTIME		=0x0042;
const UInt16 FLDTYPE_AUTOINC			=0x0043;
const UInt16 FLDTYPE_BINARY			=0x0044;
const UInt16 FLDTYPE_RELLIST			=0x0045;
const UInt16 FLDTYPE_CALC			=0x0046;
const UInt16 FLDTYPE_MODDATE			=0x0049;
const UInt16 FLDTYPE_MODTIME			=0x0050;
const UInt16 FLDTYPE_MULTLIST		=0x0051;
const UInt16 FLDTYPE_FMSYNC_CHECKBOX	=0x0052;

// these are the top 8 bits of the fld type , which are reserved for options currently
const UInt16 FLDOPT_PROTECTED		=0x8000;
const UInt16 FLDOPT_HIDDEN			=0x4000;
const UInt16 FLDOPT_CLEAR			=0x00ff;
 
 



#define 	MAX_FIELDS						50		// maximum of 50 fields per record in the database
#define 	MAX_FIELD_NAME_LENGTH			20 		// field names limited to 20 characters
#define		MAX_DATA_LENGTH					4000	// each field can have up to 4000 characters in it
													// although the record will be limited by a variable amount
													// depending on PalmOS version number.
#define		MAX_FIND_STRING					16		// find strings limited to 16 characters
#define		MAX_TOTAL_POPUP_PER_LIST_LENGTH	2500	// maximum of 2500 characters used in total per popup list
#define		MAX_POPUP_ITEMS					100		// maximum of 100 popup items per popup list
#define		MAX_POPUP_ITEM_LENGTH			25		// popup items limited to 25 character string as max
#define 	MAX_DB_NAME_LENGTH 				30
 



#define JBaseAppType		'JFil'    // type for application.  must be 4 chars, mixed case.
#define JBaseDBType			'JfDb'    // type for application database.  must be 4 chars, mixed case.
 
 

#pragma options align=mac68k


typedef struct 
{
	char	fieldNames[MAX_FIELDS][MAX_FIELD_NAME_LENGTH+1];	// holds the field names
	short	fieldTypes[MAX_FIELDS];		
	short	numFields;
	short	version;			
	short	showDBColumnWidths[MAX_FIELDS];
	short 	showDataWidth;
	short	sort1Field;
	short	sort2Field;
	short	sort3Field;
	short	findField;
	short	filterField;
	char	findString[MAX_FIND_STRING];
	char	filterString[MAX_FIND_STRING];
	short	flags;
	short	firstColumnToShow;
	long	fieldExtraData[MAX_FIELDS];				// increment counters, calculated field info etc
	long	fieldExtraData2[MAX_FIELDS];			// future reserved use - not needed in current version of JFile Pro
	//char*	popupLists;		// any 'binary' data will follow this in the appinfo.
							// if there is a popup list it will end in 2 nulls
							// if there is no popuplist, there will be 4 nulls here

							// binary data can then follow this popup list data and will be of the following format
							// 2 bytes of length data for each length segment, followed by 4 bytes for the creator code of saving app, followed by the data 
							// continue this until a final byte sequence of "EndJFileData"
							// NOTE: That this final byte sequence of "EndJFileData" should make it possible to easily locate the end of the
							// of the AppInfo structure, while only have a VERY low percentage of finding 'EndJFileData' in a true binary data field

} JFileProAppInfoType;

#pragma options align=reset

const char kJFileProBlankSignal = 7;

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


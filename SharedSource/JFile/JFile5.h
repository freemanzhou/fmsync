#pragma once



namespace JFile5 

{



/************************************************************************

* COPYRIGHT:   Copyright © 2001 Land-J Technologies

*

* FILE:        jfileinf.h

*

* SYSTEM:      JFile for PalmOS

* 

* DESCRIPTION: JFile database structure information file

*

* AUTHOR:      John J. Lehett , jlehett@land-j.com

*         

*************************************************************************/







/************************************************************************

*

*	FORMAT: This file is formatted with tab stops every 4 spaces, which

*			is receommended when viewing this file for proper spacing. 

*

*************************************************************************/







/************************************************************************

*

*	JFile Database Structure General Information

*

*	This file describes the information necessary to access/view/edit

*	JFile databases, either on the PalmOS unit itself, via a HotSync 

*	conduit, or via direct usage of the backed up .pdb file.  This file

*	ONLY describes the JFile specific portion of the format, and will

*	require the reader to know the overall .pdb format specification

*	from Palm Computing's .pdb file specifications to fully be able 

*	to make use of this information.

*	

*	This file assumes the reader has a working knowledge of the C

*	programming language.

*	

*	In general, a Palm .pdb file/database is composed of the 'generic'

*	header information that all .pdb files must posses, followed by the

*	records themselves.  JFile databases make use of two areas of the 

*	'generic' header area, and those are the 'version' value of the header

*	and also the AppInfo section.  Both of these are fully described

*	later in this document.  Similarly, the record format of each JFile

*	record is also described in a section later in this document.

*	

*	To summarize the above: for making use of JFile database, the reader

*	will need to be familiar with the overall Palm Computing .pdb

*	specification, and then the particular JFile specific areas listed

*	below.  In conjunction with each other, the reader can have full

*	access to a JFile database.

*	

*         

*************************************************************************/







/************************************************************************

*

*	JFile 4 Versus JFile 5 Internal Database Structure Differences:

*	

*	Creator ID, and Database Type Changed:

*		- JFile 4 versions use 'JFil', and 'JfDb' as the creator id/

*			database type.  JFile 5.x files use 'JFi5', and 'JfD5'

*			respectively.

*

*	AppInfo structure changes:

*		- 'version' variable renamed to 'marker' to lessen possible 

*			confusion with the new version usage of the pdb structure 

*			itself		

*		- 'flags' variable renamed to 'flags_defunct' and no longer in use

*		- 'popupLists' variable has been renamed to 'extraDataChunk'

*			to better reflect that other things besides popup lists

*			may now reside in this chunk of memory.

*		- 'EndJFileData' marker is no longer in use in the AppInfo section,

*			instead, explained further in the AppInfo comments below.

*

*	New Field Types and Options include:

*		- Calculated field

*		- Default values for field, saved in the extraDataChunk

*		- Values used for calculated fields saved in the extraDataChunk

*		- 5 'Advanced Filter' settings may be saved in the extraDataChunk

*		- 5 'Sort' settings may be saved in the extraDataChunk

*		- String fields now have an additional option for 'short text'

*	

*	Overall:

*		The primary structure of JFile 4.x and 5.x formatted databases

*		are the same.  When JFile 5.x is first run, all databases on the

*		Palm unit are checked to see if they are in JFile 5.x format.

*		This check is performed by checking the creator/db types

*		of the database.  If the old creator id/db types are found for 

*		a database, the database is a JFile 4.x formatted database, and

*		the user will be asked if they would like to have their JFile 4.x

*		databases automatically be converted to 5.x format which consists

*		of simply moving the values of the preveious 'flags' variable

*		of the AppInfo structure into the new 'version' area, and changing

*		the creator id/db type.  The record format has remained forward 

*		compatible, as have all other parts of the AppInfo area. 

*		

*		The only differences therefore, between JFile 4.x and 5.x format

*		databases, are the new creator/db types, the usage of the new 

*		'Version' 16 bit value, and the additional usages of the same 

*		AppInfo variables, such as how Calculated field types are stored, 

*		and how default values and Advanced Filter settings are stored 

*		in the AppInfo area.

*		

*		Therefore, except for the alteration of the 'version' information 

*		above and changing of the creator/db types, all JFile 4.x 

*		formatted database are more easily 'forward compatible'.

*

*		Summarization of converting from JFile 4.x .pdb to JFile 5.x pdb:

*			1) check for JFile 4.x creator id/db types

*			2) If found, change creator/ id/db type to new values

*			3) copy the defunct 'appInfo->flags' bits to the appropriate

*				'version' variable

*				

*		CAVEAT: JFile 5.x will not automatically convert a JFile 4.x .pdb

*			file to 5.x format if the INFOFLAGS_STRUCTLOCK_DEFUNCT bit is

*			set.  The reason for this is that, to date, the only usage

*			of this bit that I am aware of was for FMSync to lock in 

*			certain structural details of the database, and FMSync has

*			requested that JFile 5.x not auto-convert its files.  If you

*			do not plan to interact with FMSync created .pdb files, you 

*			should be able to safely disregard this caveat.

*    

*************************************************************************/









/************************************************************************

*

*	The following are some MAXIMUM values for JFile databases

*         

*************************************************************************/



// Maximum of 50 fields per data

#define 	MAX_FIELDS						50		



// Maximum of 20 characters per field name

#define 	MAX_FIELD_NAME_LENGTH			20 		



// Maxiimum of 4000 characters per record

#define		MAX_DATA_LENGTH					4000	



// Maximum length of strings for the 'find' or 'filter' operation is 

// 15 character

#define		MAX_FIND_STRING					16



// Maximum of 2500 characters per popup list

#define		MAX_TOTAL_POPUP_PER_LIST_LENGTH	2500	



// Maximum of 100 items in each popup list

#define		MAX_POPUP_ITEMS					100		



// Maximum of 25 characters per item in a popup list

#define		MAX_POPUP_ITEM_LENGTH			25		



// Database names for JFile limited to 30 characters

#define 	MAX_DB_NAME_LENGTH 				30

 

// Default values for fields (new to JFile 5.x) are limited to 80 chars

#define 	MAX_DEFAULTVAL_LENGTH			80		



// Maximum 'filter name' is limited to 16 characters

#define		MAX_FILTERNAME_LENGTH			16

#define		MAX_SORTNAME_LENGTH				16



// The Short Text string sub type is limited to this many characters

#define		MAX_SHORT_TEXT_CHARS			250









/************************************************************************

*

*	JFile AppInfo Section Definition:

*

*	The following is the primary structure in JFile databases that hold

* 	all of the structural information about the JFile database itself.

* 

* 	This is stored in the AppInfo area of a Palm database/.pdb file

*

* 	Each of the variables in the structure is explained below:

*

*	fieldNames[MAX_FIELDS][MAX_FIELD_NAME_LENGTH+1]:

*		This variable contains normal character strings (null terminated)

*		that represent the names of each field in the JFile database

*		

*	fieldTypes[MAX_FIELDS]:

*		This variable contains the field types for each field of the 

*		JFile database.  The values of this field are explained in the

*		section later in this document regarding the FLDTYPES_... defines.

*		

*	numFields:

*		An integer value containing the number of fields in the database

*		

*	marker:

*		This is a simple token that MUST be set to the value of the 

*		CURRENT_MARKER define.  This is checked in some cases in JFile

*		to insure that the AppInfo structure is a JFile database.

*		

*	showDBColumnWidths[MAX_FIELDS]:

*		This structure holds the pixel widths for each column when

*		displaying the database in the full database view in JFile. 

*		If the value for a field is 0, the column is 'hidden' in the JFile

*		view, and is not displayed to the end user.  

*		If the value is negative, its absolute value is the pixel width,

*		and the column is 'locked' to the screen negative values are new

*		in JFile 5).

*		

*	showDataWidth:  

*		This variable determines the pixel width of the data fields when

*		viewing a single record in JFile.  

*		

*	sort1Field:

*		this variable contains which field is the primary sort field from 

*		the last sort operation performed by the end user.  A negative 

*		value indicates a reverse sort.

*		

*	sort2Field:

*		same as above, but for the secondary sort field

*		

*	sort3Field:

*		same as above, but for the tertiary sort field

*		

*	findField:

*		this variable contains which field is the 'find' field from the 

*		last 'Find' operation performed by the end user.

*	

*	filterField:

*		this variable contains which field is the 'filter' field from the

*		last 'Filter' operation performed by the end user.

*		

*	findString[MAX_FIND_STRING]:

*		this fields contains the character string of the last 'Find' 

*		operation performed by the end user

*		

*	filterString[MAX_FILTER_STRING]:

*		this field contains the character string of the last 'Filter'

*		operation performed by the end user

*		

*	flags_defunct:

*		This field is no longer used in JFile 5.  However, older 

*		JFile 4.x databases used this to store the values described in

*		the INFOFLAGS_... defines below.  JFile 5, when encoutering a 

*		JFile 4 database will automatically transfer these values into 

*		the new location described in the VERSION_ defines below.  See 

*		the section of the document that describes the specific changes 

*		between JFile 4 and JFile 5 formatted databases for a full 

*		description.

*		

*	firstColumnToShow:

*		This contains the number of the field that is the first 

*		'move-able' column to be shown when viewing the full record 

*		list of a database.  The left most columns of this screen will

*		always be the locked fields/columns (if any), followed by the 

*		non-locked column, beginning with this variables value.

*		

*	fieldExtraData[MAX_FIELDS]:

*		Location to store extra data regarding the field, used for 

*		certain field types.  See the full explanation of this variable

*		which is given in the section on Field Types in this document.

*		

*	fieldExtraData2[MAX_FIELDS]:

*		As above, a second variable to store extra data regarding the 

*		field.

*		

*	extraDataChunk:

*		This is a pointer to a chunk of memory of variable size.  

*		Currently, the popup lists, saved sorts/advanced filters, and default 

*		or calc values for fields are stored in this area.  The structure of 

*		this memory chunk is as follows:

*

*		Conceptually:

*			the arrangement in generic form is this:

*			

*			if lists do not exist:

*			----------------------

*			4 null bytes

*			

*			if lists do exist:

*			------------------

*			list seperator string 1

*			list item 1

*			list item 2

*			... list item n

*			list seperator string 2

*			list item 1

*			list item 2

*			... list item n

*			.

*			.

*			.

*			list seperator string n

*			list item 1

*			list item 2

*			... list item n

*			terminating null character

*

*		

*		1) If there are no popup lists, sorts/advanced filters or default/calc 

*			values, the memory chunk will consist of 4 null bytes.

*			

*		2) If there are popup lists, sorts/advanced filters, or default/calc values, 

*			they are structured in the data chunk by sequences of null 

*			terminated strings, beginning with the

*			seperator string (which is defined later in this 

*			document), and then after all such lists are

*			complete, a final additional terminating null is added.  

*

*			As a result of the above, there should be no empty or null 

*			strings in the entire extraChunkData list format.  The 

*			reason is that the list sequence is always ended ended with

*			2 null bytes (the final null of the last string, followed by

*			a terminating null). 

*			

*			Seperator strings are comprised of the seperator type, 

*			which is either "popup", "flllt" or "deflt", followed by a 

*			character representing which field the seperator is 

*			referencing, in the following manner:

*			

*			a = field 1

*			b = field 2

*			...

*			z = field 26

*			A = field 27

*			B = field 28

*			etc...

*			

*			So a popup list seperator for a popup list for field number

*			2 would be "popupb", and the default value seperator for 

*			field number 3 would be "defltc", and the advanced filter

*			seperator for the 2nd advanced filter would be "fllltb".

*			

*			Following the seperator are the null terminated strings,

*			which are defined as follows for each type of list:

*			

*			popup list:

*			 	a sequence of null terminated strings, one string

*			 	for each item in the popup list

*			 	

*			default value:

*				a single null terminated string giving the default

*				value for that field

*

*			calc value:

*				a single null terminated string giving the value

*				to be used in the calc field type

*

*			saved sort list:

*				New to JFile 5.x is the ability to save up to 5 

*				sort settings, for easy recall by the end

*				user - refer to the actual Sort screen

*				in JFile 5.x to see this in action.

*				

*				Each sort can be named.

*				

*				Saving such sorts to a list takes this form:

*	

*				1st item: filter name

*				2nd item: an integer representing which field is the

*					primary sort field (0 = no field set, negative

*					values = reverse sort of the field).

*				3rd item: as above, for the secondary sort field

*				4th item: as above, for the tertiary sort field

*

*				

*				

*			advanced filter list:

*				New to JFile 5.x is the ability to save up to 5 

*				advance filter settings, for easy recall by the end

*				user - refer to the actual Advanced Filter screen

*				in JFile 5.x to see this in action.

*				

*				Each advanced filter can be named, and each can

*				have up to 5 distince filter strings.

*				

*				Saving such advanced filters to a list takes this form:

*	

*				1st item: filter name

*				2nd through 6th item: the filter strings themselves

*					NOTE: if a filter string is not present, then the 

*					string "-*-" is used instead.

*				7th item: is a bit mapped integer which uses the 15

*					defines below such as FILTER_AND1_BIT, 

*					FILTER_BEGIN3_BIT, etc.  These directly correspond

*					to the settings present in the Advanced Filter

*					Screen in JFile itself.

*				8th through 12th items: an integer representing which

*					field this filter string refers to.  'All Fields'

*					value is represented as a -1 value.

*				

*			

*			The above list usage can be seen more clearly 

*			by some examples:

*			

*			Example:

*

*	"popupb\0First 2\0Second 2\0popupd\0First 4\0Second4\0Third 4\0\0"

*			

*			The above string sets a popup list for field 2 (popupb) 

*			that consists of two items, "First 2", and "Second 2", 

*			and also popup list for field 4 (popupd), consisting of 3

*			items, "First 4", "Second 4", and "Third 4".  Notice the

*			sequence ends with an extra terminating null byte.

*			

*			Example:

*			

*			"popupc\0First\0Second\0defltd\0Hi there\0\0"

*			

*			This example shows a popup list for field 3 (popupc), 

*			consisting of 2 items, "First", and "Second", as well as a

*			default value for field 4 (defltd) that will place 

*			"Hi there" as a default value for field 4 when new records 

*			are created in JFile.

*

*			Example:

*			

*			"flllta\0My Filter\0John\05\0-*-\0-*-\0-*-\031\0-1\01\0-1\0-1\0-1\0"

*			

*			This example shows an Advanced Filter save list, for the first

*			(of 5 possible) advanced filters.  The filter name is 

*			"My Filter".  The first filter string is "John", the second is

*			the string "5", and the other 3 are the 'empty' value placeholder.

*			The 31 value indicates that each of the filter string results are

*			AND'd together (taken from the values in FILTER_AND... defines

*			listed below).  Finally, the first string represents a filter

*			on all fields (the "-1" string), the second represents the field

*			at position 1 (this is a 0-based number), so it is actually the

*			second field, and then the remaining 3 "-1" values represent 

*			All Fields again.

*			

*			The Advanced Filter list may sound like a very complex list of

*			values but it is not.  The easiest way to see this in action is

*			to create some JFile databases, and use the Advanced Filter 

*			itself to create some saved filters.  Then inspect the .pdb file

*			to see exactly what the extraChunkData looks like for specific

*			cases.  

*			

*			Example:

*			

*			"soorta\0My Sort\03\0-2\00"

*			

*			This example is for the first saved sort, named "My Sort", with 

*			the sort parameters that the primary sort field is a normal

*			sort on the 3rd field, the secondary sort item is a reverse sort

*			of the 2nd field, and the tertiary sort field is not used.

*		

*			

*		So putting all of the above together, an example of a complete 

*		memory chunk might be:

*		

*			"popupc\0First\0Second\0defltd\0Hi there\0\0EndJFileData"

*		

*		NOTE: Because of the above structure the end of the extraChunkData

*		section can always be found by searching for two null bytes

*		adjacent to one another.

*

*		NOTE: There is no JFile imposed 'ordering' of these lists in the

*		extraDataChunk section.

*		

*		NOTE: The PC side converter exports nearly all of the values

*		in this AppInfo structure, which are then saved in an .ifo

*		file as a companion file to the comma seperated value file.  We

*		do make the PC side converter source code available to those

*		interested, so feel free to request the source code to the 

*		converter if it might be of assistance in interpreting how 

*		all of the above ties together.  The converter itself may also

*		be a useful developer tool to analyze the contents of the 

*		AppInfo structure in a more textual manner by viewing the 

*		created .ifo files.

*

*		NOTE: Previous versions of JFile developer documentation 

*		mentioned that binary data could also be placed in this section

*		of the AppInfo structure.  To date this has not been used in 

*		any way, and to simplify the structure, binary data is no 

*		longer supported in this area.  As a result the "EndJFileData"

* 		marker is no longer in use, and any bytes in the AppInfo

*		section, following the double-null ending of the extraDataChunk

*		can be safely ignored

*		

*         

**************************************************************************/



#pragma options align=mac68k



typedef struct 

{

	char	fieldNames[MAX_FIELDS][MAX_FIELD_NAME_LENGTH+1];	

	short	fieldTypes[MAX_FIELDS];		

	short	numFields;

	short	marker;			

	short	showDBColumnWidths[MAX_FIELDS];

	short 	showDataWidth;

	short	sort1Field;

	short	sort2Field;

	short	sort3Field;

	short	findField; 

	short	filterField;

	char	findString[MAX_FIND_STRING];

	char	filterString[MAX_FIND_STRING];

	short	flags_defunct;									

	short	firstColumnToShow;

	long	fieldExtraData[MAX_FIELDS];				

	long	fieldExtraData2[MAX_FIELDS];	

	

	// 

	// above this break is all 'fixed length' amount of memory usage

	// below this break is the remainder of the AppInfo section, and is 

	// represented simply as a char* pointer to a chunk of memory of

	// a variable number of bytes

	//		

													

	//char*	extraDataChunk;		

} JFileAppInfoType;



#pragma options align=reset



const char kJFileProBlankSignal = 7;



/************************************************************************

*

*	As mentioned in the AppInfo discussion above, these are the defines

*	for the the Advanced Filter settings that are saved.

*         

*************************************************************************/



// No 0-length strings are permitted in the extraChunkData, so 

// instead use this define for the Advance Filter string instead

// of 0-length strings.

#define	FILTER_STRING_UNUSED_MARKER "-*-"



#define		FILTER_AND1_BIT		0x0001

#define		FILTER_AND2_BIT		0x0002

#define		FILTER_AND3_BIT		0x0004

#define		FILTER_AND4_BIT		0x0008

#define		FILTER_AND5_BIT		0x0010

#define		FILTER_BEGIN1_BIT	0x0020

#define		FILTER_BEGIN2_BIT	0x0040

#define		FILTER_BEGIN3_BIT	0x0080

#define		FILTER_BEGIN4_BIT	0x0100

#define		FILTER_BEGIN5_BIT	0x0200

#define		FILTER_NOT1_BIT		0x0400

#define		FILTER_NOT2_BIT		0x0800

#define		FILTER_NOT3_BIT		0x1000

#define		FILTER_NOT4_BIT		0x2000

#define		FILTER_NOT5_BIT		0x4000







/************************************************************************

*

*	As mentioned in the AppInfo discussion above, these are the defines

*	for the seperator type strings that can be found in the 

*	extraDataChunk section

*         

*************************************************************************/



#define	DEFAULT_SEPERATOR_STRING			"deflt" 

#define POPUP_SEPERATOR_STRING				"popup"

#define FILTER_SEPERATOR_STRING				"flllt"

#define	SORT_SEPERATOR_STRING				"soort"

#define	CALC_VALUE1_SEPERATOR_STRING		"calv1"

#define	CALC_VALUE2_SEPERATOR_STRING		"calv2"





/************************************************************************

*

*	This define is the value that the 'version' variable of the AppInfo

* 	structure MUST contain.  It is used primarily just to facilitate

* 	validation of JFile database.

*   NOTE: This value has remained the same for JFile 4 and JFile 5

*         

*************************************************************************/



#define		CURRENT_MARKER			576		





const char kJFileBlankSignal = 7;



/************************************************************************

*

*	Defines For The 'Version' Of The PDB File:

*

*	These defines are bit flags that determine the value of the 'version' 

*	of each database.  This is new in JFile 5.x.  Instead of storing 

*	these values in the version flag of the AppInfo structure, these

*	are now stored in the version variable of the entire database itself.

*	

*	To clarify this further:

*	

*	JFile 4.x:  stored these values in the AppInfo section's 'flags' 

*		variable.  That usage is now defunct in version JFile 5.x

*		

*	JFile 5.x:  stores these values in the 'version' variable of the

*		.pdb file itself.  This can be seen from the following 

*		PalmOS call:

*		DmDatabaseInfo(0, dbID, NULL, NULL, &version, NULL, NULL, NULL,

*						NULL, NULL, NULL, NULL, NULL);

*		It is this new 'version' variable that will store such values

*		as those listed in the DEFINE's below.  

*		

*	This change was made so that certain aspects of the database

*	could be determined without actually loading in the database

*	and interpreting the AppInfo section.  Instead, JFile 5.x now

*	only needs to use the above PalmOS call to grab the 'version' 

*	variable, and can easily deterine these aspects from that alone.

*

*	Category indexes: the category index for each database is stored

*	in the first 4 bits of this version flag (notice that in the 

*	DEFINE's below, only the bottom 12 bits are used).  JFile itself

*	maintains an array of up to 15 categories that the user can

*	modify the names of.  Then these top 4 bits determine which 

*	category of the array the database belongs to.

*	

*	What this means is that .pdb creators cannot set the category 

*	name of the database itself.  You can only set the index of the

*	category.  The result of this is that .pdb creators/modifiers

*	should only maintain the value of the category index for the 

*	end user, and not modify it.  This will allow the .pdb file to

*	remain in the category the user has already chosen for it.		

*         

*	NORMAL: 

*		the database has no security 

*	

*	ENCRYPTED:

*		the database is encrypted (see the encryption section of

*		this document for a full explanation of encryption)

*		

*	PASSWORDED:

*		this database requires the PalmOS security app's password

*		to be entered to access the database

*

*	AUTOSORT:

*		JFile will attempt to keep the records sorting according

*		to the last sort method used by the end user (ie. the

*		one currently stored in the 'sort' related variables

*		of the AppInfo section above.

*

*	ISFILTERING:

*		a flag that is set if the database is currently showing

*		only those records in the current filter

*		

*	STRUCTLOCK:

*		if set, field names cannot be re-ordered, or the database

*		name cannot be changed.

*		

*	FULLSTRUCTLOCK:

*		if set, no modifications to the database structure can be

*		made at all, ie. no field renaming, no field type changes,

*		re-ordering, etc.

*		

*	READONLY:

*		if set, no changes to the database are permitted

*		

*	50_FORMATTED:

*		this is a flag to let the app know that this database is

*		indeed in JFile 5.x format.  Since the JFile 4.x format

*		and 5.x format are nearly identical, the easiest way to 

*		tell if a database is in JFile 5.x format is to test this

*		bit.

*

*************************************************************************/



#define		DB_VERSION_NORMAL			0x0001		

#define		DB_VERSION_ENCRYPTED		0x0002

#define		DB_VERSION_PASSWORDED		0x0004

#define		DB_VERSION_AUTOSORT			0x0008 		

#define		DB_VERSION_ISFILTERING		0x0080

#define		DB_VERSION_STRUCTLOCK		0x0100		

#define		DB_VERSION_FULLSTRUCTLOCK	0x0200		

#define		DB_VERSION_READONLY			0x0400

#define		DB_VERSION_50_FORMATTED		0x0800		









/************************************************************************

*

*	Defunct 'flag' Variable Values:

*

*	These defines should no longer be used.  They were used in JFile 4

*   in place of the 'DB_VERSION...' defines described above.  See the

* 	section on JFile 4 vs JFile 5 differences for full details on this. 

*         

*************************************************************************/



#define 	INFOFLAGS_LOCKPWONEXIT_DEFUNCT		0x0001		

#define		INFOFLAGS_VIEWONLY_DEFUNCT			0x0002		

#define		INFOFLAGS_STRUCTLOCK_DEFUNCT		0x0008	

#define		INFOFLAGS_PRIVATEDB_DEFUNCT			0x0010																	

#define		INFOFLAGS_ISFILTERING_DEFUNCT		0x0020																	

#define 	INFOFLAGS_FULLSTRUCTLOCK_DEFUNCT	0x0040		



#define 	INFOFLAGS_SYNC_ALTERED_DB_DEFUNCT	0x0080	



/************************************************************************

*

*	Field Related Defines:

*

*	These defines are used to determine values for the fieldTypes 

* 	variable of the AppInfo structure.  The fieldTypes are 16 bit 

*	values that are used as follows:

*

*		left 8 bits = reserved for options that can be set on a per

*			field basis

*			

*		right 8 bits = determine the exact type of field

*		

*	Currently, there are only two 'options' that can be set per field,

*	and those are the FLDOPT_PROTECTED, and FLDOPT_HIDDEN options.  

*	You can 'or' these bit values to the full 16 bit field type to

*	make any field type a protected (ie. read-only) field or to make it 

*	a hidden field that the user would not see in normal usage.

*	

*	The field types list is as follow:

*	

*	FLDTYPE_STRING:

*		normal null terminated string of characters field type

*		

*		New to JFile 5.x is the 'Always less than 250 characters'

*		options for string fields.  If checked, the fieldExtraData

*		variable value for this field will be integer 1, if unchecked,

*		the fieldExtraData variable for this value will be 0.  JFile 

*		will not allow more than 250 characters to be typed into that

*		field.  The default value for string fields is the normal 4000 

*		character per string field limit.  However, this option was 

*		implemented to make it easier for developers who are working on 

*		PC/Mac side applications to specify a field type in a database

*		derived application that will accomodate only up to 250 

*		characters.

*

*		Summary: fieldExtraData[fieldNumber] = 0 -> normal 4000 char limit

*				 fieldExtraData[fieldNumber] = 1 -> 250 character limit

*				 

*		

*	FLDTYPE_BOOLEAN:

*		shown as a checkbox in JFile, this value is stored internally

*		as either a "0" or "1" string.

*	

*	FLDTYPE_DATE:

*		null terminated string of characters that should be in 'date'

*		format.

*		

*	FLDTYPE_INT:

*		null terminated string of characters that should be in 'integer'

*		format

*		

*	FLDTYPE_FLOAT:

*		null terminated string of characters that should be in 'floating 

*		point' format

*		

*	FLDTYPE_TIME:

*		null terminated string of characters that should be in 'time'

*		format

*		

*	FLDTYPE_LIST:

*		null terminated string of characters that allows the user to

*		select a value from a popup list

*		

*	FLDTYPE_AUTODATE:

*		same as date type above, but new records have this field filled

*		in automatically with the current date

*		

*	FLDTYPE_AUTOTIME:

*		same as time type above, but new records have this field filled

*		in automatically with the current time

*		

*	FLDTYPE_AUTOINC:

*		an auto increment field, the field is filled in with the integer

*		value represented in fieldExtraData[fieldNumber].  The value

*		in fieldExtraData[...], is then incremented by the integer 

*		value stored in fieldExtraData2[...].

*		

*		Example:  an autoincrement field, for field number 4 (0 based

*		since this is a C source code example), that 

*		should start at value 0, and increment by 3 for each new field 

*		would have the following settings:

*		

*		fieldExtraData[4] = 0;

*		fieldExtraData2[4] = 3;

*		

*	FLDTYPE_BINARY:

*		always a 'hidden' field type, developer could store any type of

*		binary data desired in this field

*		

*	FLDTYPE_CALC:

*		a calculated field, that is determined by the contents of the 

*		fieldExtraData value for this specific field.  The

*		fieldExtraData[fieldNumber] variable is a 32 bit value.  For the

*		purposes of calc fields, it is used this way:

*		

*		- bits 1 through 8 (left most) - the number of the first field

*				for the operation

*		- bits 9 through 16 - the number of the second field for the 

*				operation

*		- bits 17 through 32 - the operation itself to perform as defined

*				by the CALC_... defines listed in the next section of this

*				header file

*		

*		Thus this hexidecimal 32 bit value - 0x01040003 means:

*		

*		01 = field 1

*		04 = field 4

*		0003 = CALC_MULTIPLY

*		

*		So the field is: (field 1) x (field 4)

*		

*		The results of such calculations can either be calculated to a

*		variable number of decimal places, according to the value in the

*		fieldExtraData2[fieldNum] variable, with this table:

*		

*		fieldExtraData2[fieldNum] value:

*		0 = variable number of decimal places according to the value

*		1 = always show 1 and only 1 decimal place

*		2 = always show 2 and only 2 decimal places

*		3 = always show 3 and only 3 decimal places

*

*	FLDTYPE_CALC_V1:

*		same as FLDTYPE_CALC, but instead of <field> <op> <field>, it is

*		<value> <op> <field>, and the value to be used is stored in the 

*		extraDataChunk area with a seperator string of

*		CALC_VALUE1_SEPERATOR_STRING

*		

*	FLDTYPE_CALC_V2:

*		same as FLDTYPE_CALC, but instead of <field> <op> <field>, it is

*		<field> <op> <value>, and the value to be used is stored in the 

*		extraDataChunk area with a seperator string of

*		CALC_VALUE2_SEPERATOR_STRING

*

*	FLDTYPE_MODDATE:

*		same as date field, but this field is updated to the current date

*		whenever the record is modified

*		

*	FLDTYPE_MODTIME:

*		same as time field, but this field is updated to the current time

*		whenever the record is modified

*		

*	FLDTYPE_MULTLIST:

*		similar to a list field as listed above, however, with this type

*		of list, when the user selects a popup item, the item is appended

*		to the current field's data (rather than replacing it has occurs

*		with a normal list).  

*		

*		also, the seperating character used to seperate the appended

*		values is determined by the value of the 'fieldExtraData' variable

*		for this field according to this table:

*		

*		fieldExtraData[field] value table:

*		0 = space character is seperator

*		1 = comma character is seperator

*		2 = new line character is seperator

*		

*	FLDTYPE_FMSYNC_CHECKBOX:

*		this is a special checkbox that is used currently only for FMSync

*			        

*	Note On Field Type Validation:

*		JFile field data is only loosely type checked, meaning that any

*		field's value do not HAVE to conform to the type that the user

*		has set for the field.  JFile does attempt to prevent the user

*		from entering non-conforming data to a field, but there are 

*		numerous ways around this (such as importing from .csv format 

*		files), however, the developer who makes use of JFile records

*		cannot assume that the field will always match the field type

*		itself, and therefore must make allowances for the fact that

*		an integer field, for instance, could potentially have a 

*		letter in it.

*		

*		Example: a user can conceivable set a DATE field, for

*		instance, to be the string "My Birthday".  JFile only uses the

*		field types as an aid in such items as searches, sorts, and filters.

*		If a value in a field is not consistent with its type, the value

*		is considered null for the purposes of sorts/filters etc.

*		

*		

**************************************************************************/



const UInt16 FLDTYPE_STRING = 0x0001;

const UInt16 FLDTYPE_BOOLEAN = 0x0002;	

const UInt16 FLDTYPE_DATE = 0x0004;

const UInt16 FLDTYPE_INT = 0x0008;	

const UInt16 FLDTYPE_FLOAT= 0x0010;	

const UInt16 FLDTYPE_TIME= 0x0020;	

const UInt16 FLDTYPE_LIST= 0x0040;		

const UInt16 FLDTYPE_AUTODATE = 0x0041;

const UInt16 FLDTYPE_AUTOTIME = 0x0042;		

const UInt16 FLDTYPE_AUTOINC= 0x0043;	

const UInt16 FLDTYPE_BINARY= 0x0044;

#ifdef JFILE_CALCULATIONS

const UInt16 FLDTYPE_CALC= 0x0046;

const UInt16 FLDTYPE_CALC_V1= 0x0047;

const UInt16 FLDTYPE_CALC_V2= 0x0048;	

#endif

const UInt16 FLDTYPE_MODDATE= 0x0049;	

const UInt16 FLDTYPE_MODTIME= 0x0050;	

const UInt16 FLDTYPE_MULTLIST= 0x0051;		

const UInt16 FLDTYPE_FMSYNC_CHECKBOX= 0x0052;



const UInt16 FLDOPT_PROTECTED = 0x8000;

const UInt16 FLDOPT_HIDDEN = 0x4000;

const UInt16 FLDOPT_CLEAR =0x00ff;



enum {CALC_ADD = 0x0001, CALC_SUBTRACT, CALC_MULTIPLY, CALC_DIVIDE} CalculationType;



	







/************************************************************************

*

*	The following are the defines for the creator id and db type ids

*	that are required by all .pdb file

*         

*************************************************************************/



#define OldJFileAppType		'JFil'   // these are the types for JFile 4.x

#define OldJFileDBType		'JfDb'   // included for historical documentation

 

#define JFileAppType		'JFi5'   // the new types for JFile 5.x db's  

#define JFileDBType			'JfD5'    









/************************************************************************

*

*	Record Format for JFile:

*

*	The record format of individual records of JFile is as follows:

*	

*	The record begins with 2 bytes for each field, that contain the 

*	length of data in in each field.

*	

*	Ie.

*	

*	2 bytes - 00 03 - length of 3 for the first field

*	2 bytes - 00 05 - length of 5 for the second field

*	2 bytes - 00 01 - length of 1 for the third field

*	

*	field 1 data - "Hi" (ending null makes for a length of 3)

*	field 2 data - "test"

*	field 3 data - "" (null string field)

*	

*	So the first 6 bytes of the record would be:

*	

*	00 03 00 05 00 01

*

*	followed by the field's data itself:

*	

*	"Hi\0test\0\0"  (if viewed as a string)

*	

*	Creating a simple database in JFile, then doing a HotSync, and 

*	viewing the resulting .pdb file should make the above much clearer.

*

*************************************************************************/









/************************************************************************

*

*	JFile Encryption Information:

*

*	Blowfish Encryption information can be found at:

*		http://www.counterpane.com/blowfish.html

*		which also has a good supply of source code implementations

*		for various platforms.

*

*	Blowfish encryption was chosen for a number of reasons for JFile,

*	including: public availability of usage, no export restrictions, 

*	speed of the algorithms on the limited PalmOS cpu, and

*	an open format in that users and developers can specifically know

*	the algorithms used, and judge for themselves whether it is of high

*	enough security for usage.

*	

*	The encryption process of JFile uses an MD5 hash of the user's 

*	entered 'password' 

*	for the database to be the encryption key itself for the database.

*	The MD5 hash info can be found at:

*	

*	http://userpages.umbc.edu/~mabzug1/cs/md5/md5.html

*	

*	and results in a hashed sequence of 128 bits for any entered password.

*	Only the first 64 bits of this key sequence are used for the 

*	Blowfish key.

*	

*	The key length of 64 bits for Blowfish was chosen to allow JFile to

*	fall under an NLR (No License Required) status on U.S. exporting of

*	encryption software.  The government still requires a notification

*	for such NLR status applications, however, it is a much easier

*	process than other alternatives.  If you wish to include encryption/

*	decryption in your application, be sure to check the export 

*	restrictions in your country. (For U.S. based developers, the

*	'Notification' to the government consists of answering about 15

*	questions regarding the application (none of which are 'tough' to

*	answer) and emailing this, as well as sending a hardcopy, to the 

*	proper authorities.  Feel free to email me if you have questions on 

*	this.

*

*	Because of this, the password/key is not stored anywhere in the 

*	database what-so-ever.  To 'test' a password to determine validity,

*	a partion of the AppInfo section is decrypted to determine if the 

*	'marker' variable has a proper decrypted value, and if so, there is 

*	a very high probability that the encryption password is correct.

*	

*	Blowfish is a 64-bit block encryption method, therefore, encrypted

*	records and the AppInfo memory block of JFile databases may be 

*	lengthened by 1 to 7 bytes each to accomodate the encryption.

*	

*	JFile will require a 6 character long password at minimum, and

*	will recommend mixed case ascii values along with numbers to

*	help with password security.  The end result of this is that

*	a 6 byte key, consisting of upper/lower case values, plus numbers

*	has a total of 62 to the power of 6 possible combinations.  This

*	is somewhere in the range of a 50,000,000,000 combinations, which

*	may be 'crackable' but will go up exponentially if the password

*	is longer.  It will be recommended in the final version for users

*	to use a 8 or more character mixed case/digits password.  The 

*	64-bit key length for Blowfish is approximately equivalent to a

*	12 character password consisting of a random mix of upper/lower

*	case letters and numerals.

*	

*	Due to the nature of the PalmOS I do not at this time see a 

*	truely better alternative to the above method of implementation.

*	For instance, the Certicom 'secure memo' application, while it

*	may use 100+ bit keys for its encryption, is still password

*	protected to the end user, and the encryption key, as far as I

*	can tell, is only as strong as the password protecting it.  I will

*	gladly hear comments on this though if you have any.

*	

*	So, from the above, for developers to encrypt or decrypt a JFile 

*	database, the method is as follows:

*	

*	- Retrieve the users password from the user, either via dialog box

*		or command line option or any other means available for a 

*		particular implementation.

*		

*	- Using the password, decrypt the AppInfo block of the database

*		using normal Blowfish encryption algorithms, and test the 

*		AppInfo's 'marker' variable to make sure it is the proper value.

*		

*	- If it is, then full decryption can take place.  The AppInfo section

*		is encrypted as one complete memory chunk.  Similarly each 

*		record in JFile is encrypted as a seperate chunk.

*		

*	- Encryption is handled similarly - the AppInfo chunk is extended

*		to a 64-bit boundry if necessary and encrypted, and then

*		each record in the database is encrypted individually.

*         

*************************************************************************/



}
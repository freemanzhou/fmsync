#pragma once

#define		CURRENT_VERSION			452
#define		MAX_FIND_STRING			16
#define		MAX_PASSWORD_LENGTH		10
#define 	MAX_FIELDS			20
#define 	MAX_FIELD_NAME_LENGTH		20     
#define		MAX_DATA_LENGTH			500    
#define		MAX_DATA_LENGTH_2			MAX_DATA_LENGTH    
#define		MAX_TOTAL_POPUP_LENGTH		4000        
#define		MAX_POPUP_LENGTH		100        
#define		MAX_RECORDS			31000      
#define		MAX_RECORD_LENGTH		4000

#define		FLDTYPE_STRING			0x0001
#define		FLDTYPE_BOOLEAN			0x0002
#define		FLDTYPE_DATE			0x0004
#define		FLDTYPE_INT			0x0008
#define		FLDTYPE_FLOAT			0x0010
#define		FLDTYPE_TIME			0x0020
#define		FLDTYPE_LIST			0x0040
#define		FLDTYPE_MULTLIST		0x0051

#define		FLDFLAG_READONLY		0x8000

#define		MAX_DATA_LENGTH_1			490

#define		INFOFLAGS_LOCKONEXIT	0x0001
#define		INFOFLAGS_VIEWONLY		0x0002
#define		INFOFLAGS_STRUCTLOCK	0x0004

typedef struct 
{
	char	fieldNames[MAX_FIELDS][MAX_FIELD_NAME_LENGTH+1];	// holds the field names
	short	fieldTypes[MAX_FIELDS];		// future upgrade holder
	short	numFields;
	short	showDBColumns;
	short	showDBColumnWidths[MAX_FIELDS];
	short 	showDataWidth;
	char	reserved1;			// from the compiler word aligning things
	short	reserved2;
} JFile1AppInfoType;

typedef struct 
{
	char	fieldNames[MAX_FIELDS][MAX_FIELD_NAME_LENGTH+1];
	short	fieldTypes[MAX_FIELDS];			// values are the FLDTYPE_... defines above
	short	numFields;
	short	version;				// should be equal to the CURRENT_VERSION above
	short	showDBColumnWidths[MAX_FIELDS];		// width in pixels to display for each column
	short 	showDataWidth;				// width in pixels of the data area when editing a record
	short	sort1Field;				// which fields were last chosen to sort on
	short	sort2Field;				// secondary...
	short	sort3Field;				// tertiary....
	short	findField;				// which field was last 'Find'
	short	filterField;				// which field was last 'Filter'
	char	findString[MAX_FIND_STRING];		// the previous 'Find' string
	char	filterString[MAX_FIND_STRING];		// the previous 'Filter' string
	short	flags;						// 
	short	firstColumnToShow;			// which field to show as the first field 
	char	password[MAX_PASSWORD_LENGTH+2];	// password for this db, or the empty string
//	char*	popupLists;		
} JFile2AppInfoType;


#pragma once

#define		CURRENT_VERSION			452
#define		MAX_FIND_STRING			16
#define		MAX_PASSWORD_LENGTH		10
#define 	MAX_FIELDS			20
#define 	MAX_FIELD_NAME_LENGTH		20     
#define		MAX_DATA_LENGTH			500    
#define		MAX_TOTAL_POPUP_LENGTH		4000        
#define		MAX_RECORDS			5000      
#define		MAX_RECORD_LENGTH		4000

#define		FLDTYPE_STRING			0x0001
#define		FLDTYPE_BOOLEAN			0x0002
#define		FLDTYPE_DATE			0x0004
#define		FLDTYPE_INT			0x0008
#define		FLDTYPE_FLOAT			0x0010
#define		FLDTYPE_LIST			0x0040

typedef struct 
{
	char	fieldNames[MAX_FIELDS][MAX_FIELD_NAME_LENGTH+1];
	int	fieldTypes[MAX_FIELDS];			// values are the FLDTYPE_... defines above
	int	numFields;
	int	version;				// should be equal to the CURRENT_VERSION above
	int	showDBColumnWidths[MAX_FIELDS];		// width in pixels to display for each column
	int 	showDataWidth;				// width in pixels of the data area when editing a record
	int	sort1Field;				// which fields were last chosen to sort on
	int	sort2Field;				// secondary...
	int	sort3Field;				// tertiary....
	int	findField;				// which field was last 'Find'
	int	filterField;				// which field was last 'Filter'
	char	findString[MAX_FIND_STRING];		// the previous 'Find' string
	char	filterString[MAX_FIND_STRING];		// the previous 'Filter' string
	int	lockPWOnExit;				// 1 if auto-lock a password'ed db is on, 0 otherwise
	int	firstColumnToShow;			// which field to show as the first field 
	char	password[MAX_PASSWORD_LENGTH+2];	// password for this db, or the empty string
	char*	popupLists;		
} JFileAppInfoType;


typedef struct
{
	char	fieldData[MAX_FIELDS][MAX_DATA_LENGTH];		// holds the data for each field
	char	reserved1;
	short 	reserved2;
} JBaseRecordType;      

typedef unsigned short WORD;
typedef unsigned long DWORD;   
typedef unsigned char BYTE; 
typedef unsigned long tTime;         
             
struct tHeader { 
         char sTitle[32]; // description of this collection 
         WORD wAttr;      // 8=request for backup
         WORD wVersion; 
         tTime tCreated; 
         tTime tModded;   // in "seconds since Jan 1, 1970" 
         tTime tLastHS;   // time of last hotsync
         long dwRes0;
         long ofsAttributes; //
         long ofsCategories; //
         DWORD dwType;       // 'Data'
         DWORD dwCreator;    // 'dwDP'
         DWORD dwRes1;
         DWORD dwRes2;      // reserved
         WORD nRecs;        // number of records 
};                          

struct tRecEntry { 
     union { DWORD ofs; // on disk, file offset to this record 
             char* pRec; // in ram, this is a pointer to the compressed data 
           };
           DWORD _id; // one byte with flags from kRec* above 
                      // merged with 3 bytes of unique ID 
};

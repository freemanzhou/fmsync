/*******************************************************************
 * 							 Touchdown Software
 *
 *		 Copyright(c) 1994, Palm Computing Inc., All Rights Reserved
 *
 *-------------------------------------------------------------------
 * FileName:
 *		DataPrv.h
 *
 * Description:
 *		Private Header for the Data Manager
 *
 * History:
 *   	8/10/95  RM - Created by Ron Marianetti
 *
*******************************************************************/

#ifndef __DATAPRV_H__
#define __DATAPRV_H__

#include "syncmgr.h"

#if !macintosh
	#pragma pack(1)
#else
	#pragma options align= packed
#endif

//
//  Remap touchdown types to PC types
//
#ifndef LocalID
#define LocalID DWORD
#endif


#if !macintosh
#ifndef Byte
#define Byte  BYTE
#endif

#ifndef Word
#define Word unsigned short
#endif

#ifndef DWord
#define DWord DWORD
#endif


#ifndef ULong
#define ULong unsigned long
#endif
#endif

// Attributes of a Database
#define	dmHdrAttrResDB				0x0001	// Resource database
#define dmHdrAttrReadOnly			0x0002	// Read Only database
#define	dmHdrAttrAppInfoDirty		0x0004	// Set if Application Info block is dirty
// Optionally supported by an App's conduit
#define	dmHdrAttrBackup				0x0008	//	Set if database should be backed up to PC if
//	no app-specific synchronization conduit has
//	been supplied.
#define	dmHdrAttrOKToInstallNewer 	0x0010	// This tells the backup conduit that it's OK
//  for it to install a newer version of this database
//  with a different name if the current database is
//  open. This mechanism is used to update the 
//  Graffiti Shortcuts database, for example. 
#define	dmHdrAttrResetAfterInstall	0x0020 	// Device requires a reset after this database is 
// installed.
#define	dmHdrAttrOpen				0x8000	// Database not closed properly


/************************************************************
 * Structure of a Record entry
*************************************************************/
typedef struct
{
	LocalID localChunkID;						// local chunkID of a record
	Byte attributes;							// record attributes;
	Byte uniqueID[3];							// unique ID of record
} RecordEntryType;
typedef RecordEntryType* RecordEntryPtr;



/************************************************************
 * Structure of a Resource entry
*************************************************************/
typedef struct
{
	DWord type;									// resource type
	Word id;									// resource id
	LocalID localChunkID;						// resource data chunk ID
} RsrcEntryType;
typedef RsrcEntryType* RsrcEntryPtr;

// Attributes field
#define	dmRsrcAttrUnused		0x0000	// to be defined...



/************************************************************
 * Structure of a record list extension. This is used if all
 *  the database record/resource entries of a database can't fit into
 *  the database header.
*************************************************************/
typedef struct
{
	LocalID nextRecordListID;					// local chunkID of next list
	Word numRecords;							// number of records in this list
	Word firstEntry;							// array of Record/Rsrc entries 
	// starts here
} RecordListType;
typedef RecordListType* RecordListPtr;




/************************************************************
 * Structure of a Database Header
*************************************************************/
typedef struct
{
	Byte name[DB_NAMELEN];						// name of database
	Word attributes;							// database attributes
	Word version;								// version of database

	DWord creationDate;							// creation date of database
	DWord modificationDate;						// latest modification date
	DWord lastBackupDate;						// latest backup date
	DWord modificationNumber;					// modification number of database

	LocalID appInfoID;							// application specific info
	LocalID sortInfoID;							// app specific sorting info

	DWord type;									// database type
	DWord creator;								// database creator 

	DWord uniqueIDSeed;							// used to generate unique IDs

	RecordListType recordList;					// first record list
} DatabaseHdrType;

typedef DatabaseHdrType* DatabaseHdrPtr;
typedef DatabaseHdrPtr* DatabaseHdrHand;






/************************************************************
 * The owner ID of chunks allocated by the Database Manager
*************************************************************/
#define	dmDynOwnerID				0x00		// Dynamic heap chunks
#define	dmMgrOwnerID				0x01		// Management chunks
#define	dmRecOwnerID				0x02		// Record chunks
#define	dmOrphanOwnerID				0x03		// Orphaned record chunks



#if !macintosh
	#pragma pack()
#else
	#pragma options align= reset
#endif


#endif // __DATAPRV_H__



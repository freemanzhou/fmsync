/*
	File:		HotSyncDefines.h

	Contains:	xxx put contents here xxx

	Created by:	Chris LeCroy on Wed, Aug 20, 1997

	Copyright:	© 1997 by Now Software, Inc., all rights reserved.

	Change History (most recent first):

		<20>	 10/1/97	csl		
		 <8>	  9/8/97	csl		new msgs & DLL file type
		 <5>	  9/5/97	csl		Ran it through CDent
		 <2>	 8/22/97	csl		merged some junk from the Windows port in (I'll probably remove
									it again later)
				00/00/00---	file 	created

	To Do:
*/

//	The HotSync project has the following default ranges assigned to different parts of the project:
//	non-global things 	0 		-  9999
//	stuff in Util		10,000 	- 19,999
//	stuff in Shared		20,000	- 32,000
//  this is most important for resource id's, but also applies to such things as message numbers

#pragma once

#ifndef __HotSyncDefines__
#define __HotSyncDefines__




// Creator Types for the HotSync Project  (code name Gladiator)
enum
{
	kHSM_creator			=	'Gld1',
	kSPM_creator			=	'Gld2',
	kCM_creator				=	'Gld3',
	kPPInst_creator 		= 	'Gld4',
	kUsersFile				=	'Gld5',
	kConduitSettingsFile	=	'Gld6',
	kHotSyncLogFile			=	'Gld7',
	kPrefsFile				=	'Gld8',
	kConduitDLLFileType		=	'Gld9',
	kGenericHSFileType		=	'Gld0'	//backup files, etc...
};

// message numbers defined globally				   ioParam contents
enum
{
	msg_UserListChanged				= 		10000,		// nil
	msg_ProgressInfo				= 		10001,		// ProgressInfo*
	msg_LogMessage					= 		10002, 		// LogInfo*
	msg_AbortHotSync				= 		10003,		// nil
	msg_CurrentUserChanged			= 		10004,		// CPalmPilotUser*
	msg_AddThisApplication			= 		10005,		// FSSpec*
	msg_AppSelectionChanged			= 		10006,		// cell
	msg_ActionSelectionChanged		= 		10007,		// cell	
	msg_ActionChanged				= 		10008,		// cell
	msg_ChooseDefaultAction			= 		10009,		// cell
	msg_LogChanged					= 		10010,		// nil
	msg_PopupAskPermissionToChange	= 		10011,		// AllowStruct*
	msg_HotSyncStarted				= 		10012,		// nil
	msg_HotSyncEnded				= 		10013,		// nil
	msg_HotSyncPrefsChanged			= 		10014,		// nil
	msg_UpdateAllWindows			= 		10015,		// nil
	msg_RepopulateAppList			= 		10016		// nil
};


//AppleEvent constants
enum
{
//Applevent Suites
	kAEHotSyncSuite			= kHSM_creator,
	
//AppleEvents	
	kAEBroadcastMessage		= 'Mesg',
	kAEEstablishConnection	= 'Sync',
	kAEHotSyncStarted		= 'BegS',
	kAEHotSyncEnded			= 'EndS',
	kAEPrefsChanged			= 'Pref',
	
//Message IDs
	keyHSMessageID			= 'msID',
	keyHSMessageParam		= 'msPr',
	keyHSConnectionOrigin	= 'ConO',
	keyHSWakeupPacket		= 'Wake',
	kHSInputDriverRefNum	= 'IDrv',
	kHSOutputDriverRefNum	= 'ODrv'
};
const long	ae_BroadcastMessage			= 11000;
const long	ae_EstablishConnection		= 11001;

// constants for the shared file strings resource
const short kFileStringsID = 20000;
enum
{
	kPrefsFileIndex = 1,	// the name of the hotsync prefs file
	kUsersFolder,			// the folder in the hotsync folder that contains users
	kUsersFileIndex,		// the name of the users file (inside the users folder)
	kConduitsFolder,		// the name of the conduits folder (inside the users folder)
	kConduitsFileIndex,		// the name of the conduits file in the conduits folder
	kConduitSettingsFolder,	// the name of the folder for use by conduits
	kHotSyncLogFileIndex,	// name of the hotsync log file
	kSerialPortMonitorName,	// name of the serial port monitor app (in extensions) (not used)
	kPPUpdatesFolder,		// name of the general updates installation folder
	kFilesToInstall,		// formerly known as the applications folder
	kBackupsFolder,			// a place where archives are kept
	kDisabledConduitsFolder,// where we move "extra" conduits that work on same database as another
	kHelpFileName,			// name of the quick help help file in app folder
	kConduitSettingsFileEnd,// appended to conduit name to make settings file name
	kBackupConduitSettings	// used by backup and install conduits for finding backup settings
};

const short kMiscStrings = 20001;
enum
{
	kDotPRC				= 1,
	kDotPDB,
	kDotPNC,
	kDotSRC,
	kQuoteString,
	kDamagedPrefsAlert,
	kDamagedUsersFileAlert,
	kDoubleKlickLog,
	kSPMnotFound,
	kQuickHelpNotFound,
	kHelpFileNotFound,
	kDuplicateConduits,
	kPilotSameAsProfileName,
	kPasswordIncorrect
};

const short kActionStrings = 20002;
enum
{
	kSynchronize	= 1,
	kMacOverwrites,
	kPalmPilotOverwrites,
	kDoNothingAtAll
};


//"CHotSyncLog.r" strings
enum
{
	kSTRx_HotSyncLog 			= 28000,
	kstr_LogLineBreak			= 1,
	kstr_ConduitSucceeded 		= 2,
	kstr_SyncWithWarnings 		= 3,
	kstr_SyncFailed 			= 4,
	kstr_HotSyncStarted 		= 5,
	kstr_FoundPilotUser	 		= 6,
	kstr_ROMListing	 			= 7,
	kstr_RAMListing	 			= 8,
	kstr_PilotFileListHeader	= 9,
	kstr_PilotFileListing	 	= 10,
	kstr_Loading			 	= 11,
	kstr_ConduitVersion			= 12,
	kstr_SyncTypeIs				= 13,
	kstr_RemoteDBNameIs			= 14,
	kstr_PilotInfoUpdated		= 15,
	kstr_HotSyncComplete		= 16,
	kstr_HotSyncCanceled		= 17,
	kstr_HotSyncStoppedMemory	= 18,
	kstr_BackupConduitNotFound	= 19,
	kstr_InstallConduitNotFound	= 20,
	kstr_ConduitFailed			= 21,
	kstr_FirstTimePC			= 22,
	kstr_FirstTimePilot			= 23,
	kstr_DuplicateConduits		= 24,
	kstr_ModemBeforeLocal		= 25,
	kstr_CableSync				= 26,
	kstr_ModemSync				= 27,
	kstr_SyncPort				= 28,
	kstr_SyncSpeed				= 29,
	kstr_DuplicateUserName		= 30,
	kstr_SyncDidNothing			= 31
};

enum
{
	kSTRx_HotSyncLogSub 		= 28001,
	kstr_Fast		 			= 1,
	kstr_Slow			 		= 2,
	kstr_HHToPC 				= 3,
	kstr_PCToHH			 		= 4,
	kstr_Install		 		= 5,
	kstr_Backup		 			= 6,
	kstr_DoNothing	 			= 7,
	kstr_ProfileInstall			= 8
};

enum
{
	kSTRx_ConduitErrors 		= 28002,
	kstr_CorruptedPilotDB		= 1
};


const int kAlertDialogID = 10000;

// error codes
#define FATAL_ERROR_MASK			0x10000000
#define	HOTERR_NONE					0x0000
#define	HOTERR_FIRST				0x8000
#define	HOTERR_USERCANCEL			FATAL_ERROR_MASK + HOTERR_FIRST + 0x01
#define	HOTERR_LOGFILE				FATAL_ERROR_MASK + HOTERR_FIRST + 0x02
#define	HOTERR_READDBLIST			FATAL_ERROR_MASK + HOTERR_FIRST + 0x03
#define	HOTERR_ESTABLISHUSER		FATAL_ERROR_MASK + HOTERR_FIRST + 0x04
#define	HOTERR_RECVWAKEUP			FATAL_ERROR_MASK + HOTERR_FIRST + 0x05
#define	HOTERR_TASKMASTER			FATAL_ERROR_MASK + HOTERR_FIRST + 0x06
#define	HOTERR_CONDUITMANAGER		FATAL_ERROR_MASK + HOTERR_FIRST + 0x07
#define	HOTERR_SETCONNECTION		FATAL_ERROR_MASK + HOTERR_FIRST + 0x08
#define	HOTERR_NOTIFYSYNC			FATAL_ERROR_MASK + HOTERR_FIRST + 0x09
#define	HOTERR_DISKSPACE			FATAL_ERROR_MASK + HOTERR_FIRST + 0x0A
#define HOTERR_BAD_PASSWORD			FATAL_ERROR_MASK + HOTERR_FIRST + 0x0B
#define	HOTERR_GENERAL_ERROR		FATAL_ERROR_MASK + HOTERR_FIRST + 0x40

#define MIN_CONDUIT_VERSION			0x00000101
#define MAX_CONDUIT_VERSION			0x00000200

// Constants to be stored as the HotSync Activity Setting
// NOTE: These are based on the order of the radio buttons 
#define HOTSYNC_ACTIVITY_ALWAYS				0
#define HOTSYNC_ACTIVITY_DURING_JERRY		1
#define HOTSYNC_ACTIVITY_MANUAL				2

/*enum MacPortNum 
{
	eInvalidPort = -1,
	eModemPort = 0,
	ePrinterPort = 1	
};
*/

// HotSync connection type enumeration
#if 0	//see "eConnType" in "syncmgr.h"
typedef enum
{
	eHSConnUnknown,				// unknown origin (used when not connected)

	eHSConnLocal,					// connection originated via a direct Pilot-to-PC
										// connection (as in Local HotSync or LanSync)

	eHSConnRemote					// connection originated via a remote Pilot-to-PC
										// connection (as in Modem HotSync or NetSync)
} HSConnEnum;
#endif

typedef enum
{
	eHSPrimaryPCNotPrimary,
	eHSPrimaryPCIsPrimary,
	eHSPrimaryPCUnknown
} HSPrimaryPCEnum;

typedef enum
{
	eHSLocaleUnknown,
	eHSLocaleLocal,				// do local sync
	eHSLocaleLanRelay,			// relay over the Lan (LanSync)
	eHSLocaleNone					// no locale - hotsync will be aborted
} HSLocaleEnum;

//
// psuedo creators
//
#define INSTALL_TASK_CREATOR	0x00000000
#define TCP_TASK_CREATOR		0x00000001
#define BACKUP_TASK_CREATOR		0xFFFFFFFF

//
// database types
//
#define APPLICATION_IDENTIFIER	'appl'		// appl
#define DATA_IDENTIFIER			'DATA'		// DATA



#endif	//__HotSyncDefines__

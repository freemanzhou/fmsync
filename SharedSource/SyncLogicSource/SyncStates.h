// SyncStates.h
// Created by Chris LeCroy on Thu, Dec 4, 1997 @ 10:48 PM.

#ifndef __SyncStates__
#define __SyncStates__


// State = status of record
// Actions = Action to take
// Remote = Handheld
// Local = Macintosh or PC

// Description of states
// No Change = no changes since last HotSync
// Modify = Record modified since last HotSync
// ArchiveNoModify = Record to be archived - not modified since last HotSync
// ArchiveModify = Record to be archived -  has been modified since last HotSync
// Delete = Record to be deleted
// Add = Record has been added since last HotSync
// NoRecord = Status indicating there is no existing record
// ReadOnly = Record is a ReadOnly Record - can not be modified (Local Only)


// Description of Actions
// None = Do nothing
// Archive = Record is archived then deleted from database
// Delete = Record is deleted from database
// RemoteReplaceLocal = Record on the Mac/PC is replaced with the HH record
// LocalReplaceRemote = Record on the HH is replaced with the Mac/PC record
// SyncError = Indicates a condition which should not logically occur (also logs error)
// Log = Send message to log


/*
	The following are the atomic states that a pair of records might be in.
	combination of "states".  Each record pair is examined and and the appropriate
	state bits are set. 
*/

enum
{
	kRemoteNoChange			= 0x00000001,
	kRemoteModify 			= 0x00000002,
//	kRemoteAdd 				= 0x00000004,
	kRemoteDelete 			= 0x00000008,
	kRemoteArchiveModify 	= 0x00000010,
	kRemoteArchiveNoModify	= 0x00000020,
	kRemoteNoRecord 		= 0x00000040,
	
	kLocalNoChange			= 0x00000100,
	kLocalModify 			= 0x00000200,
//	kLocalAdd 				= 0x00000400,
	kLocalDelete 			= 0x00000800,
	kLocalArchiveModify 	= 0x00001000,
	kLocalArchiveNoModify	= 0x00002000,
	kLocalNoRecord 			= 0x00004000,
	kLocalReadOnly			= 0x00008000,
	
	kRecordDataUnequal		= 0x00010000,
	kRecordDataEqual 		= 0x00020000
};
const UInt32 kStatesCount = 32;

/*
	The following are the atomic actions that may be taken in response to a particular
	combination of "states".  More than one atomic action may be taken for some
	state oombination.  For example a full record deletion would be represented as:
		
		kActionLocalDelete | kActionRemoteDelete
	
	IMPORTANT:
	Atomic actions are performed in right-to-left-bitwise order (ie, the actions with smaller
	numbers happen first).
*/
enum
{
	kSyncError					= 0x00000000,
	
	kActionLocalAddToRemote		= 0x00000001,
	kActionRemoteAddToLocal		= 0x00000002,
	
	kActionRemoteArchive		= 0x00000010,	//really means archive *and* delete
	kActionRemoteReplaceLocal	= 0x00000020,
	kActionRemoteDelete			= 0x00000040,
	
	kActionLocalArchive			= 0x00000100,	//really means archive *and* delete
	kActionLocalReplaceRemote	= 0x00000200,
	kActionLocalDelete			= 0x00000400,

	kActionNone					= 0x01000000,
	kActionLog					= 0x02000000
};
const UInt32 kActionsCount = 32;


struct StateAndAction
{
	UInt32 state;
	UInt32 actions;
};

const StateAndAction kStateToActionMap[] = 
{	//	State																	Actions
	//	-----																	-------
	{ kRemoteNoChange | kLocalModify | kRecordDataEqual ,						kActionNone },
	{ kRemoteNoChange | kLocalArchiveNoModify | kRecordDataEqual ,				kActionLocalArchive | kActionRemoteDelete },
	{ kRemoteNoChange | kLocalArchiveModify | kRecordDataEqual ,				kActionLocalArchive | kActionRemoteDelete },
	{ kRemoteNoChange | kLocalDelete | kRecordDataEqual ,						kActionLocalDelete | kActionRemoteDelete },
//	{ kRemoteNoChange | kLocalAdd | kRecordDataEqual ,							kSyncError },
	{ kRemoteNoChange | kLocalNoRecord | kRecordDataEqual ,						kSyncError },
	{ kRemoteNoChange | kLocalNoChange | kRecordDataEqual ,						kActionNone },
	{ kRemoteModify | kLocalModify | kRecordDataEqual ,							kActionNone },
	{ kRemoteModify | kLocalArchiveNoModify | kRecordDataEqual ,				kActionRemoteReplaceLocal | kActionLog },
	{ kRemoteModify | kLocalArchiveModify | kRecordDataEqual ,					kActionRemoteDelete | kActionLocalArchive },
	{ kRemoteModify | kLocalDelete | kRecordDataEqual ,							kActionRemoteReplaceLocal | kActionLog },
//	{ kRemoteModify | kLocalAdd | kRecordDataEqual ,							kSyncError },
	{ kRemoteModify | kLocalNoRecord | kRecordDataEqual ,						kSyncError },
	{ kRemoteModify | kLocalNoChange | kRecordDataEqual ,						kActionNone },	
	{ kRemoteArchiveNoModify | kLocalModify | kRecordDataEqual ,				kActionLocalReplaceRemote | kActionLog },
	{ kRemoteArchiveNoModify | kLocalArchiveNoModify | kRecordDataEqual ,		kActionRemoteArchive | kActionLocalDelete },
	{ kRemoteArchiveNoModify | kLocalArchiveModify | kRecordDataEqual ,			kActionRemoteArchive | kActionLocalDelete | kActionLog },
	{ kRemoteArchiveNoModify | kLocalDelete | kRecordDataEqual ,				kActionRemoteArchive | kActionLocalDelete },
//	{ kRemoteArchiveNoModify | kLocalAdd | kRecordDataEqual ,					kSyncError },
	{ kRemoteArchiveNoModify | kLocalNoRecord | kRecordDataEqual ,				kSyncError },
	{ kRemoteArchiveNoModify | kLocalNoChange | kRecordDataEqual ,				kActionRemoteArchive | kActionLocalDelete },	
	{ kRemoteArchiveModify | kLocalModify | kRecordDataEqual ,					kActionRemoteArchive | kActionLocalDelete },
	{ kRemoteArchiveModify | kLocalArchiveNoModify | kRecordDataEqual ,			kActionRemoteArchive | kActionLocalDelete },
	{ kRemoteArchiveModify | kLocalArchiveModify | kRecordDataEqual ,			kActionRemoteArchive | kActionLocalDelete },
	{ kRemoteArchiveModify | kLocalDelete | kRecordDataEqual ,					kActionRemoteArchive | kActionLocalDelete },
//	{ kRemoteArchiveModify | kLocalAdd | kRecordDataEqual ,						kSyncError },
	{ kRemoteArchiveModify | kLocalNoRecord | kRecordDataEqual ,				kSyncError },
	{ kRemoteArchiveModify | kLocalNoChange | kRecordDataEqual ,				kActionRemoteArchive | kActionLocalDelete },		
	{ kRemoteDelete | kLocalModify | kRecordDataEqual ,							kActionLocalReplaceRemote | kActionLog },
	{ kRemoteDelete | kLocalArchiveNoModify | kRecordDataEqual ,				kActionRemoteDelete | kActionLocalArchive },
	{ kRemoteDelete | kLocalArchiveModify | kRecordDataEqual ,					kActionRemoteDelete | kActionLocalArchive },
	{ kRemoteDelete | kLocalDelete | kRecordDataEqual ,							kActionRemoteDelete | kActionLocalDelete },
//	{ kRemoteDelete | kLocalAdd | kRecordDataEqual ,							kSyncError },
	{ kRemoteDelete | kLocalNoRecord | kRecordDataEqual ,						kSyncError },
	{ kRemoteDelete | kLocalNoChange | kRecordDataEqual ,						kActionRemoteDelete },
//	{ kRemoteAdd | kLocalModify | kRecordDataEqual ,							kSyncError },
//	{ kRemoteAdd | kLocalArchiveNoModify | kRecordDataEqual ,					kSyncError },
//	{ kRemoteAdd | kLocalArchiveModify | kRecordDataEqual ,						kSyncError },
//	{ kRemoteAdd | kLocalDelete | kRecordDataEqual ,							kSyncError },
//	{ kRemoteAdd | kLocalAdd | kRecordDataEqual ,								kSyncError },
//	{ kRemoteAdd | kLocalNoRecord | kRecordDataEqual ,							kSyncError },
//	{ kRemoteAdd | kLocalNoChange | kRecordDataEqual ,							kSyncError },
	{ kRemoteNoRecord | kLocalModify | kRecordDataEqual ,						kSyncError },
	{ kRemoteNoRecord | kLocalArchiveNoModify | kRecordDataEqual ,				kSyncError },
	{ kRemoteNoRecord | kLocalArchiveModify | kRecordDataEqual ,				kSyncError },	
	{ kRemoteNoRecord | kLocalDelete | kRecordDataEqual ,						kSyncError },
//	{ kRemoteNoRecord | kLocalAdd | kRecordDataEqual ,							kSyncError },
	{ kRemoteNoRecord | kLocalNoRecord | kRecordDataEqual ,						kSyncError },
	{ kRemoteNoRecord | kLocalNoChange | kRecordDataEqual ,						kActionLocalAddToRemote },

	{ kRemoteNoChange | kLocalModify | kRecordDataUnequal ,						kActionLocalReplaceRemote },
	{ kRemoteNoChange | kLocalArchiveNoModify | kRecordDataUnequal ,			kActionRemoteDelete | kActionLocalArchive },
	{ kRemoteNoChange | kLocalArchiveModify | kRecordDataUnequal ,				kActionRemoteDelete | kActionLocalArchive },
	{ kRemoteNoChange | kLocalDelete | kRecordDataUnequal ,						kActionRemoteDelete | kActionLocalDelete },
//	{ kRemoteNoChange | kLocalAdd | kRecordDataUnequal ,						kSyncError },
	{ kRemoteNoChange | kLocalNoRecord | kRecordDataUnequal ,					kActionRemoteAddToLocal },
	{ kRemoteNoChange | kLocalNoChange | kRecordDataUnequal ,					kActionLocalAddToRemote | kActionRemoteAddToLocal | kActionLog },
	{ kRemoteModify | kLocalModify | kRecordDataUnequal ,						kActionLocalAddToRemote | kActionRemoteAddToLocal | kActionLog },
	{ kRemoteModify | kLocalArchiveNoModify | kRecordDataUnequal ,				kActionRemoteReplaceLocal | kActionLog },
	{ kRemoteModify | kLocalArchiveModify | kRecordDataUnequal ,				kActionLocalAddToRemote | kActionRemoteAddToLocal | kActionLog },	
	{ kRemoteModify | kLocalDelete | kRecordDataUnequal ,						kActionRemoteReplaceLocal | kActionLog },
//	{ kRemoteModify | kLocalAdd | kRecordDataUnequal ,							kSyncError },
	{ kRemoteModify | kLocalNoRecord | kRecordDataUnequal ,						kActionRemoteAddToLocal },
	{ kRemoteModify | kLocalNoChange | kRecordDataUnequal ,						kActionRemoteReplaceLocal },
	{ kRemoteArchiveNoModify | kLocalModify | kRecordDataUnequal ,				kActionLocalReplaceRemote | kActionLog },
	{ kRemoteArchiveNoModify | kLocalArchiveNoModify | kRecordDataUnequal ,		kActionRemoteArchive | kActionLocalArchive },
	{ kRemoteArchiveNoModify | kLocalArchiveModify | kRecordDataUnequal ,		kActionRemoteArchive | kActionLocalArchive },
	{ kRemoteArchiveNoModify | kLocalDelete | kRecordDataUnequal ,				kActionRemoteArchive | kActionLocalDelete },
//	{ kRemoteArchiveNoModify | kLocalAdd | kRecordDataUnequal ,					kSyncError },
	{ kRemoteArchiveNoModify | kLocalNoChange | kRecordDataUnequal ,			kActionRemoteArchive | kActionLocalDelete },
	{ kRemoteArchiveNoModify | kLocalNoRecord | kRecordDataUnequal ,			kActionRemoteArchive },
	{ kRemoteArchiveModify | kLocalModify | kRecordDataUnequal ,				kActionLocalAddToRemote | kActionRemoteAddToLocal | kActionLog },
	{ kRemoteArchiveModify | kLocalArchiveNoModify | kRecordDataUnequal ,		kActionRemoteArchive | kActionLocalArchive },
	{ kRemoteArchiveModify | kLocalArchiveModify | kRecordDataUnequal ,			kActionRemoteArchive | kActionLocalArchive },
	{ kRemoteArchiveModify | kLocalDelete | kRecordDataUnequal ,				kActionRemoteArchive | kActionLocalDelete },
//	{ kRemoteArchiveModify | kLocalAdd | kRecordDataUnequal ,					kSyncError },
	{ kRemoteArchiveModify | kLocalNoChange | kRecordDataUnequal ,				kActionRemoteArchive | kActionLocalDelete },
	{ kRemoteArchiveModify | kLocalNoRecord | kRecordDataUnequal ,				kActionRemoteArchive },
	{ kRemoteDelete | kLocalModify | kRecordDataUnequal ,						kActionRemoteReplaceLocal | kActionLog },
	{ kRemoteDelete | kLocalArchiveNoModify | kRecordDataUnequal ,				kActionRemoteDelete | kActionLocalArchive },
	{ kRemoteDelete | kLocalArchiveModify | kRecordDataUnequal ,				kActionRemoteDelete | kActionLocalArchive },
	{ kRemoteDelete | kLocalDelete | kRecordDataUnequal ,						kActionRemoteDelete | kActionLocalDelete },
//	{ kRemoteDelete | kLocalAdd | kRecordDataUnequal ,							kSyncError },
	{ kRemoteDelete | kLocalNoRecord | kRecordDataUnequal ,						kActionRemoteDelete },
	{ kRemoteDelete | kLocalNoChange | kRecordDataUnequal ,						kActionRemoteDelete | kActionLocalDelete },
//	{ kRemoteAdd | kLocalModify | kRecordDataUnequal ,							kSyncError },
//	{ kRemoteAdd | kLocalArchiveNoModify | kRecordDataUnequal ,					kSyncError },
//	{ kRemoteAdd | kLocalArchiveModify | kRecordDataUnequal ,					kSyncError },	
//	{ kRemoteAdd | kLocalDelete | kRecordDataUnequal ,							kSyncError },
//	{ kRemoteAdd | kLocalAdd | kRecordDataUnequal ,								kSyncError },
//	{ kRemoteAdd | kLocalNoChange | kRecordDataUnequal ,						kSyncError },
//	{ kRemoteAdd | kLocalNoRecord | kRecordDataUnequal ,						kActionRemoteAddToLocal },
	{ kRemoteNoRecord | kLocalModify | kRecordDataUnequal ,						kActionLocalAddToRemote },
	{ kRemoteNoRecord | kLocalArchiveNoModify | kRecordDataUnequal ,			kActionLocalArchive },
	{ kRemoteNoRecord | kLocalArchiveModify | kRecordDataUnequal ,				kActionLocalArchive },
	{ kRemoteNoRecord | kLocalDelete | kRecordDataUnequal ,						kActionLocalDelete },
//	{ kRemoteNoRecord | kLocalAdd | kRecordDataUnequal ,						kActionLocalAddToRemote },
	{ kRemoteNoRecord | kLocalNoRecord | kRecordDataUnequal ,					kSyncError },
	{ kRemoteNoRecord | kLocalNoChange | kRecordDataUnequal ,					kActionLocalAddToRemote },	
	

	{ kRemoteNoChange | kLocalReadOnly | kRecordDataUnequal ,					kActionLocalReplaceRemote },
	{ kRemoteModify | kLocalReadOnly | kRecordDataUnequal ,						kActionLocalAddToRemote | kActionRemoteAddToLocal | kActionLog },
	{ kRemoteArchiveNoModify | kLocalReadOnly | kRecordDataUnequal ,			kActionRemoteArchive | kActionLocalAddToRemote | kActionLog },
	{ kRemoteArchiveModify | kLocalReadOnly | kRecordDataUnequal ,				kActionRemoteArchive | kActionLocalAddToRemote | kActionLog },
	{ kRemoteDelete | kLocalReadOnly | kRecordDataUnequal ,						kActionRemoteDelete | kActionLocalAddToRemote | kActionLog },
//	{ kRemoteAdd | kLocalReadOnly | kRecordDataUnequal ,						kSyncError },
	{ kRemoteNoRecord | kLocalReadOnly | kRecordDataUnequal ,					kSyncError },


	{ kRemoteNoChange | kLocalReadOnly | kRecordDataEqual ,						kActionNone },
	{ kRemoteModify | kLocalReadOnly | kRecordDataEqual ,						kActionNone },
	{ kRemoteArchiveNoModify | kLocalReadOnly | kRecordDataEqual ,				kActionRemoteArchive | kActionLocalAddToRemote | kActionLog },
	{ kRemoteArchiveModify | kLocalReadOnly | kRecordDataEqual ,				kActionRemoteArchive | kActionLocalAddToRemote | kActionLog },
	{ kRemoteDelete | kLocalReadOnly | kRecordDataEqual ,						kActionLog },
//	{ kRemoteAdd | kLocalReadOnly | kRecordDataEqual ,							kSyncError },
	{ kRemoteNoRecord | kLocalReadOnly | kRecordDataEqual ,						kSyncError }
};
const int kNumberofStates = sizeof(kStateToActionMap) / sizeof(StateAndAction);

#ifdef _DEBUG
const char* kStateToActionMapStrings[kNumberofStates] = 
{	//	State																	Actions
	//	-----																	-------
	"[ kRemoteNoChange | kLocalModify | kRecordDataEqual ,						kActionNone ]",
	"[ kRemoteNoChange | kLocalArchiveNoModify | kRecordDataEqual ,				kActionLocalArchive | kActionRemoteDelete ]",
	"[ kRemoteNoChange | kLocalArchiveModify | kRecordDataEqual ,				kActionLocalArchive | kActionRemoteDelete ]",
	"[ kRemoteNoChange | kLocalDelete | kRecordDataEqual ,						kActionLocalDelete | kActionRemoteDelete ]",
//	"[ kRemoteNoChange | kLocalAdd | kRecordDataEqual ,							kSyncError ]",
	"[ kRemoteNoChange | kLocalNoRecord | kRecordDataEqual ,					kSyncError ]",
	"[ kRemoteNoChange | kLocalNoChange | kRecordDataEqual ,					kActionNone ]",
	"[ kRemoteModify | kLocalModify | kRecordDataEqual ,						kActionNone ]",
	"[ kRemoteModify | kLocalArchiveNoModify | kRecordDataEqual ,				kActionRemoteReplaceLocal | kActionLog ]",
	"[ kRemoteModify | kLocalArchiveModify | kRecordDataEqual ,					kActionRemoteDelete | kActionLocalArchive ]",
	"[ kRemoteModify | kLocalDelete | kRecordDataEqual ,						kActionRemoteReplaceLocal | kActionLog ]",
//	"[ kRemoteModify | kLocalAdd | kRecordDataEqual ,							kSyncError ]",
	"[ kRemoteModify | kLocalNoRecord | kRecordDataEqual ,						kSyncError ]",
	"[ kRemoteModify | kLocalNoChange | kRecordDataEqual ,						kActionNone ]",	
	"[ kRemoteArchiveNoModify | kLocalModify | kRecordDataEqual ,				kActionLocalReplaceRemote | kActionLog ]",
	"[ kRemoteArchiveNoModify | kLocalArchiveNoModify | kRecordDataEqual ,		kActionRemoteArchive | kActionLocalDelete ]",
	"[ kRemoteArchiveNoModify | kLocalArchiveModify | kRecordDataEqual ,		kActionRemoteArchive | kActionLocalDelete | kActionLog ]",
	"[ kRemoteArchiveNoModify | kLocalDelete | kRecordDataEqual ,				kActionRemoteArchive | kActionLocalDelete ]",
//	"[ kRemoteArchiveNoModify | kLocalAdd | kRecordDataEqual ,					kSyncError ]",
	"[ kRemoteArchiveNoModify | kLocalNoRecord | kRecordDataEqual ,				kSyncError ]",
	"[ kRemoteArchiveNoModify | kLocalNoChange | kRecordDataEqual ,				kActionRemoteArchive | kActionLocalDelete ]",	
	"[ kRemoteArchiveModify | kLocalModify | kRecordDataEqual ,					kActionRemoteArchive | kActionLocalDelete ]",
	"[ kRemoteArchiveModify | kLocalArchiveNoModify | kRecordDataEqual ,		kActionRemoteArchive | kActionLocalDelete ]",
	"[ kRemoteArchiveModify | kLocalArchiveModify | kRecordDataEqual ,			kActionRemoteArchive | kActionLocalDelete ]",
	"[ kRemoteArchiveModify | kLocalDelete | kRecordDataEqual ,					kActionRemoteArchive | kActionLocalDelete ]",
//	"[ kRemoteArchiveModify | kLocalAdd | kRecordDataEqual ,					kSyncError ]",
	"[ kRemoteArchiveModify | kLocalNoRecord | kRecordDataEqual ,				kSyncError ]",
	"[ kRemoteArchiveModify | kLocalNoChange | kRecordDataEqual ,				kActionRemoteArchive | kActionLocalDelete ]",		
	"[ kRemoteDelete | kLocalModify | kRecordDataEqual ,						kActionLocalReplaceRemote | kActionLog ]",
	"[ kRemoteDelete | kLocalArchiveNoModify | kRecordDataEqual ,				kActionRemoteDelete | kActionLocalArchive ]",
	"[ kRemoteDelete | kLocalArchiveModify | kRecordDataEqual ,					kActionRemoteDelete | kActionLocalArchive ]",
	"[ kRemoteDelete | kLocalDelete | kRecordDataEqual ,						kActionRemoteDelete | kActionLocalDelete ]",
//	"[ kRemoteDelete | kLocalAdd | kRecordDataEqual ,							kSyncError ]",
	"[ kRemoteDelete | kLocalNoRecord | kRecordDataEqual ,						kSyncError ]",
	"[ kRemoteDelete | kLocalNoChange | kRecordDataEqual ,						kActionRemoteDelete ]",
//	"[ kRemoteAdd | kLocalModify | kRecordDataEqual ,							kSyncError ]",
//	"[ kRemoteAdd | kLocalArchiveNoModify | kRecordDataEqual ,					kSyncError ]",
//	"[ kRemoteAdd | kLocalArchiveModify | kRecordDataEqual ,					kSyncError ]",
//	"[ kRemoteAdd | kLocalDelete | kRecordDataEqual ,							kSyncError ]",
//	"[ kRemoteAdd | kLocalAdd | kRecordDataEqual ,								kSyncError ]",
//	"[ kRemoteAdd | kLocalNoRecord | kRecordDataEqual ,							kSyncError ]",
//	"[ kRemoteAdd | kLocalNoChange | kRecordDataEqual ,							kSyncError ]",
	"[ kRemoteNoRecord | kLocalModify | kRecordDataEqual ,						kSyncError ]",
	"[ kRemoteNoRecord | kLocalArchiveNoModify | kRecordDataEqual ,				kSyncError ]",
	"[ kRemoteNoRecord | kLocalArchiveModify | kRecordDataEqual ,				kSyncError ]",	
	"[ kRemoteNoRecord | kLocalDelete | kRecordDataEqual ,						kSyncError ]",
//	"[ kRemoteNoRecord | kLocalAdd | kRecordDataEqual ,							kSyncError ]",
	"[ kRemoteNoRecord | kLocalNoRecord | kRecordDataEqual ,					kSyncError ]",
	"[ kRemoteNoRecord | kLocalNoChange | kRecordDataEqual ,					kActionLocalAddToRemote ]",

	"[ kRemoteNoChange | kLocalModify | kRecordDataUnequal ,					kActionLocalReplaceRemote ]",
	"[ kRemoteNoChange | kLocalArchiveNoModify | kRecordDataUnequal ,			kActionRemoteDelete | kActionLocalArchive ]",
	"[ kRemoteNoChange | kLocalArchiveModify | kRecordDataUnequal ,				kActionRemoteDelete | kActionLocalArchive ]",
	"[ kRemoteNoChange | kLocalDelete | kRecordDataUnequal ,					kActionRemoteDelete | kActionLocalDelete ]",
//	"[ kRemoteNoChange | kLocalAdd | kRecordDataUnequal ,						kSyncError ]",
	"[ kRemoteNoChange | kLocalNoRecord | kRecordDataUnequal ,					kActionRemoteAddToLocal ]",
	"[ kRemoteNoChange | kLocalNoChange | kRecordDataUnequal ,					kActionLocalAddToRemote | kActionRemoteAddToLocal | kActionLog ]",
	"[ kRemoteModify | kLocalModify | kRecordDataUnequal ,						kActionLocalAddToRemote | kActionRemoteAddToLocal | kActionLog ]",
	"[ kRemoteModify | kLocalArchiveNoModify | kRecordDataUnequal ,				kActionRemoteReplaceLocal | kActionLog ]",
	"[ kRemoteModify | kLocalArchiveModify | kRecordDataUnequal ,				kActionLocalAddToRemote | kActionRemoteAddToLocal | kActionLog ]",	
	"[ kRemoteModify | kLocalDelete | kRecordDataUnequal ,						kActionRemoteReplaceLocal | kActionLog ]",
//	"[ kRemoteModify | kLocalAdd | kRecordDataUnequal ,							kSyncError ]",
	"[ kRemoteModify | kLocalNoRecord | kRecordDataUnequal ,					kActionRemoteAddToLocal ]",
	"[ kRemoteModify | kLocalNoChange | kRecordDataUnequal ,					kActionRemoteReplaceLocal ]",
	"[ kRemoteArchiveNoModify | kLocalModify | kRecordDataUnequal ,				kActionLocalReplaceRemote | kActionLog ]",
	"[ kRemoteArchiveNoModify | kLocalArchiveNoModify | kRecordDataUnequal ,	kActionRemoteArchive | kActionLocalArchive ]",
	"[ kRemoteArchiveNoModify | kLocalArchiveModify | kRecordDataUnequal ,		kActionRemoteArchive | kActionLocalArchive ]",
	"[ kRemoteArchiveNoModify | kLocalDelete | kRecordDataUnequal ,				kActionRemoteArchive | kActionLocalDelete ]",
//	"[ kRemoteArchiveNoModify | kLocalAdd | kRecordDataUnequal ,				kSyncError ]",
	"[ kRemoteArchiveNoModify | kLocalNoChange | kRecordDataUnequal ,			kActionRemoteArchive | kActionLocalDelete ]",
	"[ kRemoteArchiveNoModify | kLocalNoRecord | kRecordDataUnequal ,			kActionRemoteArchive ]",
	"[ kRemoteArchiveModify | kLocalModify | kRecordDataUnequal ,				kActionLocalAddToRemote | kActionRemoteAddToLocal | kActionLog ]",
	"[ kRemoteArchiveModify | kLocalArchiveNoModify | kRecordDataUnequal ,		kActionRemoteArchive | kActionLocalArchive ]",
	"[ kRemoteArchiveModify | kLocalArchiveModify | kRecordDataUnequal ,		kActionRemoteArchive | kActionLocalArchive ]",
	"[ kRemoteArchiveModify | kLocalDelete | kRecordDataUnequal ,				kActionRemoteArchive | kActionLocalDelete ]",
//	"[ kRemoteArchiveModify | kLocalAdd | kRecordDataUnequal ,					kSyncError ]",
	"[ kRemoteArchiveModify | kLocalNoChange | kRecordDataUnequal ,				kActionRemoteArchive | kActionLocalDelete ]",
	"[ kRemoteArchiveModify | kLocalNoRecord | kRecordDataUnequal ,				kActionRemoteArchive ]",
	"[ kRemoteDelete | kLocalModify | kRecordDataUnequal ,						kActionRemoteReplaceLocal | kActionLog ]",
	"[ kRemoteDelete | kLocalArchiveNoModify | kRecordDataUnequal ,				kActionRemoteDelete | kActionLocalArchive ]",
	"[ kRemoteDelete | kLocalArchiveModify | kRecordDataUnequal ,				kActionRemoteDelete | kActionLocalArchive ]",
	"[ kRemoteDelete | kLocalDelete | kRecordDataUnequal ,						kActionRemoteDelete | kActionLocalDelete ]",
//	"[ kRemoteDelete | kLocalAdd | kRecordDataUnequal ,							kSyncError ]",
	"[ kRemoteDelete | kLocalNoRecord | kRecordDataUnequal ,					kActionRemoteDelete ]",
	"[ kRemoteDelete | kLocalNoChange | kRecordDataUnequal ,					kActionRemoteDelete | kActionLocalDelete ]",
//	"[ kRemoteAdd | kLocalModify | kRecordDataUnequal ,							kSyncError ]",
//	"[ kRemoteAdd | kLocalArchiveNoModify | kRecordDataUnequal ,				kSyncError ]",
//	"[ kRemoteAdd | kLocalArchiveModify | kRecordDataUnequal ,					kSyncError ]",	
//	"[ kRemoteAdd | kLocalDelete | kRecordDataUnequal ,							kSyncError ]",
//	"[ kRemoteAdd | kLocalAdd | kRecordDataUnequal ,							kSyncError ]",
//	"[ kRemoteAdd | kLocalNoChange | kRecordDataUnequal ,						kSyncError ]",
//	"[ kRemoteAdd | kLocalNoRecord | kRecordDataUnequal ,						kActionRemoteAddToLocal ]",
	"[ kRemoteNoRecord | kLocalModify | kRecordDataUnequal ,					kActionLocalAddToRemote ]",
	"[ kRemoteNoRecord | kLocalArchiveNoModify | kRecordDataUnequal ,			kActionLocalArchive ]",
	"[ kRemoteNoRecord | kLocalArchiveModify | kRecordDataUnequal ,				kActionLocalArchive ]",
	"[ kRemoteNoRecord | kLocalDelete | kRecordDataUnequal ,					kActionLocalDelete ]",
//	"[ kRemoteNoRecord | kLocalAdd | kRecordDataUnequal ,						kActionLocalAddToRemote ]",
	"[ kRemoteNoRecord | kLocalNoRecord | kRecordDataUnequal ,					kSyncError ]",
	"[ kRemoteNoRecord | kLocalNoChange | kRecordDataUnequal ,					kActionLocalAddToRemote ]",


	"[ kRemoteNoChange | kLocalReadOnly | kRecordDataUnequal ,					kActionLocalReplaceRemote ]",
	"[ kRemoteModify | kLocalReadOnly | kRecordDataUnequal ,					kActionLocalAddToRemote | kActionRemoteAddToLocal | kActionLog ]",
	"[ kRemoteArchiveNoModify | kLocalReadOnly | kRecordDataUnequal ,			kActionRemoteArchive | kActionLocalAddToRemote | kActionLog ]",
	"[ kRemoteArchiveModify | kLocalReadOnly | kRecordDataUnequal ,				kActionRemoteArchive | kActionLocalAddToRemote | kActionLog ]",
	"[ kRemoteDelete | kLocalReadOnly | kRecordDataUnequal ,					kActionRemoteDelete | kActionLocalAddToRemote | kActionLog ]",
//	"[ kRemoteAdd | kLocalReadOnly | kRecordDataUnequal ,						kSyncError ]",
	"[ kRemoteNoRecord | kLocalReadOnly | kRecordDataUnequal ,					kActionLocalAddToRemote ]",


	"[ kRemoteNoChange | kLocalReadOnly | kRecordDataEqual ,					kActionNone ]",
	"[ kRemoteModify | kLocalReadOnly | kRecordDataEqual ,						kActionNone ]",
	"[ kRemoteArchiveNoModify | kLocalReadOnly | kRecordDataEqual ,				kActionRemoteArchive | kActionLocalAddToRemote | kActionLog ]",
	"[ kRemoteArchiveModify | kLocalReadOnly | kRecordDataEqual ,				kActionRemoteArchive | kActionLocalAddToRemote | kActionLog ]",
	"[ kRemoteDelete | kLocalReadOnly | kRecordDataEqual ,						kActionLog ]",
//	"[ kRemoteAdd | kLocalReadOnly | kRecordDataEqual ,							kSyncError ]",
	"[ kRemoteNoRecord | kLocalReadOnly | kRecordDataEqual ,					kSyncError ]"
};
#endif	//_DEBUG



#endif	//__SyncStates__

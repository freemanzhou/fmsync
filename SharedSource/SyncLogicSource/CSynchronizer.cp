/*
	File:		CSynchronizer.cp

	Contains:	xxx put contents here xxx

	Created by:	Chris LeCroy on Sun, Nov 9, 1997

	Copyright:	© 1997 by Now Software, Inc., all rights reserved.

	Change History (most recent first):

				00/00/00	---	file 	created

	To Do:
*/

#include "CSynchronizer.h"
#include "SyncStates.h"
#include "DebugOutput.h"

#include "CSyncRecord.h"
#include "CArchiveIterator.h"

#include "StSyncClasses.h"
#include "Str255.h"
//#include "UHSDebugging.h"

static string AsString(char *const a)
{
	return string(a);
}

const WORD MAX_RECORD_SIZE = 0xFFFE;// Maximum record size (define this is in centrally somehwere)


CSynchronizer::CSynchronizer(const CSyncProperties& inSyncProperties) :
	mCategoriesPresent( true ),
	mTriedPostSync( false ),
	mArchiveIterator(0),
	mLocalRecIterator(0),
	mPilotIterator(0),
	mSharedBuffer(MAX_RECORD_SIZE, !kSharedBufferFreeWhenReleased, kSharedBufferUseTempMem)
{	
	CSystemInfo emptySysInfo = {};
	mSystemInfo = emptySysInfo;
		
	mSyncProperties = inSyncProperties;
	
#ifdef _DEBUG
	SetDebugSignal_(debugAction_Alert);
#endif
}


CSynchronizer::~CSynchronizer()
{	
}

void CSynchronizer::MakeIterators(short inDBIndex)
{
	mPilotIterator = this->MakeRemoteIterator(inDBIndex);
	mLocalRecIterator = this->MakeLocalIterator(inDBIndex);
	mArchiveIterator = this->MakeArchiveIterator(inDBIndex);
}


void CSynchronizer::DeleteIterators()
{
	delete mArchiveIterator;
	mArchiveIterator = nil;
	
	delete mLocalRecIterator;
	mLocalRecIterator = nil;
	
	delete mPilotIterator;
	mPilotIterator = nil;
}


CRecordIterator* CSynchronizer::MakeArchiveIterator(short inDBIndex)
{
	return new CArchiveIterator(mSyncProperties, inDBIndex);
}


void CSynchronizer::DoReplacePCSync()
{}


void CSynchronizer::DoPostSync(bool wasCanceled)
{
	mTriedPostSync = true;
	if(mLocalRecIterator)
		mLocalRecIterator->PostSync(wasCanceled);
	if(mPilotIterator)
		mPilotIterator->PostSync(wasCanceled);
	if(mArchiveIterator)
		mArchiveIterator->PostSync(wasCanceled);
}


void CSynchronizer::SynchronizeCategories()
{
	mCategoriesPresent = mPilotIterator->UsesCategories() &&
			mLocalRecIterator->UsesCategories() &&
			mArchiveIterator->UsesCategories();
	
	if( mCategoriesPresent )
	{
		this->BuildCategoryMapArray();
		
		this->WriteCategoryMapArray();
	}
}
void CSynchronizer::GetCategories()
{
	//get the pilot categories
	CCategory	ioCategory;
	while( mPilotIterator->NextCategory( ioCategory ) )
		mPilotCategories.InsertItemsAt( 1, LArray::index_Last, ioCategory );
	
	//get the local categories
	while( mLocalRecIterator->NextCategory( ioCategory ) )
		mLocalCategories.InsertItemsAt( 1, LArray::index_Last, ioCategory );
}

void CSynchronizer::BuildCategoryMapArray()
{
}

void CSynchronizer::WriteCategoryMapArray()
{
	mCategoryMapArray.Lock();
	for( ArrayIndexT i=1; mCategoryMapArray.ValidIndex( i ); i++ )
	{
		CCategory pilotCategory;
		CCategory localCategory;
		Boolean foundOnPilot=mCategoryMapArray[i].flagBits & CCategoryMap::kValidOnPilot;
		Boolean foundOnLocal=mCategoryMapArray[i].flagBits & CCategoryMap::kValidOnLocal;

		if( foundOnPilot )
			this->ExtractCategory( mCategoryMapArray[i], kPilotSource, pilotCategory );
		if( foundOnLocal )
			this->ExtractCategory( mCategoryMapArray[i], kLocalSource, localCategory );

		Assert_( foundOnPilot || foundOnLocal );
		Assert_( ( mCategoryMapArray[i].action & kCatSyncErr ) == 0x00000000 );

		for( short j=0; j<kNumCatSyncActions; j++ )
		{
			switch( mCategoryMapArray[i].action & ( 1 << j ) )
			{
				case kRemoteAddToLocal:
					Assert_( foundOnPilot );
					mCategoryMapArray[i].localID = pilotCategory.catID;
					mLocalRecIterator->AddCategory( pilotCategory );
					break;
				case kLocalAddToRemote:
					Assert_( foundOnLocal );
					mCategoryMapArray[i].pilotID = localCategory.catID;
					mPilotIterator->AddCategory( localCategory );
					break;
				case kRemoteReplaceLocalName:
					Assert_( foundOnPilot );
					mLocalRecIterator->SetCategoryName( localCategory, pilotCategory.catName );
					break;
				case kLocalReplaceRemoteName:
					Assert_( foundOnLocal );
					mPilotIterator->SetCategoryName( pilotCategory, localCategory.catName );
					break;
				case kRemoteReplaceLocalID:
					Assert_( foundOnPilot );
					Assert_( foundOnLocal );
					mCategoryMapArray[i].localID = pilotCategory.catID;
					mLocalRecIterator->SetCategoryID( localCategory, pilotCategory.catID );
					break;
				case kLocalReplaceRemoteID:
					Assert_( foundOnPilot );
					Assert_( foundOnLocal );
					mCategoryMapArray[i].pilotID = localCategory.catID;
					mPilotIterator->SetCategoryID( pilotCategory, localCategory.catID );
					break;
				case kLocalMapToRemoteUnfiled:
					//do nothing
					break;
			}
		}
	}
	mCategoryMapArray.Unlock();
}

void CSynchronizer::MapCategoriesOntoRecord( CSyncRecord& theRecord, CSynchronizer::SyncDirection direction )
{
	if( !mCategoriesPresent )
	{
		theRecord.SetCategoryID( 0 );
		return;
	}
	CCategoryMapArrayIterator mapIter( mCategoryMapArray );
	
	Boolean foundCat = false;
	CCategoryMap catMap;
	while( !foundCat && mapIter.Next( catMap ) )
		foundCat = ( direction == kLocalToPilot ? catMap.localID : catMap.pilotID ) == theRecord.GetCategoryID();
	if( !foundCat ) Throw_( SYNCERR_NOT_FOUND );
	theRecord.SetCategoryID( direction == kLocalToPilot ? catMap.pilotID : catMap.localID );
}

void CSynchronizer::ExtractCategory( const CCategoryMap& catMap, CategorySource catSource, CCategory& outCategory )
{
	CCategoryArrayIterator catIter( catSource == kPilotSource ? mPilotCategories : mLocalCategories );
	Boolean foundCat=false;
	while( !foundCat && catIter.Next( outCategory ) )
		foundCat = outCategory.catID == ( catSource == kPilotSource ? catMap.pilotID : catMap.localID );
	if( !foundCat ) Throw_( SYNCERR_NOT_FOUND );
}

#ifdef _DEBUG
char* CSynchronizer::SyncStateToString(UInt32 inSyncState)
{
	static char gOutStateStr[255] = {};
	
	gOutStateStr[0] = '\0';
	
	if (inSyncState == 0)
	{
		::strcat(gOutStateStr, "<No State>");
	}
	else
	{
		for (UInt32 i = 0; i < kStatesCount; i++)
		{
			switch (inSyncState & (1 << i))
			{
				case kRemoteModify:
					::strcat(gOutStateStr, "+kRemoteModify");
				break;
				
/*				case kRemoteAdd:
					::strcat(gOutStateStr, "+kRemoteModify");
				break;*/
				
				case kRemoteDelete:
					::strcat(gOutStateStr, "+kRemoteDelete");
				break;
				
				case kRemoteArchiveModify:
					::strcat(gOutStateStr, "+kRemoteArchiveModify");
				break;
				
				case kRemoteArchiveNoModify:
					::strcat(gOutStateStr, "+kRemoteArchiveNoModify");
				break;
				
				case kRemoteNoRecord:
					::strcat(gOutStateStr, "+kRemoteNoRecord");
				break;
				
				case kLocalModify:
					::strcat(gOutStateStr, "+kLocalModify");
				break;
				
/*				case kLocalAdd:
					::strcat(gOutStateStr, "+kLocalAdd");
				break;*/
				
				case kLocalDelete:
					::strcat(gOutStateStr, "+kLocalDelete");
				break;
				
				case kLocalArchiveModify:
					::strcat(gOutStateStr, "+kLocalArchiveModify");
				break;
				
				case kLocalArchiveNoModify:
					::strcat(gOutStateStr, "+kLocalArchiveNoModify");
				break;
				
				case kLocalNoRecord:
					::strcat(gOutStateStr, "+kLocalNoRecord");
				break;
				
				case kRecordDataUnequal:
					::strcat(gOutStateStr, "+kRecordsUnequal");
				break;
				
				case kRecordDataEqual:
					::strcat(gOutStateStr, "+kRecordsEqual");
				break;
			}
		}
	}
	return gOutStateStr;

}
#endif _DEBUG



void CSynchronizer::SynchronizeAppInfoBlock()
{
    CDbGenInfo remoteAppInfo = {};
    CDbGenInfo localAppInfo = {};
	Boolean localInfoExists = false;
	Boolean remoteInfoExists = false;

	ThrowIfNil_(mLocalRecIterator);
	ThrowIfNil_(mPilotIterator);
	ThrowIfNil_(mArchiveIterator);
	
	//If there is no local appinfo an zeroed CDbGenInfo should be returned
    remoteInfoExists = mPilotIterator->GetAppInfo(&remoteAppInfo);
   	localInfoExists = mLocalRecIterator->GetAppInfo(&localAppInfo);
	
	/*blocksEqual = (::memcmp(&remoteAppInfo, &localAppInfo, sizeof(remoteAppInfo)) == 0 &&
				   ::memcmp(remoteAppInfo.m_pBytes, localAppInfo.m_pBytes, localAppInfo.m_BytesRead) == 0);*/
    
    /*if (remoteInfoExists && localInfoExists) // both app infos exist
	{ 
		//if the app info blocks don't exactly match, then we need to take a more protagonistic role in the sync process
        if( !(	remoteAppInfo.m_TotalBytes == localAppInfo.m_TotalBytes &&
				remoteAppInfo.m_BytesRead == localAppInfo.m_BytesRead &&
				remoteAppInfo.m_dwReserved == localAppInfo.m_dwReserved &&
				::strcmp( remoteAppInfo.m_FileName, localAppInfo.m_FileName ) == 0 &&
				::memcmp( remoteAppInfo.m_pBytes, localAppInfo.m_pBytes, remoteAppInfo.m_TotalBytes ) == 0 ) )
		{
        }
    } 
	else if (!localInfoExists)  // no Local app info
	{	
        mLocalRecIterator->WriteAppInfo(remoteAppInfo);// save the remote info to the local storage
    } 
	else if (!remoteInfoExists) 
	{
        mPilotIterator->WriteAppInfo(localAppInfo);// save the local info to the remote storage
    }*/
    if (localInfoExists)
	    mPilotIterator->WriteAppInfo(localAppInfo);// save the local info to the remote storage
}



void CSynchronizer::SynchronizeSortInfoBlock()
{
    CDbGenInfo remoteSortInfo = {};
    CDbGenInfo localSortInfo = {};
	Boolean localInfoExists = false;
	Boolean remoteInfoExists = false;
	Boolean blocksEqual = false;

	ThrowIfNil_(mLocalRecIterator);
	ThrowIfNil_(mPilotIterator);
	ThrowIfNil_(mArchiveIterator);
	
	//If there is no local SortInfo an zeroed CDbGenInfo should be returned
    remoteInfoExists = mPilotIterator->GetSortInfo(&remoteSortInfo);
   	localInfoExists = mLocalRecIterator->GetSortInfo(&localSortInfo);
	
	/*blocksEqual = (::memcmp(&remoteSortInfo, &localSortInfo, sizeof(remoteSortInfo)) == 0 &&
				   ::memcmp(remoteSortInfo.m_pBytes, localSortInfo.m_pBytes, localSortInfo.m_BytesRead) == 0);*/
    
	blocksEqual =	remoteSortInfo.m_TotalBytes == localSortInfo.m_TotalBytes &&
					remoteSortInfo.m_BytesRead == localSortInfo.m_BytesRead &&
					remoteSortInfo.m_dwReserved == localSortInfo.m_dwReserved &&
					::strcmp( remoteSortInfo.m_FileName, localSortInfo.m_FileName ) == 0;
	if( blocksEqual )
		blocksEqual = ::memcmp( remoteSortInfo.m_pBytes, localSortInfo.m_pBytes, remoteSortInfo.m_TotalBytes ) == 0;

    if (remoteInfoExists && localInfoExists) // both app info exist
	{ 
        if (!blocksEqual)
		{
            // THIRDPARTY TODO must replace with real sync code here
            // must handle the categories here
        }
    } 
	else if (!localInfoExists)  // no Local app info
	{	
        mLocalRecIterator->WriteSortInfo(remoteSortInfo);// save the remote info to the local storage
    } 
	else if (!remoteInfoExists) 
	{
        mPilotIterator->WriteSortInfo(localSortInfo);// save the local info to the remote storage
    }
}


void CSynchronizer::DoFastSync()
{
	this->SynchronizeCategories();

	//Deal with the db headers first
	this->SynchronizeAppInfoBlock();
   
    this->SynchronizeSortInfoBlock();
 
	//Walk through the remote records first...
	CSyncRecord remoteRecord;
	CSyncRecord localRecord;

	DebugOutput::Output( "--Pilot Pass--" );
	while (mPilotIterator->NextRecord(eFast, remoteRecord)) //allocates
	{
		Assert_(remoteRecord.RequiredFieldsExist());
	
		UInt32 remoteRecID = remoteRecord.GetRemoteRecordID();
		
		//grab matching local record if there is one
		Boolean foundLocalRec = mLocalRecIterator->GetRecordByID(remoteRecID, eFast, localRecord); //allocates
		Assert_(!foundLocalRec || localRecord.RequiredFieldsExist());	//either it wasn't found - or it's filled out correctly
		
		UInt32 actionsToPerform = this->GetSyncActions(remoteRecord, localRecord );
		
		this->PerformSyncActions(actionsToPerform, remoteRecord, localRecord);
		
		remoteRecord.RemoveFields();
		localRecord.RemoveFields();
	}
	
	//...then walk through the local records
	DebugOutput::Output( "--Desktop Pass--" );
	while (mLocalRecIterator->NextRecord(eFast, localRecord)) //allocates
	{
		Assert_(localRecord.RequiredFieldsExist());

		UInt32 remoteRecID = localRecord.GetRemoteRecordID();
		
		//grab matching remote record if there is one
		Boolean foundRemoteRec = mPilotIterator->GetRecordByID(remoteRecID, eFast, remoteRecord); //allocates
		Assert_(!foundRemoteRec || remoteRecord.RequiredFieldsExist());	//either it wasn't found - or it's filled out correctly
		
		UInt32 actionsToPerform = this->GetSyncActions(remoteRecord, localRecord );
		
		this->PerformSyncActions(actionsToPerform, remoteRecord, localRecord);
		
		remoteRecord.RemoveFields();
		localRecord.RemoveFields();
	}
}


UInt32 CSynchronizer::GetSyncActions(const CSyncRecord& inRemoteRecord, const CSyncRecord& inLocalRecord )
{
	UInt32 changes = 0;
	UInt32 actionsToTake = 0;
	Boolean foundAction = false;

	if (!inRemoteRecord.RequiredFieldsExist())
	{
		changes |= kRemoteNoRecord;
	}
	else
	{
		if (inRemoteRecord.IsArchived() && inRemoteRecord.IsModified() )
			changes |= kRemoteArchiveModify;
		if (inRemoteRecord.IsArchived())
			changes |= kRemoteArchiveNoModify;
		else if (inRemoteRecord.IsDeleted())
			changes |= kRemoteDelete;
/*		else if (inRemoteRecord.IsPrivate())
			changes |= kRemoteAdd;*/
		else if ( inRemoteRecord.IsModified() )
			changes |= kRemoteModify;
		else
			changes |= kRemoteNoChange;
	}
	
	if (!inLocalRecord.RequiredFieldsExist())
	{
		changes |= kLocalNoRecord;
	}
	else
	{
		if (inLocalRecord.IsArchived() && inRemoteRecord.IsModified() )
			changes |= kLocalArchiveModify;
		else if (inLocalRecord.IsArchived())
			changes |= kLocalArchiveNoModify;
		else if (inLocalRecord.IsDeleted())
			changes |= kLocalDelete;
/*		else if (inLocalRecord.IsPrivate())
			changes |= kLocalAdd;*/
		else if ( inLocalRecord.IsModified() )
			changes |= kLocalModify;
		else
			changes |= kLocalNoChange;
	}
	
	if (inRemoteRecord.RequiredFieldsExist() &&	
		inLocalRecord.RequiredFieldsExist()	&&
		inRemoteRecord.RecordDataEqualTo(inLocalRecord) &&
		inLocalRecord.RecordDataEqualTo(inRemoteRecord))
	{
		changes |= kRecordDataEqual; //Both records exist and they're equal
	}
	else
	{
		changes |= kRecordDataUnequal; //Either both records don't exist or they're unequal
	}


	for (short i=0; i < kNumberofStates; i++)
	{
		if (changes == kStateToActionMap[i].state)
		{
			foundAction = true;
		
			actionsToTake = kStateToActionMap[i].actions;
			break;
		}
	}
	//something's gone haywire...
#ifdef _DEBUG
	if (!foundAction || actionsToTake == kSyncError)
	{
		if (!foundAction)
		{
			DebugOutput::Output( "NO ACTION FOUND FOR RECORDS: StateFlags == " );
			DebugOutput::Output( this->SyncStateToString(changes) );
			DebugOutput::Output( "" );
		}
							
		DebugOutput::Output("	Remote Record =");
		mPilotIterator->DebugStreamRecord(inRemoteRecord);
		DebugOutput::Output( "" );
		
		DebugOutput::Output("	Local Record =");
		mLocalRecIterator->DebugStreamRecord(inLocalRecord);
		DebugOutput::Output( "" );
	}

	DebugOutput::Output( "" );

#endif	//_DEBUG



	return actionsToTake;
}



void CSynchronizer::PerformSyncActions(UInt32 inActionsMap, const CSyncRecord& inRemoteRecord, const CSyncRecord& inLocalRecord)
{
	CSyncRecord recordWithCategories;
	CSyncRecord updatedRecord;
	DebugOutput::Output("PerformSyncActions: ");
	
	
	if (inActionsMap == kSyncError )
	{
		DebugOutput::Output("	kSyncError" );
	}
		
	for (UInt32 i = 0; i < kActionsCount; i++)
	{
		switch (inActionsMap & (1 << i))
		{
			case kActionRemoteDelete:
				DebugOutput::Output("	kActionRemoteDelete");
				Assert_(inRemoteRecord.RequiredFieldsExist());
				mPilotIterator->DeleteRecord(inRemoteRecord);
			break;
			
			case kActionRemoteArchive:
				DebugOutput::Output("	kActionRemoteArchive");
				Assert_(inRemoteRecord.RequiredFieldsExist());
				mArchiveIterator->WriteRecord(inRemoteRecord);
				mPilotIterator->DeleteRecord(inLocalRecord);
			break;
			
			case kActionRemoteReplaceLocal:
				DebugOutput::Output("	kActionRemoteReplaceLocal");
				Assert_(inRemoteRecord.RequiredFieldsExist());
				Assert_(inRemoteRecord.GetRemoteRecordID() != 0);
				
				recordWithCategories = inRemoteRecord;
				this->MapCategoriesOntoRecord( recordWithCategories, kPilotToLocal );
				mLocalRecIterator->WriteRecord( recordWithCategories );
				if (mLocalRecIterator->GetRecordByID(inRemoteRecord.GetRemoteRecordID(), eSlow, updatedRecord))
					mPilotIterator->WriteRecord(updatedRecord);
			break;
			
			case kActionRemoteAddToLocal:
				DebugOutput::Output("	kActionRemoteAddToLocal");
				Assert_(inRemoteRecord.RequiredFieldsExist());
				Assert_(inRemoteRecord.GetRemoteRecordID() != 0);
				
				recordWithCategories = inRemoteRecord;
				this->MapCategoriesOntoRecord( recordWithCategories, kPilotToLocal );
				mLocalRecIterator->AddRecord(recordWithCategories);
				mLocalRecIterator->GetRecordByID(inRemoteRecord.GetRemoteRecordID(), eSlow, updatedRecord);
				mPilotIterator->WriteRecord(updatedRecord);
			break;
			
			case kActionLocalDelete:
				DebugOutput::Output("	kActionLocalDelete");
				Assert_(inLocalRecord.RequiredFieldsExist());
				mLocalRecIterator->DeleteRecord(inLocalRecord);
			break;
			
			case kActionLocalArchive:
				DebugOutput::Output("	kActionLocalArchive");
				Assert_(inLocalRecord.RequiredFieldsExist());
				mArchiveIterator->WriteRecord(inLocalRecord);
				mLocalRecIterator->DeleteRecord(inLocalRecord);
			break;
			
			case kActionLocalReplaceRemote:
				DebugOutput::Output("	kActionLocalReplaceRemote");
				Assert_(inLocalRecord.RequiredFieldsExist());
				
				//mLocalRecIterator->WriteRecord(inLocalRecord);
				recordWithCategories = inLocalRecord;
				this->MapCategoriesOntoRecord( recordWithCategories, kLocalToPilot );
				mPilotIterator->WriteRecord(recordWithCategories);
			break;
			
			case kActionLocalAddToRemote:
				DebugOutput::Output("	kActionLocalAddToRemote");

				//cast const away (whoops)
				((CSyncRecord&)inLocalRecord).SetRemoteRecordID(0);
				recordWithCategories = inLocalRecord;
				this->MapCategoriesOntoRecord( recordWithCategories, kLocalToPilot );
				UInt32 pilotRecID = mPilotIterator->WriteRecord(recordWithCategories);
				//cast const away (whoops)
				mLocalRecIterator->UpdateRecordID((CSyncRecord&)recordWithCategories, pilotRecID);
			break;
			
			case kActionNone:
				DebugOutput::Output("	kActionNone");
			break;
			
			case kActionLog:
				DebugOutput::Output("	kActionLog");
			break;
			
			default:
			break;
			
		}
	}

	DebugOutput::Output("---------------------" );
}

void CSynchronizer::GetCatSyncActions()
{
}

void CSynchronizer::DoSlowSync()
{	

	DebugOutput::Output("Performing Slow Sync");
	
	DebugOutput::Output("Synchronize Categories");
	this->SynchronizeCategories();

	//Deal with the db headers first
	DebugOutput::Output("Synchronize AppInfo");
	this->SynchronizeAppInfoBlock();
   
	DebugOutput::Output("Synchronize SortInfo" );
    this->SynchronizeSortInfoBlock();
 
	//Walk through the remote records first...
	CSyncRecord remoteRecord;
	CSyncRecord localRecord;

	DebugOutput::Output( "--Pilot Pass--");
	while (mPilotIterator->NextRecord(eSlow, remoteRecord)) //allocates
	{
		Assert_(remoteRecord.RequiredFieldsExist());
	
		remoteRecord.SetModified(true);
	
		UInt32 remoteRecID = remoteRecord.GetRemoteRecordID();
		
		//grab matching local record if there is one
		Boolean foundLocalRec = mLocalRecIterator->GetRecordByID(remoteRecID, eSlow, localRecord); //allocates
		Assert_(!foundLocalRec || localRecord.RequiredFieldsExist());	//either it wasn't found - or it's filled out correctly
		
		if (foundLocalRec)
		{
			localRecord.SetModified(true);
		}
		
		UInt32 pilotActionsToPerform = this->GetSyncActions(remoteRecord, localRecord );
		
		this->PerformSyncActions(pilotActionsToPerform, remoteRecord, localRecord);
		
		//clear out data since so we can reuse these records...
		remoteRecord.RemoveFields();
		localRecord.RemoveFields();
	}
	
/*
	TO DO:
	We only want to walk over the records that were not dealt with duringthe Pilot enumeration.
	Alos, there's that archive file we're supposed to be dealing with...
*/
	
	//...then walk through the local records
	DebugOutput::Output("--Desktop Pass--");
	while (mLocalRecIterator->NextRecord(eSlow, localRecord)) //allocates
	{
		Assert_(localRecord.RequiredFieldsExist());

		//localRecord.SetModified(true);

		UInt32 localRecID = localRecord.GetRemoteRecordID();
		
		//grab matching remote record if there is one
		Boolean foundRemoteRec = mPilotIterator->GetRecordByID(localRecID, eSlow, remoteRecord); //allocates
		Assert_(!foundRemoteRec || remoteRecord.RequiredFieldsExist());	//either it wasn't found - or it's filled out correctly
		
		//if the record doesn't exist on the pilot, then we need figure out what actions to
		//perform (probably add it to the pilot) and perform those actions
		if (!foundRemoteRec)
		{
			UInt32 locActionsToPerform = this->GetSyncActions(remoteRecord, localRecord );
			
			this->PerformSyncActions(locActionsToPerform, remoteRecord, localRecord);
		}
		
		remoteRecord.RemoveFields();
		localRecord.RemoveFields();
	}
}



void CSynchronizer::DoReplaceRemoteSync()
{}



void CSynchronizer::DoPreSync()
{
	mPilotIterator->PreSync();
	mLocalRecIterator->PreSync();
	mArchiveIterator->PreSync();
}

void CSynchronizer::Synchronize()
{
	StSyncRegisterConduit registeredConduit;
	try
	{
	//    if (mSyncProperties.m_SyncType > eProfileInstall)
	//	    Throw_(CONDERR_BAD_SYNC_TYPE);

		// Obtain System Information
		ThrowIfErrorC_(::SyncReadSystemInfo(mSystemInfo));

		//automatic SyncRegisterConduit/SyncUnregisterConduit object

	    // in most cases there will be only one, but we have
	    // coded it in case there is more
		for (int remoteDBIndex=0; remoteDBIndex < mSyncProperties.m_nRemoteCount; remoteDBIndex++) 
		{		
			this->MakeIterators(remoteDBIndex);
			
			Assert_(mPilotIterator != nil);
			Assert_(mLocalRecIterator != nil);
			Assert_(mArchiveIterator != nil);
			
			ThrowIfNil_(mPilotIterator);
			ThrowIfNil_(mLocalRecIterator);
			ThrowIfNil_(mArchiveIterator);
			
			//do some preprocessing before beginning the sync
			this->DoPreSync();

		    switch (mSyncProperties.m_SyncType)
			{
			    case eFast:
				   this->DoFastSync();
				    break;
			    case eSlow:
				    this->DoSlowSync();
				    break;

			    case ePCtoHH:
				    this->DoReplaceRemoteSync();
				    break;

			    case eHHtoPC:
				    this->DoReplacePCSync();
				    break;

			    case eInstall:
			    case eProfileInstall:
			    case eBackup:
					
					break;
					
			    default: // do nothing
					break;
		    }

			//do any post processing before flushing
			this->DoPostSync(false);

			//flush records back to their respective homes
	 		this->DeleteIterators();
	    }
    }
	catch (const LException& inErr)
	{
		//try to close up shop if we had a problem, only following through if we haven't already tried to
		//post sync once, since if we have,then that means the post sync failed, and it would most likely
		//do so again.
		if( !mTriedPostSync )
			this->DoPostSync(true);

 		this->DeleteIterators();

		//pass the error on
		throw;
	}
}

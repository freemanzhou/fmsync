#include "Utilities.h"
#include "Stringiness.h"
#include "Str255.h"
#include "LErrorMsg.h"
#include "ErrorStrings.h"
#include "ErrorCodes.h"
#include "ConverterErrorStrings.h"
#include "CFileMaker.h"
#ifdef CONDUIT
#include "SyncMgr.h"
#endif

string
ConvertErrorToString(ExceptionCode errorCode)
{
	switch(errorCode) {
	case kDatabaseInTrashError:
		return LoadString(kConverterErrorStrings, kDatabaseInTrashIndex);
		break;
	case kFieldNameTooLongError:
		return LoadString(kConverterErrorStrings, kFieldNameTooLongIndex);
		break;
	case kFieldDataTooLong:
		return LoadString(kConverterErrorStrings, kFieldDataTooLongIndex);
		break;
	case kFieldDataMissingQuote:
		return LoadString(kConverterErrorStrings, kFieldMissingQuoteIndex);
		break;
	case kTooManyFields:
		return LoadString(kConverterErrorStrings, kTooManyFieldsIndex);
		break;
	case kWrongVersion:
		return LoadString(kConverterErrorStrings, kVersionMismatchIndex);
		break;
	case kCantConvertFileError:
		return LoadString(kConverterErrorStrings, kFileCannotBeConvertedIndex);
		break;
	case kNotJTutorTextFileError:
		return LoadString(kConverterErrorStrings, kNotJTutorIndex);
		break;
	case kTooManyRecords:
		return LoadString(kConverterErrorStrings, kTooManyRecordsIndex);
		break;
	case kDatabaseTooLarge:
		return LoadString(kConverterErrorStrings, kDatabaseToLargeIndex);
		break;
	case kRecordDataTooLong:
		return LoadString(kConverterErrorStrings, kRecordTooLargeIndex);
		break;
	case kCantStartExisting:
		return LoadString(kConverterErrorStrings, kCantSyncExistingIndex);
		break;
	case kNotJFileDatabase:
		return LoadString(kConverterErrorStrings, kNonJFileDatabaseIndex);
		break;
	case kPopupChoicesTooLong:
		return LoadString(kConverterErrorStrings, kPopupsTooLargeIndex);
		break;
	case kNoFileMakerErr:
		return LoadString(kConverterErrorStrings, kNoFileMakerIndex);
		break;
	case insufficientStackErr:
		return LoadString(kConverterErrorStrings, kNoStackSpaceIndex);
		break;
	case fnfErr:
		return LoadString(kConverterErrorStrings, kFileCannotBeFoundIndex);
		break;
	case errAEInTransaction:
		return LoadString(kConverterErrorStrings, kInTransactionIndex);
		break;
	case memFullErr:
		return LoadString(kConverterErrorStrings, kOutOfMemoryIndex);
		break;
#ifdef CONDUIT
#ifdef DEMO
	case kDemoLimitExceeded:
		return LoadString(kFMJErrorStrings, kDemoLimitExceededIndex);
		break;
#endif
	case kLockFileInUse:
		return LoadString(kFMJErrorStrings, kLockFileInUseIndex);
		break;
	case SYNCERR_LOCAL_CANCEL_SYNC:
		return LoadString(kConverterErrorStrings, kLocalCancelIndex);
		break;
	case SYNCERR_LOST_CONNECTION:
		return LoadString(kConverterErrorStrings, kLostConnectionIndex);
		break;
	case SYNCERR_REMOTE_CANCEL_SYNC:
		return LoadString(kConverterErrorStrings, kRemoteCancelIndex);
		break;
	case SYNCERR_TOO_MANY_OPEN_FILES:
		return LoadString(kConverterErrorStrings, kRemoteTooManyFiles);
		break;
	case SYNCERR_REMOTE_MEM:
	case SYNCERR_REMOTE_NO_SPACE:
		return LoadString(kConverterErrorStrings, kRemoteOutOfMemory);
		break;
#endif
	case kCannotFindFileMakerErr:
		return LoadString(kConverterErrorStrings, kCannotFindFileMakerIndex);
		break;
	case kNeedsAppleScript:
		return LoadString(kFMJErrorStrings, kNeedsAppleScriptIndex);
		break;
	case kUnknownError:
		return LoadString(kConverterErrorStrings, kUnexpectedErrorIndex);
		break;
	}
	string unknown(LoadString(kFMJErrorStrings, kErrorNumberIndex));
	unknown.append(AsString(errorCode));
	unknown.append(" [0x");
	unknown.append(HexAsString(errorCode));
	unknown.append("])");
	return unknown;
}

bool IsQuietError(ExceptionCode thisError)
{
	switch(thisError) {
	case kSyncUserCanceledErr:
	case kSyncQuietAbortErr:
		return true;
	}
	return false;
}

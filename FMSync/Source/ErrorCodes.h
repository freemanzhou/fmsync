#pragma once

const ExceptionCode kNeedSys8Code = 'Sys8';
const ExceptionCode kRunInHotSyncMangerCode = 'HotS';
const ExceptionCode kNeedAppearanceCode = 'Appe';
const ExceptionCode kSyncQuietAbortErr = 'QErr';
const ExceptionCode kSyncUserCanceledErr = userCanceledErr;
const ExceptionCode kNoFileMakerErr = 'FMP3';
const ExceptionCode kUnknownError = 'UNKN';
const ExceptionCode kLockFileInUse = 'Lock';
const ExceptionCode kDemoLimitExceeded = 'zExC';
const ExceptionCode kNotAJFileDatabase = 'zExC';
const ExceptionCode kDatabaseInTrashError = 'dItr';
const ExceptionCode kCantOverwriteFolderError = 'fold';
const ExceptionCode kNeedsAppleScript = 'NAsc';
const ExceptionCode kCannotFindFileMakerErr = 'CFFM';
const ExceptionCode kModifiedDuringSyncError = 'ModS';
const ExceptionCode kSettingsFileDamaged = 'SetD';

bool IsQuietError(ExceptionCode thisError);
string ConvertErrorToString(ExceptionCode errorCode);

#pragma once

typedef enum {
	kFieldNameTooLongError = 'FNTL',
	kFieldDataTooLong = 'FDTL',
	kFieldDataMissingQuote = 'FDMQ',
	kTooManyFields = 'TMFD',
	kWrongVersion = 'WVer',
	kCantConvertFileError = 'CCFE',
	kNotJTutorTextFileError = 'NJTF',
	kTooManyRecords = 'TMRE',
	kDatabaseTooLarge = 'DTLE',
	kRecordDataTooLong = 'RDTL',
	kPopupChoicesTooLong = 'PCTL',
	kCantStartExisting = 'EXIS',
	kNotJFileDatabase = 'NJFL'
} ConverterError;
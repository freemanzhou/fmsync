#include <types.r>
#include "ErrorStrings.h"

resource 'STR#' (kFMJErrorStrings, purgeable)
{
{
/* 1 */		"The selected layout cannot be used.";		
/* 2 */		"Layouts containing container fields cannot be used to define a remote database.";		
/* 3 */		"Layout has too many fields.";		
/* 4 */		"The selected layout has %%1 fields. The remote database cannot have more than %%2 fields.";		
/* 5 */		"Name already in use.";		
/* 6 */		"The name “%%1” is already in use for a remote database. Please choose a different name.";		
/* 7 */		"The selected FileMaker database cannot be used.";		
/* 8 */		"All layouts contain container fields or too many fields and such layouts cannot be used to define a remote database.";		
/* 9 */		"The previously selected pre-sync script no longer exists.";		
/*10 */		"The previously selected post-sync script no longer exists.";		
/*11 */		"The previously selected layout no longer exists.";		
/*12 */		"Remote database name cannot be blank.";		
/*13 */		"The name of the remote database must have at least one character.";		
/*14 */		"Cannot find FileMaker database.";		
/*15 */		"The FileMaker database “%%1” can no longer be found. Press OK to locate it, or cancel to do nothing.";
/*16 */		"FMSync for JFile 5 needs System 7.5.5 or later.";
/*17 */		"an error occurred (Error number ";
/*18 */		"An unhandled error has occurred.";
/*19 */		"Multiuser databases cannot be used to HotSync.";
/*20 */		"FMSync for JFile 5 needs the Appearance Manager to run.";
/*21 */		"This error occurred while trying to ";
/*22 */		"Remote record ID: ";
/*23 */		"Remote record data follows: ";
/*24 */		"Local record ID: ";
/*25 */		"Local record data follows: ";
/*26 */		"This error occurred while setting the FileMaker field named “";
/*27 */		"FileMaker responded with “";
/*28 */		"Are you sure you want to cancel during synchronization?";
/*29 */		"Canceling at this point may cause you to lose data.";
/*30 */		"A needed field no longer exists in the FileMaker database so modified data will be written to the log.";
/*31 */		"Could not complete the last command because ";
/*32 */		"You cannot change the name of the JFile database after synchronizing.";
/*33 */		"The originally selected name will be used instead.";
/*34 */		"An error occurred while trying to write the settings file.";
/*35 */		"the FMSync for JFile 5 settings window is still open.";
/*36 */		"the demo version of FMSync for JFile 5 will only work with databases of 25 records or less.";
/*37 */		"Make sure the matching version FileMaker is installed. If it is, rebuilding the desktop database may make this problem go away.";
/*38 */		"Could not open the FileMaker database named “%%1” (file type '%%2') because of error %%3.";
/*39 */		"AppleScript is required and does not appear to be available. Check that “AppleScript” and “AppleScriptLib” are in the Exensions folder.";
/*40 */		"The FileMaker database named “%%1” is not open.";
/*41 */		"Make sure to open a database before trying to edit the settings for that database.";
/*42 */		"The selected file does not appear to be a valid settings file.";
/*43 */		"Could not set the user name field because the field selected to receive the handheld user name no longer exists.";
/*44 */		"User name field no longer exists.";
/*45 */		"The user name option will be disabled.";
/*46 */		"The last progress string displayed was “";
/*47 */		"”";
/*48 */		"kNoDatabasesOpenIndex";
/*49 */		"kNoDatabasesOpenIndex2";
/*50 */		"A JFile calculation can have only one value, the other must be a field.";		
}
};

resource 'STR#' (kConverterErrorStrings, purgeable)
{
{
	"HotSync failed for file “";		
	"” because ";		
	"a field name is too long.";		
	"the data in a field is too long.";		
	"the data in a field is missing a quote.";		
	"there are too many fields.";		
	"there is a version mismatch.";		
	"the file cannot be converted.";		
	"the file is not a JTutor file.";		
	"there are too many records.";		
	"the resulting database would be too large.";		
	"a record would exceed the maximum size.";		
	"the pop-up choices would exceed the maximum size.";		
	"the file cannot be found.";		
	"FMSync for JFile 5 cannot start synchronizing with an existing JFile database. Look above in the log for the changed records in tab-delimited format.";		
	"FileMaker is currently busy.";		
	"HotSync succeeded for file “%%1”.";		
	"FileMaker does not appear to be open. Please open FileMaker before using FMSync for JFile 5.";		
	"there is not enough free stack space.";
	"there is not enough unused memory.";
	"an unexpected error occurred.";
	"HotSync was canceled by the user.";
	"HotSync was canceled by the user.";
	"the connection to the organizer was lost.";
	"the organizer does not have enough unused memory.";
	"the organizer has too many files open.";
	"HotSync failed because ";		
	"Upload failed for JFile database “";		
	"” because ";		
	"there is an existing non-JFile Palm database with the same name. Please choose a new name for the JFile database and synchronize again.";		
	"the specified database is in the trash.";		
	"a copy the appropriate version of FileMaker cannot be found on your disk.";		
	"Used the name “%%2” for the JFile database, since the desired name “%%1” was already in use for a Palm database.";		
	"FMSync cannot be configured from Palm Desktop. Please run HotSync Manager from the Finder to configure FMSync. You'll have to ask Palm why this is necessary.";		
}
};

resource 'STR#' (kTaskDescriptionStrings, purgeable)
{
{
	"process a task.";
	"do nothing.";
	"transfer a new record from FileMaker to JFile.";
	"transfer a new record from JFile to FileMaker.";
	"delete a record in FileMaker.";
	"delete a record in JFile.";
	"transfer a modified record from FileMaker to JFile.";
	"transfer a modified record from JFile to FileMaker.";
	"transfer a modified record from JFile to FileMaker.";
	"merge changes and update FileMaker and JFile.";
	"resolve a conflict.";
	"resolve a conflict.";
	"merge changes and update FileMaker and JFile.";
	"duplicate records in order to keep conflicting changes.";
}
};
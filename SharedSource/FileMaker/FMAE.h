#pragma once

#ifndef _FMAE_
#define _FMAE_

#include "OpaqueInteger.h"

namespace FileMakerAccess
{

string SimpleFieldName(const string&);

class FieldID {
public:
				FieldID();
				explicit FieldID(int fieldID);
				explicit FieldID(int fieldID, int relationship);
				
	bool GetValue(int& fieldID, int& relationship) const;
	bool IsRelated() const {return fHasRelationship;}
	bool IsValid() const;
	
	static void Write(LStream&, const FieldID&);
	static void Read(LStream&, FieldID&);

	// not defined a full set, < defined for map et al., == defined for == of vectors
	friend bool operator < (const FieldID&, const FieldID&); 
	friend bool operator == (const FieldID&, const FieldID&);

	static vector<FieldID> ExtractFromDescriptor(AEDesc*);
	void	MakeDescriptor(AEDesc*);

private:
	int fFieldID;
	int fRelationship;
	bool fHasRelationship;
};

//class FieldSecret;
//typedef OpaqueInteger<FieldSecret> FieldID;

typedef long RecordID;

class ScriptIDSecret;
typedef OpaqueInteger<ScriptIDSecret> ScriptID;

typedef long LayoutID;

void		GetDocumentDescriptor(ConstStr255Param docName, AEDesc* desc);
void 		GetDocumentDescriptor(int index, AEDesc* desc);
void		GetDatabaseDescriptor(ConstStr255Param docName, AEDesc* desc);
void		GetDefaultLayoutDescriptor(AEDesc* desc, const AEDesc* docDesc);
void		GetLayoutDescriptor(ConstStr255Param layoutName, AEDesc* desc, const AEDesc* docDesc);
void		GetLayoutIDDescriptor(LayoutID layoutID, AEDesc* desc, const AEDesc* docDesc);
void		GetRecordDescriptor(long recordNum, AEDesc* desc, const AEDesc* layoutDesc);
void		GetRecordIDDescriptor(RecordID recordID, AEDesc* desc, const AEDesc* layoutDesc);
void		GetRecordRangeDescriptor(long firstRecordIndex, long lastRecordIndex, AEDesc* desc, const AEDesc* layoutDesc);
void		GetMenuItemDescriptor(long menuItem, AEDesc* desc, const AEDesc* menuDesc);
void		GetMenuIDDescriptor(long menuID, AEDesc* desc);
void		GetAllRecordsDescriptor(AEDesc* desc, const AEDesc* layoutDesc);
void		GetAllRecordIDsDescriptor(AEDesc* desc, const AEDesc* layoutDesc);
void		GetAllFieldsDescriptor(AEDesc* desc, const AEDesc* layoutDesc);
void		GetAllLayoutsDescriptor(AEDesc* desc, const AEDesc* layoutDesc);
void		GetAllScriptsDescriptor(AEDesc* desc, const AEDesc* layoutDesc);
void		GetFieldDescriptor(ConstStr255Param fieldName, AEDesc* desc, const AEDesc* recordDesc);
void		GetFieldDescriptor(long fieldNumber, AEDesc* desc, const AEDesc* recordDesc);
void		GetFieldIDDescriptor(FileMakerAccess::FieldID fieldID, RecordID recordID, AEDesc* desc, const AEDesc* layoutDesc);
void		GetFieldIDDescriptor(FileMakerAccess::FieldID fieldID, AEDesc* desc, const AEDesc* layoutDesc);
void		GetFieldIDPropertyDescriptor(vector<FileMakerAccess::FieldID>& fieldIDs, FourCharCode prop, AEDesc* desc, const AEDesc* layoutDesc);
void		GetScriptDescriptor(ConstStr255Param scriptName, AEDesc* desc, const AEDesc* recordDesc);
void		GetScriptDescriptor(ScriptID scriptID, AEDesc* desc, const AEDesc* recordDesc);
void		GetPropertyDescriptor(FourCharCode prop, AEDesc* desc, const AEDesc* fieldDesc);
	
void			NewGetDataAppleEvent(OSType, AppleEvent *appleEvent, AEDesc* desc, long transactionID = kAnyTransactionID);
void			NewSetDataAppleEvent(OSType, AppleEvent *appleEvent, AEDesc* desc, AEDesc* theData);
void			NewDoesExistAppleEvent(OSType, AppleEvent *appleEvent, AEDesc* desc);
void			NewShowAppleEvent(OSType, AppleEvent *appleEvent, AEDesc* desc);
void			NewDoScriptAppleEvent(OSType, AppleEvent *appleEvent, AEDesc* desc);
void			NewDoMenuAppleEvent(OSType, AppleEvent *appleEvent, AEDesc* menuItemDesc);
void			NewCreateNewAppleEvent(OSType, AppleEvent *appleEvent, AEDesc* thingToCreate, AEDesc* theData);
void			NewCreateNewAppleEvent(OSType, AppleEvent *appleEvent, AEDesc* thingToCreate);
void			NewDeleteAppleEvent(OSType, AppleEvent *appleEvent, AEDesc* thingToDelete);
void			NewCountAppleEvent(OSType creator, AppleEvent *appleEvent, AEDesc* container, AEDesc* thingToCount);
void			NewGoToAppleEvent(OSType creator, AppleEvent *appleEvent, AEDesc* thingToGoTo);

extern OSType gFileMakerCreator;

}

namespace FMAE = FileMakerAccess;

namespace DebugOutput {
	void DoOutput(const FMAE::FieldID& item);
}
#endif
#include "FMAE.h"
#include "CAEDescriptor.h"
#include "BinaryFormat.h"
#include "DebugOutput.h"
#include "Utilities.h"

namespace {

void PutInteger(AEDesc* descriptor, int index, int integer)
{
	ThrowIfOSErr_(AEPutPtr(descriptor, index, typeInteger, &integer, sizeof(integer)));
}

void PutIntegerPair(AEDesc* descriptor, int index, int i, int j)
{
	CAEDescriptor pairDesc;
	ThrowIfOSErr_(AECreateList(0, 0, false, pairDesc));
	PutInteger(pairDesc, 1, i);
	PutInteger(pairDesc, 2, j);
	ThrowIfOSErr_(AEPutDesc(descriptor, index, pairDesc));
}

void PutID(AEDesc* descriptor, int index, FileMakerAccess::FieldID fieldID)
{
	CAEDescriptor fieldDesc;
	fieldID.MakeDescriptor(fieldDesc);
	ThrowIfOSErr_(AEPutDesc(descriptor, index, fieldDesc));
}

map<OSType, CAEDescriptor> gDescriptors;

CAEDescriptor
TargetAddress(OSType creator)
{
	map<OSType, CAEDescriptor>::iterator f = gDescriptors.find(creator);
	if (f == gDescriptors.end()) {
		CAEDescriptor addr;
		ThrowIfOSErr_(AECreateDesc(typeApplSignature, &creator, sizeof(creator), addr));
		return addr;
	}
	return f->second;
}

}

namespace FileMakerAccess {

string
SimpleFieldName(const string& fn)
{
	string basicName(fn);
	int colonPos = basicName.find_last_of(':');
	if (colonPos != string::npos) {
		basicName.erase(0, colonPos+1);
	}
	return basicName;
}

FieldID::FieldID()
	: fHasRelationship(false), fFieldID(0), fRelationship(0xDEADBEEF)
{
}

FieldID::FieldID(int fieldID)
	: fHasRelationship(false), fFieldID(fieldID), fRelationship(0xDEADBEEF)
{
}

FieldID::FieldID(int fieldID, int relationship)
	: fHasRelationship(true), fRelationship(relationship), fFieldID(fieldID)
{
}

bool FieldID::GetValue(int& fieldID, int& relationship) const
{
	fieldID = fFieldID;
	relationship = fRelationship;
	return fHasRelationship;
}

bool operator < (const FieldID& a, const FieldID& b)
{
	if (a.fHasRelationship < b.fHasRelationship)
		return true;

	if (a.fHasRelationship > b.fHasRelationship)
		return false;
	
	if (a.fHasRelationship) {
		if (a.fRelationship < b.fRelationship)
			return true;

		if (a.fRelationship > b.fRelationship)
			return false;
	}
	
	return a.fFieldID < b.fFieldID;
}

vector<FieldID>
FieldID::ExtractFromDescriptor(AEDesc* theDesc)
{
	vector<FieldID> list;

	long listCount = CountItems(*theDesc);
	for (int i = 1; i <= listCount; i += 1) {
		AEKeyword keyWord;
		CAEDescriptor listItem;
		ThrowIfOSErr_(AEGetNthDesc(theDesc, i, typeWildCard, &keyWord, listItem));
		long itemListCount = listItem.Count();
		DescType typeCode;
		long actualSize;
		if (itemListCount > 1) {
			long field;
			long relationship;
			ThrowIfOSErr_(AEGetNthPtr(listItem, 1, typeLongInteger, &keyWord, &typeCode, &relationship, sizeof(relationship), &actualSize));
			ThrowIfOSErr_(AEGetNthPtr(listItem, 2, typeLongInteger, &keyWord, &typeCode, &field, sizeof(field), &actualSize));
			list.push_back(FileMakerAccess::FieldID(field, relationship));
		} else {
			long field;
			ThrowIfOSErr_(AEGetNthPtr(theDesc, i, typeLongInteger, &keyWord, &typeCode, &field, sizeof(field), &actualSize));
			list.push_back(FileMakerAccess::FieldID(field));
		}
	}
	return list;
}

bool
FieldID::IsValid() const
{
	return fFieldID != 0;
}

bool operator == (const FieldID& a, const FieldID& b)
{
	return a.fFieldID == b.fFieldID && a.fHasRelationship == b.fHasRelationship &&
			a.fRelationship == b.fRelationship;
}

void FieldID::Read(LStream&s, FieldID&f)
{
	BinaryFormat::Read(s, f.fFieldID);
	BinaryFormat::Read(s, f.fRelationship);
	int hasRelationship;
	BinaryFormat::Read(s, hasRelationship);
	f.fHasRelationship = hasRelationship;
}

void FieldID::Write(LStream&s, const FieldID&f)
{
	BinaryFormat::Write(s, f.fFieldID);
	BinaryFormat::Write(s, f.fRelationship);
	int hasRelationship = f.fHasRelationship;
	BinaryFormat::Write(s, hasRelationship);
}

void FieldID::MakeDescriptor(AEDesc* desc)
{
	if (fHasRelationship) {
		ThrowIfOSErr_(AECreateList(0, 0, false, desc));
		PutInteger(desc, 1, fRelationship);
		PutInteger(desc, 2, fFieldID);
	} else {
		ThrowIfOSErr_(::AECreateDesc(typeLongInteger, &fFieldID, sizeof(fFieldID),
										desc));
	}
}

OSType gFileMakerCreator = 'FMP3';

void GetDocumentDescriptor(ConstStr255Param docName, AEDesc* desc)
{
	CAEDescriptor	docDesc(docName);
	CAEDescriptor	nullDesc;
	ThrowIfOSErr_(CreateObjSpecifier(cDocument, nullDesc, formName, docDesc, false, desc));
}

void GetDocumentDescriptor(int index, AEDesc* desc)
{
	CAEDescriptor	docDesc((long)index);
	CAEDescriptor	nullDesc;
	ThrowIfOSErr_(CreateObjSpecifier(cDocument, nullDesc, formAbsolutePosition, docDesc, false, desc));
}

const long cDatabase = 'cDB ';

void GetDatabaseDescriptor(ConstStr255Param docName, AEDesc* desc)
{
	CAEDescriptor	docDesc(docName);
	CAEDescriptor	nullDesc;
	ThrowIfOSErr_(CreateObjSpecifier(cDatabase, nullDesc, formName, docDesc, false, desc));
}

void GetLayoutDescriptor(ConstStr255Param layoutName, AEDesc* desc, const AEDesc* docDesc)
{
	CAEDescriptor	layoutDesc(layoutName);
	CAEDescriptor	localDocDesc(docDesc);
	ThrowIfOSErr_(CreateObjSpecifier(cTable, localDocDesc, formName, layoutDesc, false, desc));
}

void GetDefaultLayoutDescriptor(AEDesc* desc, const AEDesc* docDesc)
{
	long 			layoutNumber = 0;
	CAEDescriptor	layoutDesc(layoutNumber);
	CAEDescriptor	localDocDesc(docDesc);
	ThrowIfOSErr_(CreateObjSpecifier(cTable, localDocDesc, formUniqueID, layoutDesc, false, desc));
}

void GetLayoutIDDescriptor(LayoutID layoutID, AEDesc* desc, const AEDesc* docDesc)
{
	CAEDescriptor	layoutDesc(layoutID);
	CAEDescriptor	localDocDesc(docDesc);
	ThrowIfOSErr_(CreateObjSpecifier(cTable, localDocDesc, formUniqueID, layoutDesc, false, desc));
}

void GetRecordDescriptor(long recordNum, AEDesc* desc, const AEDesc* layoutDesc)
{
	CAEDescriptor	recordDesc(recordNum);
	CAEDescriptor	localLayoutDesc(layoutDesc);
	ThrowIfOSErr_(CreateObjSpecifier(cRow, localLayoutDesc, formAbsolutePosition, recordDesc,
								false, desc));
}

void GetRecordIDDescriptor(long recordID, AEDesc* desc, const AEDesc* layoutDesc)
{
	CAEDescriptor	recordDesc(recordID);
	CAEDescriptor	localLayoutDesc(layoutDesc);
	ThrowIfOSErr_(CreateObjSpecifier(cRow, localLayoutDesc, formUniqueID, recordDesc,
								false, desc));
}

void GetRecordRangeDescriptor(long firstRecordIndex, long lastRecordIndex, AEDesc* desc, const AEDesc* layoutDesc)
{
	CAEDescriptor	localLayoutDesc(layoutDesc);
	CAEDescriptor	rangeDesc;
	CAEDescriptor	firstRecordDesc;
	GetRecordDescriptor(firstRecordIndex, firstRecordDesc, localLayoutDesc);
	CAEDescriptor	lastRecordDesc;
	GetRecordDescriptor(lastRecordIndex, lastRecordDesc, localLayoutDesc);
	ThrowIfOSErr_(CreateRangeDescriptor(firstRecordDesc, lastRecordDesc, false, rangeDesc));
	ThrowIfOSErr_(CreateObjSpecifier(cRow, localLayoutDesc, formRange, rangeDesc,
								false, desc));
}

void GetMenuItemDescriptor(long menuItem, AEDesc* desc, const AEDesc* menuDesc)
{
	CAEDescriptor menuItemDesc(menuItem);
	CAEDescriptor	localMenuDesc(menuDesc);
	ThrowIf_(CreateObjSpecifier(cMenuItem, localMenuDesc, formAbsolutePosition, menuItemDesc,
							false, desc));
}

void GetMenuIDDescriptor(long menuID, AEDesc* desc)
{
	CAEDescriptor menuIDDesc(menuID);
	CAEDescriptor	nullDesc;
	ThrowIfOSErr_(CreateObjSpecifier(cMenu, nullDesc, formUniqueID, menuIDDesc,
							false, desc));
}

static void ConstructDescriptor(AEDesc* targetDesc, const AEDesc* container, long itemNum, int itemNumberForm, int itemType)
{
	CAEDescriptor	itemDesc(typeAbsoluteOrdinal, &itemNum, sizeof(itemNum));
	CAEDescriptor	localContainerDesc(container);
	ThrowIfOSErr_(CreateObjSpecifier(itemType, localContainerDesc, itemNumberForm, itemDesc,
								false, targetDesc));
}

void GetAllRecordsDescriptor(AEDesc* desc, const AEDesc* layoutDesc)
{
	ConstructDescriptor(desc, layoutDesc, kAEAll, formAbsolutePosition, cRow);
}

void GetAllFieldsDescriptor(AEDesc* desc, const AEDesc* layoutDesc)
{
	ConstructDescriptor(desc, layoutDesc, kAEAll, formAbsolutePosition, cColumn);
}

void GetAllLayoutsDescriptor(AEDesc* desc, const AEDesc* layoutDesc)
{
	ConstructDescriptor(desc, layoutDesc, kAEAll, formAbsolutePosition, cTable);
}

void GetAllScriptsDescriptor(AEDesc* desc, const AEDesc* layoutDesc)
{
	ConstructDescriptor(desc, layoutDesc, kAEAll, formAbsolutePosition, 'cSCP');
}

void GetFieldDescriptor(ConstStr255Param fieldName, AEDesc* desc, const AEDesc* recordDesc)
{
	CAEDescriptor	fieldNameDesc(fieldName);
	CAEDescriptor	localRecordDesc(recordDesc);
	ThrowIfOSErr_(CreateObjSpecifier(cCell, localRecordDesc, formName, fieldNameDesc, false, 
								desc));
}

void GetScriptDescriptor(ConstStr255Param scriptName, AEDesc* desc, const AEDesc* recordDesc)
{
	CAEDescriptor	scriptNameDesc(scriptName);
	CAEDescriptor	localRecordDesc(recordDesc);
	ThrowIfOSErr_(CreateObjSpecifier('cSCP', localRecordDesc, formName, scriptNameDesc, false, 
								desc));
}

void GetScriptDescriptor(ScriptID scriptID, AEDesc* desc, const AEDesc* recordDesc)
{
	long			i(scriptID.GetValue());
	CAEDescriptor	scriptIDDesc(i);
	CAEDescriptor	localRecordDesc(recordDesc);
	ThrowIfOSErr_(CreateObjSpecifier('cSCP', localRecordDesc, formUniqueID, scriptIDDesc, false, 
								desc));
}

void GetFieldDescriptor(long fieldNumber, AEDesc* desc, const AEDesc* recordDesc)
{
	CAEDescriptor	fieldNumberDesc(fieldNumber);
	CAEDescriptor	localRecordDesc(recordDesc);
	ThrowIfOSErr_(CreateObjSpecifier(cCell, localRecordDesc, formAbsolutePosition, fieldNumberDesc, false, 
								desc));
}

void GetFieldIDDescriptor(FileMakerAccess::FieldID fieldID, AEDesc* desc, const AEDesc* layoutDesc)
{
	CAEDescriptor	fieldIDDesc;
	fieldID.MakeDescriptor(fieldIDDesc);
	CAEDescriptor	localLayoutDesc(layoutDesc);
	
	ThrowIfOSErr_(CreateObjSpecifier(cCell, localLayoutDesc, formUniqueID, fieldIDDesc, false, 
							desc));
}

void GetFieldIDDescriptor(FileMakerAccess::FieldID fieldID, long recordID, AEDesc* desc, const AEDesc* layoutDesc)
{
	CAEDescriptor	fieldIDDesc;
	fieldID.MakeDescriptor(fieldIDDesc);
	CAEDescriptor	localLayoutDesc(layoutDesc);
	
	ThrowIfOSErr_(AECreateList(0, 0, false, fieldIDDesc));
	
	//int recordIDAsInteger = recordID.GetValue();
	ThrowIfOSErr_(AEPutPtr(fieldIDDesc, 1, typeLongInteger, &recordID, sizeof(recordID)));
	PutID(fieldIDDesc, 2, fieldID);
	ThrowIfOSErr_(CreateObjSpecifier(cCell, localLayoutDesc, formUniqueID, fieldIDDesc, false, 
							desc));
}

void GetFieldIDPropertyDescriptor(vector<FileMakerAccess::FieldID>& fieldIDs, FourCharCode prop, AEDesc* desc, const AEDesc* layoutDesc)
{
	ThrowIfOSErr_(AECreateList(0, 0, false, desc));
	int index = 1;
	vector<FileMakerAccess::FieldID>::const_iterator i = fieldIDs.begin();
	while (i != fieldIDs.end()) {
		CAEDescriptor idDesc;
		CAEDescriptor propDesc;
		GetFieldIDDescriptor(*i, idDesc, layoutDesc);
		GetPropertyDescriptor(prop, propDesc, idDesc);
		ThrowIfOSErr_(AEPutDesc(desc, index, propDesc));
		index += 1;
		i++;
	}
}

void GetPropertyDescriptor(FourCharCode prop, AEDesc* desc, const AEDesc* fieldDesc)
{
	CAEDescriptor	propDesc(typeType, &prop, sizeof(prop));
	CAEDescriptor	localFieldDesc(fieldDesc);
	ThrowIfOSErr_(CreateObjSpecifier(cProperty, localFieldDesc, formPropertyID, propDesc, false, 
								desc));
}
	
void NewGetDataAppleEvent(OSType creator, AppleEvent *appleEvent, AEDesc* desc, long transactionID)
{
	ThrowIfOSErr_(AECreateAppleEvent(kAECoreSuite, kAEGetData, TargetAddress(creator), kAutoGenerateReturnID,
							transactionID, appleEvent));
	ThrowIfOSErr_(AEPutParamDesc(appleEvent, keyDirectObject, desc));
}

void
NewSetDataAppleEvent(OSType creator, AppleEvent *appleEvent, AEDesc* desc, AEDesc *theData)
{
	ThrowIfOSErr_(AECreateAppleEvent(kAECoreSuite, kAESetData, TargetAddress(creator), kAutoGenerateReturnID,
							kAnyTransactionID, appleEvent));
	ThrowIfOSErr_(AEPutParamDesc(appleEvent, keyDirectObject, desc));
	ThrowIfOSErr_(AEPutParamDesc(appleEvent, keyAEData, theData));
}

void
NewCreateNewAppleEvent(OSType creator, AppleEvent *appleEvent, AEDesc* thingToCreate, AEDesc* theData)
{
	ThrowIfOSErr_(AECreateAppleEvent(kAECoreSuite, kAECreateElement, TargetAddress(creator), kAutoGenerateReturnID,
							kAnyTransactionID, appleEvent));
	ThrowIfOSErr_(AEPutParamDesc(appleEvent, keyAEObjectClass, thingToCreate));
	ThrowIfOSErr_(AEPutParamDesc(appleEvent, keyAEData, theData));
}

void
NewCreateNewAppleEvent(OSType creator, AppleEvent *appleEvent, AEDesc* thingToCreate)
{
	ThrowIfOSErr_(AECreateAppleEvent(kAECoreSuite, kAECreateElement, TargetAddress(creator), kAutoGenerateReturnID,
							kAnyTransactionID, appleEvent));
	ThrowIfOSErr_(AEPutParamDesc(appleEvent, keyAEObjectClass, thingToCreate));
}

void
NewDeleteAppleEvent(OSType creator, AppleEvent *appleEvent, AEDesc* thingToDelete)
{
	ThrowIfOSErr_(AECreateAppleEvent(kAECoreSuite, kAEDelete, TargetAddress(creator), kAutoGenerateReturnID,
							kAnyTransactionID, appleEvent));
	ThrowIfOSErr_(AEPutParamDesc(appleEvent, keyDirectObject, thingToDelete));
}

void
NewDoesExistAppleEvent(OSType creator, AppleEvent *appleEvent, AEDesc* thingToCheck)
{
	ThrowIfOSErr_(AECreateAppleEvent(kAECoreSuite, kAEDoObjectsExist, TargetAddress(creator), kAutoGenerateReturnID,
							kAnyTransactionID, appleEvent));
	ThrowIfOSErr_(AEPutParamDesc(appleEvent, keyDirectObject, thingToCheck));
}

void
NewShowAppleEvent(OSType creator, AppleEvent *appleEvent, AEDesc* thingToShow)
{
	ThrowIfOSErr_(AECreateAppleEvent(kAEMiscStandards, kAEMakeObjectsVisible, TargetAddress(creator), kAutoGenerateReturnID,
							kAnyTransactionID, appleEvent));
	ThrowIfOSErr_(AEPutParamDesc(appleEvent, keyDirectObject, thingToShow));
}

void
NewGoToAppleEvent(OSType creator, AppleEvent *appleEvent, AEDesc* thingToGoTo)
{
	ThrowIfOSErr_(AECreateAppleEvent('FMPR', 'GOTO', TargetAddress(creator), kAutoGenerateReturnID,
							kAnyTransactionID, appleEvent));
	ThrowIfOSErr_(AEPutParamDesc(appleEvent, keyDirectObject, thingToGoTo));
}

void
NewCountAppleEvent(OSType creator, AppleEvent *appleEvent, AEDesc* container, AEDesc* thingToCount)
{
	ThrowIfOSErr_(AECreateAppleEvent(kAECoreSuite, kAECountElements, TargetAddress(creator), kAutoGenerateReturnID,
							kAnyTransactionID, appleEvent));
	ThrowIfOSErr_(AEPutParamDesc(appleEvent, keyAEObjectClass, thingToCount));
	ThrowIfOSErr_(AEPutParamDesc(appleEvent, keyDirectObject, container));
}

void NewDoMenuAppleEvent(OSType creator, AppleEvent *appleEvent, AEDesc* menuItemDesc)
{
	ThrowIfOSErr_(AECreateAppleEvent(kAEMiscStandards, 'menu', TargetAddress(creator), kAutoGenerateReturnID,
								kAnyTransactionID, appleEvent));
	ThrowIfOSErr_(AEPutParamDesc(appleEvent, keyDirectObject, menuItemDesc));
}

void NewDoScriptAppleEvent(OSType creator, AppleEvent *appleEvent, AEDesc* desc)
{
	ThrowIfOSErr_(AECreateAppleEvent(kAEMiscStandards, kAEDoScript, TargetAddress(creator), kAutoGenerateReturnID,
								kAnyTransactionID, appleEvent));
	ThrowIfOSErr_(AEPutParamDesc(appleEvent, keyDirectObject, desc));
}

}

namespace DebugOutput {

void DoOutput(const FMAE::FieldID& item)
{
	int fieldID, relationship;
	bool hasR = item.GetValue(fieldID, relationship);
	if (hasR) {
		pair<int,int> a(relationship, fieldID);
		DoOutput(a);
	} else {
		DoOutput(fieldID);
	}
}

}

#pragma once
#include <vector>
#include <string>

#include "CFieldIDList.h"
#include "CAEDescriptor.h"
#include "CDatabaseName.h"
#include "CDatabase.h"
#include "FMAE.h"
#include "CFieldIDList.h"
#include "CFileMaker.h"

class CRecord;
class CReaderProgress;

namespace FileMaker {

class Database {
friend class CFileMaker;
public:
			typedef UInt32 Uint32;
							Database(const FSSpec& databaseSpec, bool useFoundSet = false, 
								CReaderProgress *progress = 0);
							Database(const CDatabaseName& databaseName, bool useFoundSet = false, 
								CReaderProgress *progress = 0);
							~Database();

			void CreateDocumentDescriptor(ConstStringPtr dbName);			
			void CheckForJapaneseSystem();

			void			DoScript(FMAE::ScriptID, bool waitAfter = true);
			void			EnterBrowseMode();
			void			FindAll();
			
			int				MajorVersion();

			CFieldIDList	GetFieldIDs();
			CFieldIDList	GetAllFieldIDs();

			bool			HasRelatedFields();
			bool			HasContainerFields();
			bool			IsMultiUser();
			vector<string>	GetRecordAllFields(int recordID);
			map<FMAE::FieldID, string> GetRecordAllFieldsMap(int recordID);
			int				AddRecord(const CFieldIDList& fieldIDs, const vector<string>& fieldData);
			int				AddRecordComplex(const CFieldIDList& fieldIDs, const vector<string>& fieldData);
			int				AddRecordSimple(const vector<string>& fieldData);
			int				GetRecordID(const CAEDescriptor& recordReference);
			void			WriteRecord(Uint32 recordID, const CFieldIDList& fieldIDs, const vector<string>& fieldData);
			void			WriteRecordSimple(Uint32 recordID, const vector<string>& fieldData);
			void			WriteRecordComplex(Uint32 recordID, const CFieldIDList& fieldIDs, const vector<string>& fieldData);
			vector<string>	GetRecord(int recordID);
			void			ShowRecord(int recordID);
			void			ShowLayout(FMAE::LayoutID layoutID);
			void			DeleteRecord(Uint32 recordID, bool withProgress = true);
			void			UseLayout(int layoutID);
			bool			ScriptExists(FMAE::ScriptID scriptID);
			bool			LayoutExists(int layoutID);
			int				FieldCount();
			FMAE::FieldID	LastFieldWritten();
			map<FMAE::FieldID, string> GetAllFieldsNameMap();
			map<FMAE::FieldID, string> GetFieldNameMap();
			bool			FieldExists(FMAE::FieldID fieldID);
			bool			FieldReadOnly(FMAE::FieldID fieldID);
			vector<string>	GetFieldNames();
			vector<int>		GetRecordIDs();
			vector<int>		GetAllRecordIDs();
			vector<int>		GetFieldWidths();
			vector<int>		GetFieldReadOnly();
			vector<int>		GetAllFieldReadOnly();
			vector<string>	GetFields();
			vector<int>		GetFieldTypes();
			vector<string>	GetAllFieldNames();
			vector<int>		GetAllFieldTypes();
			vector<string_vector>		GetFieldChoices();
			vector<string_vector>		GetAllFieldChoices();
			vector<string>	GetLayoutNames();
			vector<int>		GetLayoutIDs();
			vector<string>	GetScriptNames();
			vector<FMAE::ScriptID>		GetScriptIDs();

			int				AllRecordsCount();

			vector<int>		GetFieldRepeats();
			
			void			LockRecord(UInt32);
			void			UnlockRecord(UInt32);
private:
			void			WriteRecordLow(Uint32 recordID, const vector<string>& fieldData);

			void		SetupDatabaseName(const FSSpec&);

			vector<int>		GetAllFieldRepeats();
			vector<int>		GetAllFieldAccess();

			vector<int>		GetFieldAccess();

			string			GetFieldByID(Uint32 recordID, FMAE::FieldID fieldID);
			bool			DoesExist(AEDesc* thisItem);
			long			Count(AEDesc* thisItem);
			int				FoundRecordsCount();
			bool			RecordExists(int recordID);
			static void		FieldToDescriptor(string theField, int repeatCount, 
								CAEDescriptor& fieldData, bool inJapanese = false);

			void			FieldListToDescriptor(const vector<string>& theFields, CAEDescriptor& fieldListDescriptor, bool inJapanese = false);
template <class T>
static		void			ExtractList(CAEDescriptor& theDesc, vector<T>& theList);
static 		vector<string>	ExtractStringList(CAEDescriptor& theDesc);
static 		vector<int>		ExtractIntList(CAEDescriptor& theDesc);
static 		vector<int>		ExtractEnumList(CAEDescriptor& theDesc);
static 		vector<string_vector> ExtractStringVectorList(CAEDescriptor& theDesc);
static		bool			HasAEListItems(CAEDescriptor& theDesc);

			operator	AEDesc*() { return fDatabaseDesc; }
			operator	AEDesc&() { return fDatabaseDesc; }

			void		DoProgress(const string& progressString);

			vector<string>	GetRecord(int recordID, const AEDesc *layout, const vector<FMAE::FieldID>& fieldIDs);

			bool		HasNonWritableFields();
			void		WriteField(Uint32 recordID, int fieldIndex, const string& fieldData);
			void		WriteFieldByID(Uint32 recordID, FMAE::FieldID fieldID, const string& fieldData);
			
			void		GetFieldsProperties(AEDesc* layoutDesc, OSType property, AEDesc* target);
			
			void		Setup();

			void		LoadLayout0FieldIDs();
			void		LoadFieldIDs();
			void		LoadAllRecordIDs();
			void		LoadRecordIDs();

			void		SetupAllFields();
			void		SetupRepeats();
			void		SetupAccess();
			void		SetupFieldTypes();
			void		SetupNames();
			void		SetupChoices();
			void		SetupFieldExists();
			
			void		UpdateFieldChoices();
			
			void		GetAllFieldsProperty(int property, const AEDesc* layout, AEDesc* props, int fieldCount);
			
			template <class T>
			void 		SetupPropertyMap(int property, 
							map<FMAE::FieldID, T>& map)
				{
					map.clear();
					CAEDescriptor desc;
					GetAllFieldsProperty(property, fLayout0Desc, desc, fLayout0FieldIDs.size());
					vector<T> list;
					Descriptors::ExtractList(desc, list);
					AddToMap(fLayout0FieldIDs, list, map);
					GetAllFieldsProperty(property, fDefaultLayoutDesc, desc, fFieldIDs.size());
					list.clear();
					Descriptors::ExtractList(desc, list);
					AddToMap(fFieldIDs, list, map);
				}

	CDatabaseName			fName;
	CAEDescriptor			fDatabaseDesc;
	CAEDescriptor			fDocumentDesciptor;
	CAEDescriptor			fLayout0Desc;
	CAEDescriptor			fDefaultLayoutDesc;
	CAEDescriptor			fDatabaseLayoutDesc;
	CAEDescriptor			fDocumentLayoutDesc;
	CReaderProgress*		fProgress;
	CFieldIDList			fAllFieldIDs;
	CFieldIDList			fFieldIDs;
	CFieldIDList			fLayout0FieldIDs;
	map<FMAE::FieldID,int>	fRepeats;
	map<FMAE::FieldID,int>	fAccess;
	map<FMAE::FieldID,int>	fFieldTypes;
	map<FMAE::FieldID,bool>	fFieldExists;
	map<FMAE::FieldID,int>	fFieldReadOnly;
	map<FMAE::FieldID,string> fFieldNames;
	map<FMAE::FieldID,string_vector> fFieldChoices;
	vector<int>				fRecordIDs;
	vector<int>				fAllRecordIDs;
	FMAE::FieldID			fLastFieldIDWritten;
	int						fLayoutID;
	UInt32					fLockedRecordID;
	bool					fHasNonWritableFields;
	bool					fCheckedForRelated;
	bool					fHasRelated;
	bool					fCheckedForContainers;
	bool					fHasContainers;
	bool					fUseFoundSet;
	bool					fJapanese;
	bool					fFieldChoicesDirty;
	bool					fRecordIDsDirty;
	bool					fAllRecordIDsDirty;
	bool					fCloseOnDelete;
	bool					fRecordLocked;
};

}
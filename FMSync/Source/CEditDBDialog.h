#pragma once

#include "CConduitDialog.h"
#include "CConduitWarner.h"
#include "CDatabaseFilePtr.h"
#include "CFieldIDList.h"
#include "CFieldOverrides.h"
#include "FMDatabasePtr.h"
#include "FieldTypeBoxHandler.h"
#include "JFile5.h"
#include "OpaqueInteger.h"

class LPopupButton;
class LCheckBox;
class LStaticText;
class LEditText;
class LMultiPanelView;
class CStringTable;
class CWarnings;
class LPopupGroupBox;
class LRadioButton;

class DialogFieldTypeSecret;
typedef OpaqueInteger<DialogFieldTypeSecret> DialogFieldType;


class CFieldAttributes {
public:
	map<FMAE::FieldID,string>	fFieldNames;
	map<FMAE::FieldID,vector<string> > fFieldPopups;
	map<FMAE::FieldID,int>		fFieldTypes;
	map<FMAE::FieldID,int>		fFieldWidths;
	map<FMAE::FieldID,int>		fFieldReadOnly;
	map<FMAE::FieldID,int>		fFieldExtra;
	map<FMAE::FieldID,int>		fFieldExtra2;
	map<FMAE::FieldID,string>	fFieldValue1;
	map<FMAE::FieldID,string>	fFieldValue2;
};

namespace DebugOutput {

void DoOutput(const CFieldAttributes& inAttr);

}

class CEditDBDialog : public CConduitDialog, public CConduitWarner
{
public:
	friend class IntFieldTypeBoxHandler;
	friend class CalcFieldTypeBoxHandler;
	friend class PopupFieldTypeBoxHandler;
	friend class CheckboxFieldTypeBoxHandler;
	
							CEditDBDialog(CDatabaseFilePtr inDB, CWarnings& w);
					virtual ~CEditDBDialog();
	
	virtual void		FinishCreateSelf();
	
	virtual void		ListenToMessage(
							MessageT		inMessage,
							void			*ioParam);

	virtual void	UpdateControlState();

	virtual void	RegisterClasses();


protected:
	virtual bool	DialogDone();

private:
	FMAE::FieldID	GetLastSelectedField();

	void			SetupForDatabase();
	void			SetupWindowTitle();
	void			PrepareDatabase();
	void			PrepareLayouts();
	void			PrepareScripts();
	void			PrepareOptions();
	void			PrepareAdvanced();
	void			PrepareUserNameFieldMenu();

#ifdef NETWORK
	bool			AskForExportFile(FSSpec& outFile);
	void			DoExport();
#endif
	
	void			SetupFieldsPane();
	void			SetupFieldMaps();
	bool			DoSetRemoteDatabaseName();
	bool			RememberLayoutChanges();
	void			SetToDefaults();
	void			RememberChanges();
	void			RememberPreSyncScript();
	void			RememberPostSyncScript();
	void			RememberUserNameField();

	bool			DefaultLayoutID(int *layoutIDP);
	void			HandleSelectedField(int);
	void			HandleNoSelectedField();
	void			HandleSelectedFieldMessage();
	void			HandleFieldTypeChangedMessage();
	void			SaveSelectedFieldInfo();
	void			SetupDefaultFieldMaps();
	
	void			UpdateFieldDisplay(FMAE::FieldID field);
	void			UpdateSchedule();
	
	void			SetupSyncDescription();
	
	void			SetupFieldInfoBox(int fieldType);
	int				ExtractFieldType(int);
	void			EnableAutoIncrementEdit(bool);
	void			UpdateFieldAttributes();
	void			SetupCalculatedField();
	
	void			UseLayout(int layoutID);
	void			SwitchToLayoutUseLayout(int layoutID);
	
	int				SelectedLayoutID();
	
	void			SaveSelectedExtraData();
	CFieldOverrides	MakeOverrides();
	
	void			UnselectField();
	
	void			OpenDatabase();
	void			OpenDatabaseByName();
	void			OpenDatabaseBySpec();
	int				JFileVersionFromMenu(LPopupButton*);
	int				MenuFromJFileVersion(int);

	void			SetupHandlers();
	void			FindPanes();
	void			SetupListeners();
	void			SetupNetworkOnlyFeatures();
	void			SetupJFileFieldTypeMap();
	
	FieldTypeBoxHandler::Ptr GetHandler(DialogFieldType fieldTypeMenuValue);
	void AddHandler(int, FieldTypeBoxHandler*);
	
	DialogFieldType		GetDialogFieldType();
	void				SetDialogFieldType(const DialogFieldType& f);
	
	map<DialogFieldType,FieldTypeBoxHandler::Ptr> fHandlers;
	map<UInt32,DialogFieldType> fJFileFieldTypeMap;
	CDatabaseFilePtr 	fDatabase;
	CWarnings&			fWarnings;
	FileMaker::DatabasePtr		fFMDatabase;
	LMultiPanelView*	fMPV;
	LMultiPanelView*	fFieldTypeMPV;
	LPopupButton*		fLayout;
	LPopupButton*		fPreSyncScript;
	LPopupButton*		fPostSyncScript;
	LPopupButton*		fSyncModeMenu;
	LPopupButton*		fUserNameField;
	LPopupButton*		fJFileVersionMenu;
	LPopupButton*		fSchedule;
	LCheckBox*			fTranslateText;
	LCheckBox*			fUseFoundSet;
	LCheckBox*			fReadOnly;
	LCheckBox*			fDuplicateOnConflict;
	LEditText*			fJFileName;
	LEditText*			fWidthEdit;
	LEditText*			fFieldNameEdit;
	LStaticText*		fSyncModeDescription;
	LStaticText*		fNextSyncTimeDescription;
	LPushButton*		fDefaults;
	LPushButton*		fExportButton;
	LPushButton*		fNextTimeButton;
	LPopupGroupBox*		fFieldTypeBox;
	int					fLastSelected;
	DialogFieldType		fLastFieldTypeMenuValue;
	CStringTable*		fFieldNamesList;
	map<int, int>		fJFileToMenu;
	map<int, int>		fMenuToJFile;
	vector<string>		fLayoutNames;
	vector<int>			fLayoutIDs;
	vector<string>		fScriptNames;
	map<FMAE::FieldID, string>		fCachedFields;
	map<FMAE::FieldID, bool>		fFieldCached;
	vector<int>			 fAllRecordIDs;
	vector<FMAE::ScriptID> fScriptIDs;
	CFieldIDList		fFieldIDs;
	CFieldIDList		fAllFieldIDs;
	CFieldIDList		fGlobalFieldIDs;
	
	CFieldOverrides		fOverrides;
	
	CFieldAttributes	fFieldAttributes;
	CFieldAttributes	fDefaultFieldAttributes;

	map<FMAE::FieldID,bool>		fFieldFMReadOnly;

	string				fRemoteDatabaseName;
	string				fSyncScheduleString;
	bool				fHandlingFieldTypeChange;
	bool				fOverrideSchedule;
};


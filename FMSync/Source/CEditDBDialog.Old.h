#pragma once

#include "CConduitDialog.h"
#include "CConduitWarner.h"
#include "CFieldIDList.h"
#include "CFieldOverrides.h"
#include "CDatabaseFile.h"

class LPopupButton;
class LCheckBox;
class LStaticText;
class CFileMakerDatabase;
class LEditText;
class LMultiPanelView;
class CStringTable;
class CWarnings;
class LPopupGroupBox;
class LRadioButton;

class CFieldAttributes {
public:
	map<FMAE::FieldID,string>	fFieldNames;
	map<FMAE::FieldID,vector<string> > fFieldPopups;
	map<FMAE::FieldID,int>		fFieldTypes;
	map<FMAE::FieldID,int>		fFieldWidths;
	map<FMAE::FieldID,int>		fFieldReadOnly;
	map<FMAE::FieldID,int>		fFieldExtra;
	map<FMAE::FieldID,int>		fFieldExtra2;
};

namespace DebugOutput {

void DoOutput(const CFieldAttributes& inAttr);

}

class CEditDBDialog : public CConduitDialog, public CConduitWarner
{
public:
							CEditDBDialog(CDatabaseFile::Ptr inDB, CWarnings& w);
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
	void			SetupForDatabase();
	void			SetupWindowTitle();
	void			PrepareDatabase();
	void			PrepareLayouts();
	void			PrepareScripts();
	void			PrepareOptions();
	
	void			SetupFieldsPane();
	void			SetupFieldMaps();
	bool			DoSetRemoteDatabaseName();
	bool			RememberLayoutChanges();
	void			DoEditPopupValues();
	void			SetToDefaults();
	void			RememberChanges();
	bool			DefaultLayoutID(int *layoutIDP);
	void			HandleSelectedField();
	void			SaveSelectedFieldInfo();
	void			SetupDefaultFieldMaps();
	
	void			UpdateFieldDisplay(FMAE::FieldID field);
	
	void			SetupSyncDescription();
	
	void			SetupFieldInfoBox(int fieldType);
	int				ExtractFieldType(int);
	void			EnableAutoIncrementEdit(bool);
	void			UpdateFieldAttributes();
	
	void			UseLayout(int layoutID);
	void			SwitchToLayoutUseLayout(int layoutID);
	
	int				SelectedLayoutID();
	
	void			SaveSelectedExtraData(int fieldType);
	FMAE::FieldID	GetLastSelectedField();
	CFieldOverrides	MakeOverrides();
	
	void			UnselectField();
	
	void			AnnounceFailToOpenDatabase(const FSSpec&, ExceptionCode err);
	
	
	map<FMAE::FieldID,int>	MenuToJFile(const map<FMAE::FieldID,int>&);
	map<FMAE::FieldID,int>	JFileToMenu(const map<FMAE::FieldID,int>&);
	
	CDatabaseFile::Ptr	fDatabase;
	CWarnings&			fWarnings;
	CFileMakerDatabase*	fFMDatabase;
	LMultiPanelView*	fMPV;
	LMultiPanelView*	fFieldTypeMPV;
	LView*				fAutoIncView;
	LPopupButton*		fLayout;
	LPopupButton*		fPreSyncScript;
	LPopupButton*		fPostSyncScript;
	LPopupButton*		fSyncModeMenu;
	LCheckBox*			fTranslateText;
	LCheckBox*			fUseFoundSet;
	LCheckBox*			fReadOnly;
	LCheckBox*			fDuplicateOnConflict;
	LEditText*			fJFileName;
	LEditText*			fWidthEdit;
	LEditText*			fFieldNameEdit;
	LEditText*			fIncStartEdit;
	LEditText*			fIncValueEdit;
	LStaticText*		fSyncModeDescription;
	LPushButton*		fEditPopups;
	LPushButton*		fEditPopupsChk;
	LPushButton*		fDefaults;
	LPopupGroupBox*		fFieldTypeBox;
	LRadioButton*		fCreateDateRadio;
	LRadioButton*		fModDateRadio;
	LRadioButton*		fNormalDateRadio;
	LRadioButton*		fCreateTimeRadio;
	LRadioButton*		fModTimeRadio;
	LRadioButton*		fNormalTimeRadio;
	LRadioButton*		fAutoIncRadio;
	LRadioButton*		fNormalIntRadio;
	LRadioButton*		fNormalListRadio;
	LRadioButton*		fMultiListRadio;
	int					fLastSelected;
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
	
	CFieldOverrides		fOverrides;
	
	CFieldAttributes	fFieldAttributes;
	CFieldAttributes	fDefaultFieldAttributes;

	map<FMAE::FieldID,int>		fJFileFieldTypes;
	map<FMAE::FieldID,bool>		fFieldFMReadOnly;

	string				fRemoteDatabaseName;
};


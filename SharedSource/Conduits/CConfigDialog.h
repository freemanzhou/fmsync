#pragma once

#include "CConduitDialog.h"
#include "CUploadRequest.h"
#include "CDatabaseFile.h"

class CSyncPreference;
class LPushButton;
class LPopupButton;
class CStringTable;
class LRadioButton;
class CConduitSettings;
class LTextGroupBox;
class LStaticText;

class CConfigDialog : public CConduitDialog
{
public:
							CConfigDialog(CSyncPreference& inSyncProperties, const FSSpec& settingsSpec);
					virtual ~CConfigDialog();
	
	virtual void	ListenToMessage(
							MessageT		inMessage,
							void			*ioParam);

	virtual void	FinishCreateSelf();
	virtual void	UpdateControlState();
	void			DoCheckRegistration();
	void			AnnounceRegFailed();

private:
	void			UpdateWindow();
	void			UpdateButtonState();
	void			UpdateDefaultAction();
	void			DoRegister();
	void			DoAdd();
	void			DoAddDatabase();
	void			DoCredits();
	void			DoDisableDatabase();
	void			DoRemoveDatabase();
	void			DoEditDatabase();
	void			DoUploadDatabase();
	void			DoSetSyncAction(int syncAction);
	void			DoSetDefaultSyncAction(int syncAction);
	
	pair<bool,bool>	DoChooseMethod(bool chooseLocal);
	void			ComplainAboutNoDatabases();
	void			ComplainAboutDatabaseNotOpen(const string&);
	void			DoAddExistingDatabase();
	void			DoAddDiskDatabase();
	void			HandleLoadSettingsFile(const FSSpec& theSpec);
	void			DoEditLocalDatabase(CDatabaseFile::Ptr);
	void			DoEditNamedDatabase(CDatabaseFile::Ptr);
	void			PrepareAndEditNewDatabase(CDatabaseFile::Ptr f);
	void			AppendUserNameToTextBox(LTextGroupBox*);
	virtual void	PrepareNewDatabaseFile(CDatabaseFile::Ptr);
	void			ComplainAboutMissingFile(CDatabaseFile::Ptr theFile);
	
	CConduitSettings*	fSettings;
	CSyncPreference& fSyncProperties;
	LPushButton*	fDeleteButton;
	LPushButton*	fEditButton;
	LPushButton*	fAddButton;
	LPushButton*	fDisableButton;
	LPushButton*	fBuyButton;
	LRadioButton*	fDoNothing;
	LRadioButton*	fSync;
	LTextGroupBox*	fDatabaseBox;
	LStaticText*	fDefaultAction;
	CStringTable*	fList;
	LPopupButton*	fQuitModeMenu;
	string			fUserName;
	string			fPotentialRegCode;
	vector<CUploadRequest> fReqs;

#ifdef SELF_TESTS
	void			DoTest();
	LPushButton*	fTestButton;
#endif
};


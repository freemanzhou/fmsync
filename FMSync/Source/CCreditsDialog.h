#pragma once

#include "CConduitDialog.h"
#include "CDatabaseFile.h"

class LStaticText;
class CConduitSettings;

class CCreditsDialog : public CConduitDialog
{
public:
							CCreditsDialog(CConduitSettings*);
					virtual ~CCreditsDialog();
	
	virtual void	FinishCreateSelf();

	virtual void	RegisterClasses();

	virtual void	ListenToMessage(
							MessageT		inMessage,
							void			*ioParam);
private:
	void			DoProfile();
	void			OutputProfileForDatabaseFile(std::ostream& profileInfo, CDatabaseFile::Ptr dFile);
	void			UpdateWindow();
	
	LStaticText*	fVersionText;
	LStaticText*	fExpirationText;
	CConduitSettings* fSettings;
};


void DebugStart(StringPtr s);
void DebugStop();
void _DebugReportString(StringPtr s, bool noLineDelim = false);
void _DebugReportValue(unsigned long val, bool noLineDelim = false);


#define DebugReportPosition() \
	{ _DebugReportString("\p" __FILE__, true); _DebugReportValue(__LINE__); }

#define DebugReportPositionPrivate() \
	{ _DebugReportString("\p" __FILE__ ", ", true); _DebugReportValue(__LINE__, true); }

#define DebugReportString(_s) \
	{											\
		DebugReportPositionPrivate();			\
		_DebugReportString("\p: ", true);		\
		_DebugReportString(_s);					\
	}
	
#define DebugReportValue(_v) \
	{											\
		DebugReportPositionPrivate();			\
		_DebugReportString("\p: ", true);		\
		_DebugReportValue(_v);					\
	}

#define DebugReportFSSpec(_f) \
	{											\
		DebugReportPositionPrivate();			\
		_DebugReportString("\p: ", true);		\
		_DebugReportString((StringPtr) (_f).name, true);		\
		_DebugReportString("\p, ", true);		\
		_DebugReportValue((_f).vRefNum, true);	\
		_DebugReportString("\p, ", true);		\
		_DebugReportValue((_f).parID);		\
	}





class StDebug
{
	public:
		StDebug(StringPtr s) { DebugStart(s); }
		~StDebug() { DebugStop(); }
};









#define MacroNum(_3) #_3
#define MacroCat(_1, _2) MacroNum(_1) ## _2
#define ThrowIfErr(_res) if (_res) { \
	throw LException(_res, "\pFile: " __FILE__ ", Line: " MacroCat(__LINE__, "") ); \
}

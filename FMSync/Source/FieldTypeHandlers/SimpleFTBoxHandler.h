#include "FieldTypeBoxHandler.h"

class SimpleFieldTypeBoxHandler : public FieldTypeBoxHandler {
public:
	SimpleFieldTypeBoxHandler(CEditDBDialog*, UInt32 jFileFieldType);
	
	UInt32 GetJFileFieldType();
	void SetupForField(UInt32) {}

private:
	UInt32 fJFileType;
};

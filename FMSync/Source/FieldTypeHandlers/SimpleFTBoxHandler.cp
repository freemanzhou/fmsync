#include "SimpleFTBoxHandler.h"

SimpleFieldTypeBoxHandler::SimpleFieldTypeBoxHandler(CEditDBDialog* d, UInt32 jFileFieldType)
	: FieldTypeBoxHandler(d), fJFileType(jFileFieldType)
{
}

UInt32 SimpleFieldTypeBoxHandler::GetJFileFieldType()
{
	return fJFileType;
}

#include "CEditDBDialog.h"
#include "FieldTypeBoxHandler.h"

FieldTypeBoxHandler::FieldTypeBoxHandler(CEditDBDialog* d)
	: fDialog(d), fView(d->GetView())
{
}

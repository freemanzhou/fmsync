#include <types.r>
#include "WarningsStrings.h"

resource 'STR#' (kWarningsTitleStrings, purgeable)
{
{
	"Field order will be fixed",
	"Multiuser database do not work";
}
};

resource 'STR#' (kWarningsDescStrings, purgeable)
{
{
	"Since you are using FileMaker Pro version 3.x, the order of fields in"
	" JFile will not match the default tab order.",
	"Since you are using FileMaker Pro version 3.x, FMSync for JFile 5 cannot detect if the "
	"database is in Multiuser mode. Synchronization will not work with databases in "
	"multiuser mode.";
}
};

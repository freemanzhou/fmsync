#include "DragUtilities.h"

namespace Drag {

static Size MinimumBytesForFSSpec (const FSSpec *fss)
{
    // callers can and do assume this does not move memory
    return sizeof (*fss) - sizeof (fss->name) + *(fss->name) + 1;
}

void GetHFSFlavorFromDragReference(DragReference dragRef, ItemReference itemRef,
        HFSFlavor &hfsFlavor)
{
    OSErr err = noErr;

    Size size = sizeof (hfsFlavor);
    ThrowIfOSErr_(GetFlavorData 
        (dragRef,itemRef, flavorTypeHFS, &hfsFlavor,&size,0));

   Size minSize = sizeof (hfsFlavor) -
       sizeof (hfsFlavor.fileSpec);
   minSize += MinimumBytesForFSSpec (&(hfsFlavor.fileSpec));
       // see snippet 3 for MinimumBytesForFSSpec
   if (size < minSize)
       Throw_(cantGetFlavorErr);
}


bool HasFlavor(DragReference inDragRef, ItemReference inItemRef, FlavorType flavorType)
{
	FlavorFlags		theFlags;
	return (GetFlavorFlags(inDragRef, inItemRef, flavorType, &theFlags)
					== noErr);
}

}
#pragma once


namespace Drag {

void GetHFSFlavorFromDragReference(DragReference dragRef, ItemReference itemRef,
        HFSFlavor &hfsFlavor);
        
bool HasFlavor(DragReference dragRef, ItemReference itemRef, FlavorType flavorType);

template <class T>
void GetFlavor(DragReference dragRef, ItemReference itemRef, FlavorType flavorType, T* &p)
{
    Size size = sizeof (p);
    ThrowIfOSErr_(GetFlavorData 
        (dragRef,itemRef, flavorType, &p,&size,0));
}

}

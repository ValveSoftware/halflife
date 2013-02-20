
#ifndef VGUI_INT_H
#define VGUI_INT_H

extern "C"
{
void VGui_Startup();
void VGui_Shutdown();

//Only safe to call from inside subclass of Panel::paintBackground
void VGui_ViewportPaintBackground(int extents[4]);
}


#endif
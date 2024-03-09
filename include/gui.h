#pragma once

// Public functions
int GUI_init();
void GUI_NewFrame();
void GUI_Render();
void GUI_Destroy();

// Private functions
void CleanupDeviceD3D();
void ResetDevice();
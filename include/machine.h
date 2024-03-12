// Pragma once to avoid multiple inclusion
#pragma once

#define INDEX_REG 0
#define EXTENSION_REG 1

// 4K of memory ram; default is 256 bytes
#define RAM_SIZE 4096

void Machine_setup();
void Machine_ControlLoop();
void Machine_EmulationLoop();

void read_inputs();
void printer_writeByte(char c);
void emulator_loop();
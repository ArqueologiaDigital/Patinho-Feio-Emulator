#include <thread>
#include <iostream>
#include <SDL2/SDL.h>
#include "panel.h"
#include "machine.h"
#include "debug.h"

int main(int argc, char *argv[])
{

    Panel_init();
    std::thread Desenho(Panel_loop);

    Machine_setup();
    std::thread CPU(Machine_EmulationLoop);
    std::thread CONTROL(Machine_ControlLoop);

    Debug_init();
    Debug_loop();

    // while (true)
    // {
    //     // emulator_loop();
    //     if (Panel_close() > 0)
    //     {
    //         break;
    //     }
    // }

    Panel_destroy();
    Debug_destroy();

    // Destroy the CPU thread
    CPU.join();
    CONTROL.join();
    Desenho.join();

    return EXIT_SUCCESS;
}
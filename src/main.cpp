#include <thread>
#include <iostream>
#include <SDL2/SDL.h>
#include "panel.h"
#include "machine.h"

int main(int argc, char *argv[])
{
    Panel_init();
    std::thread Desenho(Panel_loop);

    Machine_setup();
    std::thread CPU(Machine_loop);

    while (true)
    {
        // emulator_loop();
        if (Panel_close() > 0)
        {
            break;
        }
    }

    Panel_destroy();

    return EXIT_SUCCESS;
}
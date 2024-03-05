#include <iostream>
#include <SDL2/SDL.h>
#include "panel.h"

const int WIDTH = 800;
const int HEIGHT = 600;

int main(int argc, char *argv[]){
    SDL_Init(SDL_INIT_VIDEO);

    test();

    SDL_Window *window = SDL_CreateWindow("Hello World", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WIDTH, HEIGHT, SDL_WINDOW_SHOWN);

    if(window == NULL){
        std::cout << "Window could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        return 1;
    }

    SDL_Event windowEvent;

    while(true){
        if(SDL_PollEvent(&windowEvent)){
            if(windowEvent.type == SDL_QUIT){
                break;
            }
        }
    }


    SDL_DestroyWindow(window);
    SDL_Quit();

    return EXIT_SUCCESS;
}
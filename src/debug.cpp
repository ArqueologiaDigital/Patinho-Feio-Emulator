#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>
#include <thread>
#include <chrono>
#include "machine.h"

#define SCREEN_WIDTH 450
#define SCREEN_HEIGHT 350
#define FONT_SIZE 20
#define ELEMENT_PER_ROW 8
#define SIZE 256

extern byte RAM[RAM_SIZE];

TTF_Font *font = NULL;
SDL_Color color = {255, 255, 255};
SDL_Renderer *renderer_debug;
SDL_Window *window_debug;

void render(SDL_Renderer *renderer, uint8_t *data)
{
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

    int ROWS = SIZE / 16;

    for (int row = 0; row < ROWS; row++)
    {
        int address = row * 16;
        char adr[64];
        sprintf(adr, "%04X: ", address);

        for (int i = 0; i < ELEMENT_PER_ROW; i++)
        {
            sprintf(adr + strlen(adr), "%02X%02X ", data[address + i * 2], data[address + i * 2 + 1]);
        }

        SDL_Surface *surface = TTF_RenderText_Solid(font, adr, color);
        SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);

        SDL_Rect dstRect;
        dstRect.x = FONT_SIZE;
        dstRect.y = (row * FONT_SIZE) + 10;
        dstRect.w = surface->w;
        dstRect.h = surface->h;

        SDL_RenderCopy(renderer, texture, NULL, &dstRect);
        SDL_DestroyTexture(texture);
        SDL_FreeSurface(surface);
    }

    SDL_RenderPresent(renderer);
}

int Debug_init()
{
    window_debug = SDL_CreateWindow("Hexdump", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    renderer_debug = SDL_CreateRenderer(window_debug, -1, SDL_RENDERER_ACCELERATED);

    if (TTF_Init() < 0)
    {
        printf("TTF_Init: %s\n", TTF_GetError());
        return 1;
    }

    font = TTF_OpenFont("assets/JoganSoft.ttf", 18);

    if (font == NULL)
    {
        printf("TTF_OpenFont: %s\n", TTF_GetError());
        return 1;
    }

    return 0;
}

void Debug_loop()
{
    while (true)
    {
        render(renderer_debug, RAM);
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
}

void Debug_destroy()
{
    SDL_DestroyRenderer(renderer_debug);
    SDL_DestroyWindow(window_debug);
    SDL_Quit();

    TTF_CloseFont(font);
    TTF_Quit();
}

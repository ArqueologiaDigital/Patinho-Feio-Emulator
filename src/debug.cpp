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

const char *Debug_get_mnemonic(int opcode)
{
    // Switch based on the opcode
    switch (opcode)
    {
    case 0x80:
        return "LIMPO";
    case 0x81:
        return "UM";
    case 0x82:
        return "CMP1";
    case 0x83:
        return "CMP2";
    case 0x84:
        return "LIM";
    case 0x85:
        return "INC";
    case 0x86:
        return "UNEG";
    case 0x87:
        return "LIMP1";
    case 0x88:
        return "PNL_0";
    case 0x89:
        return "PNL_1";
    case 0x8A:
        return "PNL_2";
    case 0x8B:
        return "PNL_3";
    case 0x8C:
        return "PNL_4";
    case 0x8D:
        return "PNL_5";
    case 0x8E:
        return "PNL_6";
    case 0x8F:
        return "PNL_7";
    case 0x90:
        return "ST_0";
    case 0x91:
        return "STM_0";
    case 0x92:
        return "ST_1";
    case 0x93:
        return "STM_1";
    case 0x94:
        return "SV_0";
    case 0x95:
        return "SVM_0";
    case 0x96:
        return "SV_1";
    case 0x97:
        return "SVM_1";
    case 0x98:
        return "PUL";
    case 0x99:
        return "TRE";
    case 0x9A:
        return "INIB";
    case 0x9B:
        return "PERM";
    case 0x9C:
        return "ESP";
    case 0x9D:
        return "PARE";
    case 0x9E:
        return "TRI";
    case 0x9F:
        return "IND";
    case 0xD1:
        return "SH/RT/XOR/NAND";
    case 0xD2:
        return "XOR";
    case 0xD4:
        return "NAND";
    case 0xD8:
        return "SOMI";
    case 0xDA:
        return "CARI";
    default:
        // Switch based on the opcode's most significant nibble
        switch (opcode & 0xF0)
        {
        case 0x00:
            return "PLA";
        case 0x10:
            return "PLAX";
        case 0x20:
            return "ARM";
        case 0x30:
            return "ARMX";
        case 0x40:
            return "CAR";
        case 0x50:
            return "CARX";
        case 0x60:
            return "SOM";
        case 0x70:
            return "SOMX";
        case 0xA0:
            return "PLAN";
        case 0xB0:
            return "PLAZ";
        case 0xC0:
            return "IO";
        case 0xE0:
            return "SUS";
        case 0xF0:
            return "PUG";
        default:
            return "PARADO";
        }
    }
}

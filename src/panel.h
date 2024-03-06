// Pragma once to avoid multiple inclusion
#pragma once
#include <cstdint>
#include <SDL2/SDL.h>

struct button_t
{
    int x, y, w, h;
    bool state;
};

void Panel_init();
void Panel_destroy();
void Panel_backgroundDefault();
void Panel_8BIT(int posX, int posY, bool *led_panel);
void Panel_12BIT(int posX, int posY, bool *led_panel);
void Panel_refreshButton(button_t *btn, int size);
void DrawCircle(SDL_Renderer *renderer, int32_t centreX, int32_t centreY, int32_t radius, bool fill);
int Panel_close();
void Panel_loop();

void Panel_createButton(int posX, int posY, int id, button_t *btn);
void handleMouseClick(int x, int y, button_t *btn, int size);
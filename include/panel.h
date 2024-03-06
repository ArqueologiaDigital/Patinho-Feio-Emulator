// Pragma once to avoid multiple inclusion
#pragma once
#include <cstdint>
#include <SDL2/SDL.h>

#define BTN_TYPE_TOGGLE 1
#define BTN_TYPE_PUSH 2

struct button_t
{
    int type;
    int x, y, w, h;
    bool state;
    bool glowing;
};

void Panel_init();
void Panel_destroy();
void Panel_backgroundDefault();
void Panel_LEDBIT(int posX, int posY, int bits, bool *led_panel, float dist_between = 20.6);
void Panel_refreshButton(button_t *btn, int size);
void DrawCircle(SDL_Renderer *renderer, int32_t centreX, int32_t centreY, int32_t radius, bool fill);
int Panel_close();
void Panel_loop();

void Panel_createButton(int posX, int posY, int id, int type, button_t *btn);
void handleMouseClick(int x, int y, button_t *btn, int size);
void handlerMouseUp(button_t *btn, int size);
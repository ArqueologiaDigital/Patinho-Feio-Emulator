#include <iostream>
#include <thread>
#include <chrono>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include "panel.h"
#include "state.h"

SDL_Window *window;
SDL_Renderer *renderer;
SDL_Event windowEvent;

const float zoom = 1.5;

const int WIDTH = 667 * zoom;
const int HEIGHT = 600 * zoom;

extern bool led_ACC[8]; // 8-bit "Acumulador" = Accumulator Register
extern bool led_RD[8];  // 8-bit "Registrador de Dados" = Data Register
extern bool led_RI[8];  // 8-bit "Registrador de Instrução" = Instruction Register

extern bool led_RE[12];              // 12-bit "Registrador de Endereço" = Address Register
extern bool led_CI[12];              // 12-bit "Contador de Instrução" = Instruction Counter
extern bool led_DADOS_DO_PAINEL[12]; // 12-bit of data provided by the user via panel toggle-switches

extern bool led_FASE[7];  // "Fase" = Phase
extern bool led_STATE[2]; // "Estado" = State (parado e externo)

extern button_t buttons[QTD_BUTTONS_GENERAL]; // 0 - Endereçamento, 1 - Memoria, 2 - Espera, 3 - Interrupção, 4 - Partida , 5 - Preparação
extern button_t btn_address[QTD_BUTTONS_ADDRESS];
extern button_t btn_mode[QTD_BUTTONS_MODE];

using namespace std;

/*
 * ################### UTILS ###################
 */

// Function to handle mouse click events
void handleMouseClick(int x, int y, button_t *btn, int size)
{

    for (int i = 0; i < size; i++)
    {
        if (x > btn[i].x && x < btn[i].x + btn[i].w && y > btn[i].y && y < btn[i].y + btn[i].h)
        {

            if (btn[i].type == BTN_TYPE_TOGGLE)
            {
                btn[i].state = !btn[i].state;
            }
            else if (btn[i].type == BTN_TYPE_PUSH)
            {
                btn[i].state = true;
            }
        }
    }
}

void handlerMouseUp(button_t *btn, int size)
{
    for (int i = 0; i < size; i++)
    {
        if (btn[i].type == BTN_TYPE_PUSH)
        {
            btn[i].state = false;
        }
    }
}

void DrawCircle(SDL_Renderer *renderer, int32_t centreX, int32_t centreY, int32_t radius, bool fill)
{
    const int32_t diameter = (radius * 2);

    int32_t x = (radius - 1);
    int32_t y = 0;
    int32_t tx = 1;
    int32_t ty = 1;
    int32_t error = (tx - diameter);

    while (x >= y)
    {
        // Each of the following renders an octant of the circle
        SDL_RenderDrawPoint(renderer, centreX + x, centreY - y);
        SDL_RenderDrawPoint(renderer, centreX + x, centreY + y);
        SDL_RenderDrawPoint(renderer, centreX - x, centreY - y);
        SDL_RenderDrawPoint(renderer, centreX - x, centreY + y);
        SDL_RenderDrawPoint(renderer, centreX + y, centreY - x);
        SDL_RenderDrawPoint(renderer, centreX + y, centreY + x);
        SDL_RenderDrawPoint(renderer, centreX - y, centreY - x);
        SDL_RenderDrawPoint(renderer, centreX - y, centreY + x);

        if (error <= 0)
        {
            ++y;
            error += ty;
            ty += 2;
        }

        if (error > 0)
        {
            --x;
            tx += 2;
            error += (tx - diameter);
        }
    }

    // Fill circle
    if (fill)
    {
        for (int i = centreX - radius; i < centreX + radius; i++)
        {
            for (int j = centreY - radius; j < centreY + radius; j++)
            {
                if ((i - centreX) * (i - centreX) + (j - centreY) * (j - centreY) < radius * radius)
                {
                    SDL_RenderDrawPoint(renderer, i, j);
                }
            }
        }
    }
}

void Panel_init()
{
    SDL_Init(SDL_INIT_VIDEO);

    window = SDL_CreateWindow("Hello World", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WIDTH, HEIGHT, SDL_WINDOW_SHOWN);
    if (window == NULL)
    {
        std::cout << "Window could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        return;
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == NULL)
    {
        std::cout << "Renderer could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        return;
    }

    // Set background color to #83ab8f
    Panel_backgroundDefault();

    // Render button
    Panel_createButton(663, 495, 0, BTN_TYPE_PUSH, buttons);
    Panel_createButton(838, 495, 1, BTN_TYPE_PUSH, buttons);

    // Render button to memory address, 12 bits, horizontal
    for (int i = 0; i < 12; i++)
    {
        Panel_createButton(132 + (i * 63), 622, i, BTN_TYPE_TOGGLE, btn_address);
    }

    // Render button to mode, 6 bits, horizontal
    for (int i = 0; i < 6; i++)
    {
        Panel_createButton(95 + (i * 79), 750, i, BTN_TYPE_PUSH, btn_mode);
    }

    Panel_createButton(706, 750, 2, BTN_TYPE_PUSH, buttons);
    Panel_createButton(784, 750, 3, BTN_TYPE_PUSH, buttons);
    Panel_createButton(865, 750, 4, BTN_TYPE_PUSH, buttons);
    Panel_createButton(865, 822, 5, BTN_TYPE_PUSH, buttons);

    // for (int i = 0; i < 12; i++)
    // {
    //     led_RE[i] = true;
    //     led_CI[i] = true;
    //     led_DADOS_DO_PAINEL[i] = true;
    // }

    // for (int i = 0; i < 7; i++)
    // {
    //     led_FASE[i] = true;
    // }
}

void Panel_loop()
{

    while (true)
    {
        Panel_LEDBIT(597, 66, 12, led_RE); // Endereço de memória

        Panel_LEDBIT(201, 155, 8, led_ACC); // Acumulador
        Panel_LEDBIT(597, 155, 12, led_CI); // Endereço de instrução

        Panel_LEDBIT(201, 245, 8, led_RI); // Código de instrução

        Panel_LEDBIT(201, 332, 8, led_RD);               // Dados da Memória
        Panel_LEDBIT(597, 370, 12, led_DADOS_DO_PAINEL); // Dados do Painel

        Panel_LEDBIT(110, 507, 5, led_FASE, 66);      // Fase
        Panel_LEDBIT(178, 575, 2, led_FASE + 5, 130); // Fase
        Panel_LEDBIT(468, 507, 2, led_STATE, 81);     // Estado

        Panel_refreshButton(buttons, 6);
        Panel_refreshButton(btn_address, 12);
        Panel_refreshButton(btn_mode, 6);

        SDL_RenderPresent(renderer);

        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
}

void Panel_destroy()
{
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

void Panel_backgroundDefault()
{
    // Set background color to #83ab8f
    // SDL_SetRenderDrawColor(renderer, 0x83, 0xab, 0x8f, 0xff);
    // SDL_RenderClear(renderer);

    SDL_Surface *surface = IMG_Load("assets/panel.png");
    if (surface == NULL)
    {
        cout << "Unable to load image! SDL_Error: " << SDL_GetError() << endl;
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
    }

    // Create a texture from the loaded surface
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface); // Surface no longer needed

    if (texture == NULL)
    {
        cout << "Unable to create texture from surface! SDL_Error: " << SDL_GetError() << endl;
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
    }

    // Clear the renderer
    SDL_RenderClear(renderer);

    // Draw the texture to the renderer
    SDL_RenderCopy(renderer, texture, NULL, NULL);
}

// Function to render the 8 bit panel
void Panel_LEDBIT(int posX, int posY, int bits, bool *led_panel, float dist_between)
{
    // Draw the first panel
    for (int i = 0; i < bits; i++)
    {
        if (led_panel[i])
        {
            SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        }
        else
        {
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        }

        // Horizontal line leds
        DrawCircle(renderer, (posX) + (i * dist_between), posY, 6, true);
    }
}

// Function to render the button
void Panel_createButton(int posX, int posY, int id, int type, button_t *btn)
{

    // Check exist button
    SDL_Rect buttonRect;
    buttonRect.x = posX;
    buttonRect.y = posY;
    buttonRect.w = 30;
    buttonRect.h = 30;

    btn[id].x = buttonRect.x;
    btn[id].y = buttonRect.y;
    btn[id].w = buttonRect.w;
    btn[id].h = buttonRect.h;
    btn[id].type = type;
    btn[id].state = false;

    // Set button color based on state
    SDL_SetRenderDrawColor(renderer, 0xFF, 0x00, 0x00, 0xFF);
    SDL_RenderFillRect(renderer, &buttonRect);
}

// Function to refresh the button
void Panel_refreshButton(button_t *btn, int size)
{
    for (int i = 0; i < size; i++)
    {
        SDL_Rect buttonRect;
        buttonRect.x = btn[i].x;
        buttonRect.y = btn[i].y;
        buttonRect.w = btn[i].w;
        buttonRect.h = btn[i].h;

        if (btn[i].type == BTN_TYPE_PUSH)
        {

            // GLowing true and state true -> red
            // Glowing true and state false -> amber
            // Glowing false and state true -> green
            // Glowing false and state false -> back
            if (btn[i].glowing && btn[i].state)
            {
                // Red
                SDL_SetRenderDrawColor(renderer, 0xFF, 0x00, 0x00, 0xFF);
            }
            else if (btn[i].glowing && !btn[i].state)
            {
                // Amber
                SDL_SetRenderDrawColor(renderer, 0xFF, 0xBF, 0x00, 0xFF);
            }
            else if (!btn[i].glowing && btn[i].state)
            {
                // Dark Amber
                SDL_SetRenderDrawColor(renderer, 0x45, 0x34, 0x01, 0xFF);
            }
            else
            {
                // Back
                SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xFF);
            }
        }
        else if (btn[i].type == BTN_TYPE_TOGGLE)
        {
            if (btn[i].state)
            {
                SDL_SetRenderDrawColor(renderer, 0x00, 0xFF, 0x00, 0xFF);
            }
            else
            {
                SDL_SetRenderDrawColor(renderer, 0xFF, 0x00, 0x00, 0xFF);
            }
        }

        SDL_RenderFillRect(renderer, &buttonRect);
    }
}

// TODO: Mudar para outro local
int Panel_close()
{
    if (SDL_PollEvent(&windowEvent))
    {
        if (windowEvent.type == SDL_QUIT)
        {
            return 1;
        }
        else if (windowEvent.type == SDL_MOUSEBUTTONDOWN)
        {
            int x, y;
            SDL_GetMouseState(&x, &y);
            handleMouseClick(x, y, buttons, 6);
            handleMouseClick(x, y, btn_address, 12);
            handleMouseClick(x, y, btn_mode, 6);
        }
        else if (windowEvent.type == SDL_MOUSEBUTTONUP)
        {
            handlerMouseUp(buttons, 6);
            handlerMouseUp(btn_address, 12);
            handlerMouseUp(btn_mode, 6);
        }
    }

    return 0;
}
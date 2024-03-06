#include <iostream>
#include <thread>
#include <chrono>
#include <SDL2/SDL.h>
#include "panel.h"
#include "state.h"

SDL_Window *window;
SDL_Renderer *renderer;
SDL_Event windowEvent;

const int WIDTH = 620;
const int HEIGHT = 853;

extern bool led_ACC[8]; // 8-bit "Acumulador" = Accumulator Register
extern bool led_RD[8];  // 8-bit "Registrador de Dados" = Data Register
extern bool led_RI[8];  // 8-bit "Registrador de Instrução" = Instruction Register

extern bool led_RE[12];              // 12-bit "Registrador de Endereço" = Address Register
extern bool led_CI[12];              // 12-bit "Contador de Instrução" = Instruction Counter
extern bool led_DADOS_DO_PAINEL[12]; // 12-bit of data provided by the user via panel toggle-switches

extern button_t buttons[2];
extern button_t btn_address[12];
extern button_t btn_mode[6];

using namespace std;

/*
 * ################### UTILS ###################
 */

// Function to handle mouse click events
void handleMouseClick(int x, int y, button_t *btn, int size)
{

    for (int i = 0; i < size; i++)
    {
        if (btn[i].w == 0 || btn[i].h == 0)
        {
            continue;
        }

        if (x > btn[i].x && x < btn[i].x + btn[i].w && y > btn[i].y && y < btn[i].y + btn[i].h)
        {
            std::cout << "Button " << i << " clicked!" << std::endl;
            btn[i].state = !btn[i].state;
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
    Panel_createButton(18, 18, 0, buttons);
    Panel_createButton(20, 18, 1, buttons);

    // Render button to memory address, 12 bits, horizontal
    for (int i = 0; i < 12; i++)
    {
        Panel_createButton(1 + i * 2, 20, i, btn_address);
    }

    // Render button to mode, 6 bits, horizontal
    for (int i = 0; i < 6; i++)
    {
        Panel_createButton(1 + i * 2, 22, i, btn_mode);
    }
}

void Panel_loop()
{

    while (true)
    {
        Panel_12BIT(10, 0, led_RE); // Endereço de memória

        Panel_8BIT(0, 5, led_ACC);  // Acumulador
        Panel_12BIT(10, 5, led_CI); // Endereço de instrução

        Panel_8BIT(0, 10, led_RI); // Código de instrução

        Panel_8BIT(0, 15, led_RD);                // Dados da Memória
        Panel_12BIT(10, 15, led_DADOS_DO_PAINEL); // Dados do Painel

        Panel_refreshButton(buttons, 2);
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
    SDL_SetRenderDrawColor(renderer, 0x83, 0xab, 0x8f, 0xff);
    SDL_RenderClear(renderer);
}

// Function to render the 8 bit panel
void Panel_8BIT(int posX, int posY, bool *led_panel)
{
    // Draw the first panel
    for (int i = 0; i < 8; i++)
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
        DrawCircle(renderer, 25 + posX * 25 + i * 25, 25 + posY * 25, 10, true);
    }
}

// Function to render the 12 bit panel
void Panel_12BIT(int posX, int posY, bool *led_panel)
{
    // Draw the first panel
    for (int i = 0; i < 12; i++)
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
        DrawCircle(renderer, 25 + posX * 25 + i * 25, 25 + posY * 25, 10, true);
    }
}

// Function to render the button
void Panel_createButton(int posX, int posY, int id, button_t *btn)
{

    // Check exist button
    SDL_Rect buttonRect;
    buttonRect.x = posX * 25;
    buttonRect.y = posY * 25;
    buttonRect.w = 25;
    buttonRect.h = 25;

    btn[id].x = buttonRect.x;
    btn[id].y = buttonRect.y;
    btn[id].w = buttonRect.w;
    btn[id].h = buttonRect.h;
    btn[id].state = false;

    cout << "Button " << id << " created!" << endl;

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

        if (btn[i].state)
        {
            SDL_SetRenderDrawColor(renderer, 0x00, 0xFF, 0x00, 0xFF);
        }
        else
        {
            SDL_SetRenderDrawColor(renderer, 0x54, 0x00, 0x00, 0xFF);
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
            handleMouseClick(x, y, buttons, 2);
            handleMouseClick(x, y, btn_address, 12);
            handleMouseClick(x, y, btn_mode, 6);
        }
    }

    return 0;
}
#include "state.h"
#include "panel.h"
#include "machine.h"

bool running = true;

bool led_ACC[8]; // 8-bit "Acumulador" = Accumulator Register
bool led_RD[8];  // 8-bit "Registrador de Dados" = Data Register
bool led_RI[8];  // 8-bit "Registrador de Instrução" = Instruction Register

bool led_RE[12];              // 12-bit "Registrador de Endereço" = Address Register
bool led_CI[12];              // 12-bit "Contador de Instrução" = Instruction Counter
bool led_DADOS_DO_PAINEL[12]; // 12-bit of data provided by the user via panel toggle-switches

bool led_FASE[7];  // "Fase" = Phase
bool led_STATE[2]; // "Estado" = State (parado e externo)

button_t buttons[QTD_BUTTONS_GENERAL]; // 0 - Endereçamento, 1 - Memoria, 2 - Espera, 3 - Interrupção, 4 - Partida , 5 - Preparação
button_t btn_address[QTD_BUTTONS_ADDRESS];
button_t btn_mode[QTD_BUTTONS_MODE];

byte RAM[RAM_SIZE];

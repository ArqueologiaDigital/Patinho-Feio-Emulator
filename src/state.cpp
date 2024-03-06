#include "state.h"
#include "panel.h"

bool led_ACC[8]; // 8-bit "Acumulador" = Accumulator Register
bool led_RD[8];  // 8-bit "Registrador de Dados" = Data Register
bool led_RI[8];  // 8-bit "Registrador de Instrução" = Instruction Register

bool led_RE[12];              // 12-bit "Registrador de Endereço" = Address Register
bool led_CI[12];              // 12-bit "Contador de Instrução" = Instruction Counter
bool led_DADOS_DO_PAINEL[12]; // 12-bit of data provided by the user via panel toggle-switches

button_t buttons[2];
button_t btn_address[12];
button_t btn_mode[6];

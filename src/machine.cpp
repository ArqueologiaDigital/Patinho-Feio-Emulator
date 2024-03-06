#include "state.h"
#define byte unsigned char

extern bool led_ACC[8]; // 8-bit "Acumulador" = Accumulator Register
extern bool led_RD[8];  // 8-bit "Registrador de Dados" = Data Register
extern bool led_RI[8];  // 8-bit "Registrador de Instrução" = Instruction Register

extern bool led_RE[12];              // 12-bit "Registrador de Endereço" = Address Register
extern bool led_CI[12];              // 12-bit "Contador de Instrução" = Instruction Counter
extern bool led_DADOS_DO_PAINEL[12]; // 12-bit of data provided by the user via panel toggle-switches

extern button_t buttons[2];
extern button_t btn_address[12];
extern button_t btn_mode[6];

// CPU registers:

#define INDEX_REG 0
#define EXTENSION_REG 1
#define RAM_SIZE 256

byte RAM[RAM_SIZE];
bool _VAI_UM;
bool _TRANSBORDO;

int _RE;              // 12-bit "Registrador de Endereço" = Address Register
int _RD;              //  8-bit "Registrador de Dados" = Data Register
int _RI;              //  8-bit "Registrador de Instrução" = Instruction Register
int _ACC;             //  8-bit "Acumulador" = Accumulator Register
int _CI;              // 12-bit "Contador de Instrução" = Instruction Counter
int _DADOS_DO_PAINEL; // 12-bit of data provided by the user via panel toggle-switches

int _FASE; // determines which cycle of a cpu instruction we are currently executing

bool _PARADO;  // CPU is stopped.
bool _EXTERNO; // CPU is waiting interrupt.
int _MODO;     // CPU operation modes:

bool indirect_addressing;
bool scheduled_IND_bit_reset;
bool memoria_protegida = false;
bool enderecamento_sequencial = false;

#define NORMAL 1          // normal execution
#define CICLO_UNICO 2     // single-cycle
#define INSTRUCAO_UNICA 3 // single-instruction
#define ENDERECAMENTO 4   // addressing mode
#define ARMAZENAMENTO 5   // data write mode
#define EXPOSICAO 6       // data read mode

void DADOS_DO_PAINEL(int value)
{
    _DADOS_DO_PAINEL = value;

    for (int i = 0; i < 12; i++)
    {
        led_DADOS_DO_PAINEL[i] = value & (1 << i);
    }
}

// TODO: implement the carry led
void VAI_UM(bool value)
{
    _VAI_UM = value;
    // leds[2][7] = value;
}

// TODO: implement the overflow led
void TRANSBORDO(bool value)
{
    _TRANSBORDO = value;
    // leds[2][6] = value;
}

void PARADO(bool value)
{
    // This represents that the CPU is stopped.
    // Only a startup command ("PARTIDA") at execution modes (NORMAL, SINGLE-INST,  or SINGLE-CYCLE) can restart it.
    _PARADO = value;
    // leds[2][4] = value;
}

void EXTERNO(bool value)
{
    // This represents that the CPU is stopped
    // waiting for an interrupt from an external device.
    _EXTERNO = value;
    // leds[2][3] = value;
}

void CI(int value)
{
    _CI = value;
    for (int i = 0; i < 12; i++)
    {
        led_CI[i] = value & (1 << i);
    }
}

void RE(int value)
{
    _RE = value;
    for (int i = 0; i < 12; i++)
    {
        led_RE[i] = value & (1 << i);
    }
}

void RD(int value)
{
    _RD = value;
    for (int i = 0; i < 8; i++)
    {
        led_RD[i] = value & (1 << i);
    }
}

void RI(int value)
{
    _RI = value;
    for (int i = 0; i < 8; i++)
    {
        led_RI[i] = value & (1 << i);
    }
}

void ACC(int value)
{
    _ACC = value;
    for (int i = 0; i < 8; i++)
    {
        led_ACC[i] = value & (1 << i);
    }
}

// TODO: implement the leds for the phase
void FASE(int value)
{
    _FASE = value;
    int k = 0;
    int map_col[7] = {11, 13, 14, 8, 12, 15, 9};
    int map_row[7] = {0, 0, 0, 0, 0, 0, 0};
    for (int i = 0; i < 7; i++)
    {
        // leds[map_row[i]][map_col[i]] = (_FASE == i + 1);
    }
}

void MODO(int value)
{
    _MODO = value;
    for (int i = 0; i < 6; i++)
    {
        btn_mode[i].state = (value == i + 1);
    }
}

void LED_ESPERA(bool value)
{
    // I think this button did not really have a lamp
}

void LED_INTERRUPCAO(bool value)
{
    // I think this button did not really have a lamp
}

void LED_PREPARACAO(bool value)
{
    // I think this button did not really have a lamp
}

void reset_CPU()
{
    VAI_UM(false);
    TRANSBORDO(false);
    CI(0x000);
    RE(0x000);
    RD(0x00);
    RI(0x00);
    ACC(0x00);
    DADOS_DO_PAINEL(0x000);
    FASE(1);
    PARADO(true);
    EXTERNO(false);
    MODO(NORMAL);
    LED_ESPERA(false);
    LED_INTERRUPCAO(false);
    LED_PREPARACAO(false);
    indirect_addressing = false;
    scheduled_IND_bit_reset = false;
}
int inc = 0;
void read_inputs()
{
    int col;
    // read_switch_matrix();
    // read the toggle switches and update DADOS_DO_PAINEL
    int dados = 0;
    for (col = 0; col < 12; col++)
    {
        if (btn_address[col].state)
        {
            dados |= (1 << col);
        }
    }
    DADOS_DO_PAINEL(dados);

    // when a mode button is pressed, set the corresponding mode:
    for (col = 0; col < 6; col++)
    {
        // if (inputs[2][col])
        // {
        //     MODO(6 - col);
        // }
    }

    // chaves de modos de memória:
    enderecamento_sequencial = buttons[0].state;
    memoria_protegida = buttons[1].state;

    // // botão "PREPARAÇÂO":
    if (inc == 0)
    {
        reset_CPU();
    }

    inc++;

    // // botão "ESPERA":
    // if (inputs[1][3])
    //     espera();

    // // botão "INTERRUPÇÂO"
    // if (inputs[1][2])
    //     interrupcao();

    // // botão "PARTIDA":
    // if (inputs[1][1])
    //     partida();
}

void Machine_loop()
{
    read_inputs();
}
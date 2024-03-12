#include <iostream>
#include <thread>
#include <chrono>
#include <bitset>
#include "state.h"
#include "machine.h"
#include "debug.h"
#include "interface.h"

using namespace std;

#define DEBUG 1

// Controller Emulation
extern bool running;

// Panel connections:
extern bool led_ACC[8]; // 8-bit "Acumulador" = Accumulator Register
extern bool led_RD[8];  // 8-bit "Registrador de Dados" = Data Register
extern bool led_RI[8];  // 8-bit "Registrador de Instrução" = Instruction Register

extern bool led_RE[12];              // 12-bit "Registrador de Endereço" = Address Register
extern bool led_CI[12];              // 12-bit "Contador de Instrução" = Instruction Counter
extern bool led_DADOS_DO_PAINEL[12]; // 12-bit of data provided by the user via panel toggle-switches

extern bool led_FASE[7];  // "Fase" = Phase
extern bool led_STATE[2]; // "Estado" = State (parado e externo)

extern button_t buttons[QTD_BUTTONS_GENERAL];
extern button_t btn_address[QTD_BUTTONS_ADDRESS];
extern button_t btn_mode[QTD_BUTTONS_MODE];

// CPU registers:

extern byte RAM[RAM_SIZE];
bool _VAI_UM;
bool _TRANSBORDO;

int _RE;              // 12-bit "Registrador de Endereço" = Address Register
int _RD;              //  8-bit "Registrador de Dados" = Data Register
int _RI;              //  8-bit "Registrador de Instrução" = Instruction Register
int _ACC;             //  8-bit "Acumulador" = Accumulator Register
int _CI;              // 12-bit "Contador de Instrução" = Instruction Counter
int old_CI;           // 12-bit "Contador de Instrução" = Instruction Counter
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

// IO
extern InterfaceIO *canais[16];

byte read_index_reg()
{
    return RAM[INDEX_REG];
}

void write_index_reg(byte value)
{
    RAM[INDEX_REG] = value;
}

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
    led_STATE[0] = value;
}

void EXTERNO(bool value)
{
    // This represents that the CPU is stopped
    // waiting for an interrupt from an external device.
    _EXTERNO = value;
    led_STATE[1] = value;
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

void FASE(int value)
{
    _FASE = value;
    for (int i = 0; i < 7; i++)
    {
        led_FASE[i] = (_FASE == i + 1);
    }
}

void MODO(int value)
{
    _MODO = value;
    for (int i = 0; i < 6; i++)
    {
        btn_mode[i].glowing = (value == i + 1);
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

byte read_ram(int addr)
{
    return RAM[addr];
}

void write_ram(int addr, byte value)
{
    RAM[addr % RAM_SIZE] = value;
}

void inc_CI()
{
    CI((_CI + 1) % RAM_SIZE);
}

unsigned int compute_effective_address(unsigned int addr)
{
    if (indirect_addressing)
    {
        addr = (read_ram(addr + 1) << 8) | read_ram(addr);
        if (addr & 0x1000)
        {
            return compute_effective_address(addr & 0xFFF);
        }
    }
    return addr;
}

byte opcode;

/*******************************
CPU Instructions implementation
********************************/

void PLA_instruction()
{
    /* OPCODE: 0x0X */
    // PLA = "Pula": Jump to address
    unsigned int addr = (opcode & 0x0F) << 8 | read_ram(_CI);
    addr = compute_effective_address(addr);
    inc_CI();
    CI(addr);
}

void PLAX_instruction()
{
    /* OPCODE: 0x1X */
    // PLAX = "Pula indexado": Jump to indexed address
    unsigned int addr = (opcode & 0x0F) << 8 | read_ram(_CI);
    inc_CI();
    byte idx = read_index_reg();
    addr = compute_effective_address(idx + addr);
    CI(addr);
}

void ARM_instruction()
{
    /* OPCODE: 0x2X */
    // ARM = "Armazena": Store the value of the accumulator into a given memory position
    unsigned int addr = (opcode & 0x0F) << 8 | read_ram(_CI);
    addr = compute_effective_address(addr);
    inc_CI();
    write_ram(addr, _ACC);
}

void ARMX_instruction()
{
    /* OPCODE: 0x3X */
    // ARMX = "Armazena indexado": Store the value of the accumulator into a given indexed memory position
    unsigned int addr = (opcode & 0x0F) << 8 | read_ram(_CI);
    inc_CI();
    byte idx = read_index_reg();
    addr = compute_effective_address(idx + addr);
    write_ram(addr, _ACC);
}

void CAR_instruction()
{
    /* OPCODE: 0x4X */
    // CAR = "Carrega": Load a value from a given memory position into the accumulator
    unsigned int addr = (opcode & 0x0F) << 8 | read_ram(_CI);
    inc_CI();
    addr = compute_effective_address(addr);
    ACC(read_ram(addr));
}

void CARX_instruction()
{
    /* OPCODE: 0x5X */
    // CARX = "Carga indexada": Load a value from a given indexed memory position into the accumulator
    unsigned int addr = (opcode & 0x0F) << 8 | read_ram(_CI);
    inc_CI();
    byte idx = read_index_reg();
    addr = compute_effective_address(idx + addr);
    ACC(read_ram(addr));
}

void SOM_instruction()
{
    /* OPCODE: 0x6X */
    // SOM = "Soma": Add a value from a given memory position into the accumulator
    unsigned int addr = (opcode & 0x0F) << 8 | read_ram(_CI);
    addr = compute_effective_address(addr);
    inc_CI();
    ACC(_ACC + read_ram(addr));
    // TODO: update V and T flags
}

void SOMX_instruction()
{
    /* OPCODE: 0x7X */
    // SOMX = "Soma indexada": Add a value from a given indexed memory position into the accumulator
    unsigned int addr = (opcode & 0x0F) << 8 | read_ram(_CI);
    inc_CI();
    byte idx = read_index_reg();
    addr = compute_effective_address(idx + addr);
    ACC(_ACC + read_ram(addr));
    // TODO: update V and T flags
}

void LIMPO_instruction()
{
    /* OPCODE: 0x80 */
    // LIMPO:
    //      Clear accumulator and flags
    ACC(0);
    TRANSBORDO(0);
    VAI_UM(0);
}

void UM_instruction()
{
    /* OPCODE: 0x81 */
    // UM="One":
    //     Load 1 into accumulator
    //     and clear the flags
    ACC(1);
    TRANSBORDO(0);
    VAI_UM(0);
}

void CMP1_instruction()
{
    /* OPCODE: 0x82 */
    // CMP1:
    //  Compute One's complement of the accumulator
    //     and clear the flags
    ACC(~_ACC);
    TRANSBORDO(0);
    VAI_UM(0);
}

void CMP2_instruction()
{
    /* OPCODE: 0x83 */
    // CMP2:
    //  Compute Two's complement of the accumulator
    //     and updates flags according to the result of the operation
    ACC(~_ACC + 1);

    // TODO: fix-me (I'm not sure yet how to compute the flags here):
    TRANSBORDO(0);
    VAI_UM(0);
}

void LIM_instruction()
{
    /* OPCODE: 0x84 */
    // LIM="Limpa":
    //  Clear flags
    TRANSBORDO(0);
    VAI_UM(0);
}

void INC_instruction()
{
    /* OPCODE: 0x85 */
    // INC:
    //  Increment accumulator
    ACC(_ACC + 1);
    TRANSBORDO(0); // TODO: fix-me (I'm not sure yet how to compute the flags here)
    VAI_UM(0);     // TODO: fix-me (I'm not sure yet how to compute the flags here)
}

void UNEG_instruction()
{
    /* OPCODE: 0x86 */
    // UNEG="Um Negativo":
    //  Load -1 into accumulator and clear flags
    ACC(-1);
    TRANSBORDO(0);
    VAI_UM(0);
}

void LIMP1_instruction()
{
    /* OPCODE: 0x87 */
    // LIMP1:
    //     Clear accumulator, reset T and set V
    ACC(0);
    TRANSBORDO(0);
    VAI_UM(1);
}

void PNL_0_instruction()
{
    /* OPCODE: 0x88 */
    // PNL 0:
    ACC(_DADOS_DO_PAINEL & 0xFF);
    TRANSBORDO(0);
    VAI_UM(0);
}

void PNL_1_instruction()
{
    /* OPCODE: 0x89 */
    // PNL 1:
    ACC((_DADOS_DO_PAINEL & 0xFF) + 1);
    // TODO: TRANSBORDO(?);
    // TODO: VAI_UM(?);
}

void PNL_2_instruction()
{
    /* OPCODE: 0x8A */
    // PNL 2:
    ACC((_DADOS_DO_PAINEL & 0xFF) - _ACC - 1);
    // TODO: TRANSBORDO(?);
    // TODO: VAI_UM(?);
}

void PNL_3_instruction()
{
    /* OPCODE: 0x8B */
    // PNL 3:
    ACC((_DADOS_DO_PAINEL & 0xFF) - _ACC);
    // TODO: TRANSBORDO(?);
    // TODO: VAI_UM(?);
}

void PNL_4_instruction()
{
    /* OPCODE: 0x8C */
    // PNL 4:
    ACC((_DADOS_DO_PAINEL & 0xFF) + _ACC);
    // TODO: TRANSBORDO(?);
    // TODO: VAI_UM(?);
}

void PNL_5_instruction()
{
    /* OPCODE: 0x8D */
    // PNL 5:
    ACC((_DADOS_DO_PAINEL & 0xFF) + _ACC + 1);
    // TODO: TRANSBORDO(?);
    // TODO: VAI_UM(?);
}

void PNL_6_instruction()
{
    /* OPCODE: 0x8E */
    // PNL 6:
    ACC((_DADOS_DO_PAINEL & 0xFF) - 1);
    // TODO: TRANSBORDO(?);
    // TODO: VAI_UM(?);
}

void PNL_7_instruction()
{
    /* OPCODE: 0x8F */
    // PNL 7:
    ACC(_DADOS_DO_PAINEL & 0xFF);
    VAI_UM(1);
}

void ST_0_instruction()
{
    /* OPCODE: 0x90 */
    // ST 0 = "Se T=0, Pula"
    //        If T is zero, skip the next instruction
    if (_TRANSBORDO == 0)
    {
        inc_CI(); // skip
    }
}

void STM_0_instruction()
{
    /* OPCODE: 0x91 */
    // STM 0 = "Se T=0, Pula e muda"
    //         If T is zero, skip the next instruction
    //         and toggle T.
    if (_TRANSBORDO == 0)
    {
        inc_CI(); // skip
        TRANSBORDO(1);
    }
}

void ST_1_instruction()
{
    /* OPCODE: 0x92 */
    // ST 1 = "Se T=1, Pula"
    //        If T is one, skip the next instruction
    if (_TRANSBORDO == 1)
    {
        inc_CI(); // skip
    }
}

void STM_1_instruction()
{
    /* OPCODE: 0x93 */
    // STM 1 = "Se T=1, Pula e muda"
    //         If T is one, skip the next instruction
    //         and toggle T.
    if (_TRANSBORDO == 1)
    {
        inc_CI(); // skip
        TRANSBORDO(0);
    }
}

void SV_0_instruction()
{
    /* OPCODE: 0x94 */
    // SV 0 = "Se V=0, Pula"
    //        If V is zero, skip the next instruction
    if (_VAI_UM == 0)
    {
        inc_CI(); // skip
    }
}

void SVM_0_instruction()
{
    /* OPCODE: 0x95 */
    // SVM 0 = "Se V=0, Pula e muda"
    //         If V is zero, skip the next instruction
    //         and toggle V.
    if (_VAI_UM == 0)
    {
        inc_CI(); // skip
        VAI_UM(1);
    }
}

void SV_1_instruction()
{
    /* OPCODE: 0x96 */
    // SV 1 = "Se V=1, Pula"
    //        If V is one, skip the next instruction
    if (_VAI_UM == 1)
    {
        inc_CI(); // skip
    }
}

void SVM_1_instruction()
{
    /* OPCODE: 0x97 */
    // SVM 1 = "Se V=1, Pula e muda"
    //         If V is one, skip the next instruction
    //         and toggle V.
    if (_VAI_UM == 1)
    {
        inc_CI(); // skip
        VAI_UM(0);
    }
}

void PUL_instruction()
{
    /* OPCODE: 0x98 */
    // PUL="Pula para /002 a limpa estado de interrupcao"
    //      Jump to address /002 and disables interrupts
    CI(0x002);
    EXTERNO(0); // TODO: verify this!
}

void TRE_instruction()
{
    /* OPCODE: 0x99 */
    // TRE="Troca conteúdos de ACC e EXT"
    //      Exchange the value of the accumulator with the ACC extension register
    byte value = _ACC;
    ACC(read_ram(EXTENSION_REG));
    write_ram(EXTENSION_REG, value);
}

void INIB_instruction()
{
    /* OPCODE: 0x9A */
    // INIB="Inibe"
    //      disables interrupts
    // TODO: Implement-me!  m_interrupts_enabled = false;
}

void PERM_instruction()
{
    /* OPCODE: 0x9B */
    // PERM="Permite"
    //      enables interrupts
    // TODO: Implement-me!  m_interrupts_enabled = true;
}

void ESP_instruction()
{
    /* OPCODE: 0x9C */
    // ESP="Espera":
    //     Holds execution and waits for an interrupt to occur.
    // TODO:  m_run = false;
    // TODO:  m_wait_for_interrupt = true;
}

void PARE_instruction()
{
    /* OPCODE: 0x9D */
    // PARE="Pare":
    //     Holds execution. This can only be recovered by
    //     manually triggering execution again by
    //     pressing the "Partida" (start) button in the panel
    PARADO(true);
    EXTERNO(false);
}

void TRI_instruction()
{
    /* OPCODE: 0x9E */
    // TRI="Troca com Indexador":
    //      Exchange the value of the accumulator with the index register
    byte value = _ACC;
    ACC(read_index_reg());
    write_index_reg(value);
}

void PLAN_instruction()
{
    /* OPCODE: 0xAX */
    // PLAN = "Pula se ACC negativo": Jump to a given address if ACC is negative
    unsigned int addr = (opcode & 0x0F) << 8 | read_ram(_CI);
    addr = compute_effective_address(addr);
    inc_CI();
    if ((signed char)_ACC < 0)
    {
        CI(addr);
    }
}

void PLAZ_instruction()
{
    /* OPCODE: 0xBX */
    // PLAZ = "Pula se ACC for zero": Jump to a given address if ACC is zero
    unsigned int addr = (opcode & 0x0F) << 8 | read_ram(_CI);
    addr = compute_effective_address(addr);
    inc_CI();
    if (_ACC == 0)
    {
        CI(addr);
    }
}

void FNC_instruction(byte channel, byte function)
{
    /* OPCODE: 0xCX 0x1X */
    // FNC = "Função tipo c para dispositivo N"
    // Implement-me!
}

void SAL_instruction(byte channel, byte function)
{
    /* OPCODE: 0xCX 0x2X */
    // SAL = "Salta"
    // SAL = Salto, tipo c para dispositivo N

    //     Skips a couple bytes if a condition is met
    bool skip = false;
    switch (function)
    {
    case 1:
        // TODO: implement-me! skip = (m_iodev_status[channel] == IODEV_READY);
        skip = true;
        break;
    case 2:
        /* TODO:
           skip = false;
           if (! m_iodev_is_ok_cb[channel].isnull()
              && m_iodev_is_ok_cb[channel](0))
        */
        skip = true;
        break;
    case 4:
        /*TODO:
           skip =false;
           if (! m_iodev_IRQ_cb[channel].isnull()
              && m_iodev_IRQ_cb[channel](0) == true)
        */
        skip = true;
        break;
    }
    if (skip)
    {
        inc_CI();
        inc_CI();
    }
}

void ENTR_instruction(byte channel)
{
    /* OPCODE: 0xCX 0x4X */
    /*
    ENTR = "Input data from I/O device"
    ENTR = "Entrada de dados, comando "C" do dispositivo "N"
    */
    // TODO: handle multiple device channels: m_iodev_write_cb[channel](ACC);
    // Implement-me!

    cout << "GETTING DATA FROM DEVICE " << endl;
    if (canais[channel])
    {
        ACC(canais[channel]->read());
    }
}

void SAI_instruction(byte channel)
{
    /* OPCODE: 0xCX 0x8X */
    /*
    SAI = Saida de dados, comando "C" para dispositivo "N"
    SAI = "Output data to I/O device"
    */
    if (canais[channel])
    {
        canais[channel]->write(_ACC);
    }
}

void IO_instructions()
{
    // Executes I/O functions
    // TODO: Implement-me!
    byte value = read_ram(_CI);
    inc_CI();
    byte channel = opcode & 0x0F;
    byte function = value & 0x0F;

#if DEBUG
    cout << "DISPOSITIVO N: " << hex << (int)channel << ", INSTRUCAO: " << hex << (int)(value & 0xF0) << ", FNC: " << hex << (int)function << endl;
#endif
    switch (value & 0xF0)
    {
    case 0x10: // FUNÇÃO TIPO C PARA DISPOSITIVO N
        FNC_instruction(channel, function);
        return;
    case 0x20: // SALTO, TIPO C PARA DISPOSITIVO N
        SAL_instruction(channel, function);
        return;
    case 0x40: // ENTRADA DE DADOS, COMANDO "C" DO DISPOSITIVO "N"
        ENTR_instruction(channel);
        return;
    case 0x80: // SAIDA DE DADOS, COMANDO "C" PARA DISPOSITIVO "N"
        SAI_instruction(channel);
        return;
    }
}

void SUS_instruction()
{
    /* OPCODE: 0xEX */
    // Subtrai trai 1 ou salta

    unsigned int addr = (opcode & 0x0F) << 8 | read_ram(_CI);
    inc_CI();

    // se (END) = 0: (CI) = (CI) + 2
    if (addr == 0x000)
    {
        cout << "E ZEROU" << endl;
        CI(_CI + 2);
    }
    // SE (END) != 0: (END) = (END) - 1
    else
    {
        write_ram(addr, read_ram(addr) - 1);
    }
}

void PUG_instruction()
{
    /* OPCODE: 0xFX */
    // PUG = "Pula e guarda" : Pega o CI e salva na RAM, CI tem 12 bits e a memoria tem 8 bits, o CI vai ocupar duas posições subsequentes na memoria
    // PUG = "Jump and save": Take the CI and save it in RAM, CI has 12 bits and the memory has 8 bits, the CI will occupy two subsequent positions in memory
    // Read address
    unsigned int addr = (opcode & 0x0F) << 8 | read_ram(_CI);
    inc_CI();

    // Read CI 8 bits first
    unsigned int CIFirst = (_CI >> 8) & 0x0F;
    write_ram(addr, CIFirst);

    // Read CI 4 bits
    unsigned int CISecond = _CI & 0xFF;
    write_ram(addr + 1, CISecond);

#if DEBUG
    // Debug
    // CI value
    cout << "CI: " << hex << _CI << endl;
    cout << "PUG: " << hex << (int)opcode << endl;
    cout << "1 - Value: " << hex << CIFirst << " Addr: " << hex << addr << endl;
    cout << "2 - Value: " << hex << CISecond << " Addr: " << hex << addr + 1 << endl;
#endif
    // Increment CI

    CI(addr + 2);
    //
}

void XOR_instruction()
{
    /* OPCODE: 0xD2 */
    // XOR: Computes the bitwise XOR of an immediate into the accumulator
    ACC(_ACC ^ read_ram(_CI));
    inc_CI();
    // TODO: update T and V flags
}

void NAND_instruction()
{
    /* OPCODE: 0xD4 */
    // NAND: Computes the bitwise XOR of an immediate into the accumulator
    ACC(~(_ACC & read_ram(_CI)));
    inc_CI();
    // TODO: update T and V flags
}

void SOMI_instruction()
{
    /* OPCODE: 0xD8 */
    // SOMI="Soma Imediato":
    // Add an immediate into the accumulator
    // TODO: set_flag(V, ((((int16_t) ACC) + ((int16_t) READ_BYTE_PATINHO(PC))) >> 8));
    // TODO: set_flag(T, ((((int8_t) (ACC & 0x7F)) + ((int8_t) (READ_BYTE_PATINHO(PC) & 0x7F))) >> 7) == V);
    ACC(_ACC + read_ram(_CI));
    inc_CI();
}

void CARI_instruction()
{
    /* OPCODE: 0xDA */
    // CARI="Carrega Imediato":
    //      Load an immediate into the accumulator
    ACC(read_ram(_CI));
    inc_CI();
}

void IND_instruction()
{
    indirect_addressing = true;
}

void shift_rotate_instructions()
{
    // TODO: Verificar se está pegando o endereço correto, talvez esteja pegando a primeira palavra ao invés da segunda
    unsigned int value = read_ram(_CI);

    // Campo do código dos deslocamentos, 4bits
    unsigned int code = (value & 0xF0) >> 4;

    // Quantidade de "1" especifica o numero de deslocamentos, 4bits
    unsigned int amount = (value & 0x0F);

#if DEBUG
    cout << "Shift/Rotate instruction: " << hex << (int)opcode << " " << (int)code << " " << (int)amount << endl;
#endif

    switch (code)
    {
    case 0b1000:
        // Descola a direita com duplicação do bit de sinal
        // TODO:  Implement-me!
        cout << " NOT IMPLEMENTED YET" << endl;
        break;
    case 0b0000:
        // Desloca a direita
        // TODO:  Implement-me!
        cout << " NOT IMPLEMENTED YET" << endl;
        break;
    case 0b0100:
        // Desloca a esquerda
        ACC(_ACC << amount);
        inc_CI();
        break;
    case 0b0001:
        // Desloca a direita com V
        // TODO:  Implement-me!
        cout << " NOT IMPLEMENTED YET" << endl;
        break;
    case 0b0101:
        // Desloca a esquerda com V
        // TODO:  Implement-me!
        cout << " NOT IMPLEMENTED YET" << endl;
        break;
    case 0b0010:
        // Rotaciona a direita
        ACC((_ACC >> amount) | (_ACC << (8 - amount)));
        inc_CI();
        break;
    case 0b0110:
        // Rotaciona a esquerda
        ACC((_ACC << amount) | (_ACC >> (8 - amount)));
        inc_CI();
        break;
    case 0b0011:
        // Rotaciona a direita com V
        // TODO:  Implement-me!
        cout << " NOT IMPLEMENTED YET" << endl;
        break;
    case 0b0111:
        // Rotaciona a esquerda com V
        // TODO:  Implement-me!
        cout << " NOT IMPLEMENTED YET" << endl;
        break;
    default:
        cout << "Invalid shift/rotate instruction" << endl;
        break;
    }
}
/*
 * ############################## END ##################################
 */

void load_example_hardcoded_program()
{
    /*
      HELLO WORLD PROGRAM that prints
      "PATINHO FEIO" to the teletype:
    */

    RAM[0x06] = 0x80; // 006: 80     LIMPO
    RAM[0x07] = 0x9e; // 007: 9E     TRI
    RAM[0x08] = 0x50; // 008: 50 1C  CARX (IDX) + /01C
    RAM[0x09] = 0x1c;
    RAM[0x0A] = 0xca; // 00A: CA 80  SAI /A0             // output ACC value to the teletype (channel A)
    RAM[0x0B] = 0x80;
    RAM[0x0C] = 0xca; // 00C: CA 21  SAL /A1             // channel A (teletype), function 1 (check READY flag)
    RAM[0x0D] = 0x21;
    RAM[0x0E] = 0x00; // 00E: 00 0C  PLA /00C            // Jump back to previous instruction (unless the teletype READY flag causes this instruction to be skipped).
    RAM[0x0F] = 0x0c;
    RAM[0x10] = 0x9e; // 010: 9E     TRI                 // TRI = Exchange values of IND reg and ACC reg.
    RAM[0x11] = 0x85; // 011: 85     INC                 // Increment ACC
    RAM[0x12] = 0x20; // 012: 20 00  ARM (IDX)           // Store ACC value into the IDX register.
    RAM[0x13] = 0x00;
    RAM[0x14] = 0x60; // 014: 60 1B  SOM /01B            // Subtract the length of the string from the ACC value (see DB -14 below. "SOM" means "add")
    RAM[0x15] = 0x1B;
    RAM[0x16] = 0xA0; // 016: A0 08  PLAN /008           // Conditional jump to /008 (jumps if ACC is negative)
    RAM[0x17] = 0x08;
    RAM[0x18] = 0x9D; // 018: 9D     PARE                // Halt the CPU. Can be restarted by manually pushing the PARTIDA (startup) panel button.
    RAM[0x19] = 0x00; // 019: 00 06  PLA /006            // If you restart the CPU, this is the next instruction, which jumps straight back to the routine entry point, effectively causing the whole program to run once again.
    RAM[0x1A] = 0x06;
    // RECOMEÇA O PROGRAMA

    RAM[0x1B] = 0xEF; // 01B: 0E     DB -14              // This is the 2's complement for -len(string) // RAM[0x1B] = 0xF2; // 01B: F2     DB -14              // This is the 2's complement for -len(string)
    // VALOR A SER DECREMENTADO
    RAM[0x1C] = 'P'; // 01C: "PATINHO FEIO\n"           // This is the string.
    RAM[0x1D] = 'A';
    RAM[0x1E] = 'T';
    RAM[0x1F] = 'I';
    RAM[0x20] = 'N';
    RAM[0x21] = 'H';
    RAM[0x22] = 'O';
    RAM[0x23] = ' ';
    RAM[0x24] = 'F';
    RAM[0x25] = 'E';
    RAM[0x26] = 'I';
    RAM[0x27] = 'O';
    RAM[0x28] = '!';
    RAM[0x29] = '!';
    RAM[0x2A] = '!';
    RAM[0x2B] = 0x0D;
    RAM[0x2C] = 0x0A;
    // STRING

    // Entry point:
    CI(0x06);
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

void espera()
{
    // TODO: Implement-me!
}

void interrupcao()
{
    // TODO: Implement-me!
}

void partida()
{
    switch (_MODO)
    {
    case NORMAL:
    case CICLO_UNICO:
    case INSTRUCAO_UNICA:
        PARADO(false);
        break;

    case ENDERECAMENTO:
        RE(_DADOS_DO_PAINEL);
        break;

    case ARMAZENAMENTO:
        if (!(memoria_protegida && _RE >= 0xF80))
        {
            uint8_t dado = _DADOS_DO_PAINEL & 0xFF;
            write_ram(_RE, dado);
            RD(dado);
        }

        if (enderecamento_sequencial)
            RE(_RE + 1);
        break;

    case EXPOSICAO:
        RD(read_ram(_RE));
        if (enderecamento_sequencial)
            RE(_RE + 1);
        break;
    }
    // wait_for_buttons_release();
}

void run_one_instruction()
{
    opcode = read_ram(_CI);
    old_CI = _CI;
    RI(opcode); // para mostrar o opcode no painel

    if (scheduled_IND_bit_reset)
        indirect_addressing = false;

    if (indirect_addressing)
        scheduled_IND_bit_reset = true;

        // DEBUG
        // printf("CI: %04x OPCODE: %04x, Mascarado: %04x \n", _CI, opcode, opcode & 0xF0);

#if DEBUG
    printf("\n");
    printf("Executing instruction: %s\n", Debug_get_mnemonic(opcode));
    printf("CI: %04x OPCODE: %02x, Mascarado: %02x\n", old_CI, opcode, opcode & 0xF0);
#endif
    inc_CI();

    switch (opcode)
    {
    case 0x80:
        LIMPO_instruction();
        return;
    case 0x81:
        UM_instruction();
        return;
    case 0x82:
        CMP1_instruction();
        return;
    case 0x83:
        CMP2_instruction();
        return;
    case 0x84:
        LIM_instruction();
        return;
    case 0x85:
        INC_instruction();
        return;
    case 0x86:
        UNEG_instruction();
        return;
    case 0x87:
        LIMP1_instruction();
        return;
    case 0x88:
        PNL_0_instruction();
        return;
    case 0x89:
        PNL_1_instruction();
        return;
    case 0x8A:
        PNL_2_instruction();
        return;
    case 0x8B:
        PNL_3_instruction();
        return;
    case 0x8C:
        PNL_4_instruction();
        return;
    case 0x8D:
        PNL_5_instruction();
        return;
    case 0x8E:
        PNL_6_instruction();
        return;
    case 0x8F:
        PNL_7_instruction();
        return;
    case 0x90:
        ST_0_instruction();
        return;
    case 0x91:
        STM_0_instruction();
        return;
    case 0x92:
        ST_1_instruction();
        return;
    case 0x93:
        STM_1_instruction();
        return;
    case 0x94:
        SV_0_instruction();
        return;
    case 0x95:
        SVM_0_instruction();
        return;
    case 0x96:
        SV_1_instruction();
        return;
    case 0x97:
        SVM_1_instruction();
        return;
    case 0x98:
        PUL_instruction();
        return;
    case 0x99:
        TRE_instruction();
        return;
    case 0x9A:
        INIB_instruction();
        return;
    case 0x9B:
        PERM_instruction();
        return;
    case 0x9C:
        ESP_instruction();
        return;
    case 0x9D:
        PARE_instruction();
        return;
    case 0x9E:
        TRI_instruction();
        return;
    case 0x9F:
        IND_instruction();
        return;
    case 0xD1:
        shift_rotate_instructions();
        return;
    case 0xD2:
        XOR_instruction();
        return;
    case 0xD4:
        NAND_instruction();
        return;
    case 0xD8:
        SOMI_instruction();
        return;
    case 0xDA:
        CARI_instruction();
        return;
    }

    switch (opcode & 0xF0)
    {
    case 0x00:
        PLA_instruction();
        return;
    case 0x10:
        PLAX_instruction();
        return;
    case 0x20:
        ARM_instruction();
        return;
    case 0x30:
        ARMX_instruction();
        return;
    case 0x40:
        CAR_instruction();
        return;
    case 0x50:
        CARX_instruction();
        return;
    case 0x60:
        SOM_instruction();
        return;
    case 0x70:
        SOMX_instruction();
        return;
    // case 0x80: single byte instructions
    // case 0x90:     declared above
    case 0xA0:
        PLAN_instruction();
        return;
    case 0xB0:
        PLAZ_instruction();
        return;
    case 0xC0:
        IO_instructions();
        return;
    // case 0xD0: shift/rotate, XOR, NAND,
    //            SOMI, CARI instructions above
    case 0xE0:
        SUS_instruction();
        return;
    case 0xF0:
        PUG_instruction();
        return;
    default:
        PARADO(true);
        return;
    }
}

void emulator_loop()
{

    if (!_PARADO)
    {
        run_one_instruction();
    }

    if (_MODO == CICLO_UNICO || _MODO == INSTRUCAO_UNICA)
    {
        PARADO(true);
    }
}

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
            dados |= 1 << col;
        }
    }
    DADOS_DO_PAINEL(dados);
    // Print HEX value of dados to the printer
    // printf("dados: %04x\n", dados);
    // Print Binary value of dados to the printer
    // printf("dados: %s\n", std::bitset<12>(dados).to_string().c_str());

    // when a mode button is pressed, set the corresponding mode:
    for (col = 0; col < 6; col++)
    {
        if (btn_mode[col].state)
        {
            MODO(col + 1);
        }
    }

    if (buttons[0].state)
    {

        enderecamento_sequencial = !enderecamento_sequencial;
        buttons[0].glowing = enderecamento_sequencial;
    }

    if (buttons[1].state)
    {
        memoria_protegida = !memoria_protegida;
        buttons[1].glowing = memoria_protegida;
    }

    // chaves de modos de memória:
    // enderecamento_sequencial = buttons[0].state;
    // memoria_protegida = buttons[1].state;

    // // botão "PREPARAÇÂO":
    if (buttons[5].state)
    {
        reset_CPU();
    }

    // // botão "ESPERA":
    if (buttons[2].state)
    {
        espera();
    }

    // // botão "INTERRUPÇÂO"
    if (buttons[3].state)
    {
        interrupcao();
    }

    // // botão "PARTIDA":
    if (buttons[4].state)
    {
        partida();
    }
}

void Machine_setup()
{
    for (int c = 0; c < 16; c++)
    {
        canais[c] = NULL;
    }
    canais[0x0] = nullptr;        // Painel
    canais[0x5] = nullptr;        // Impressora HP-2607A
    canais[0x6] = nullptr;        // 8bits duplex
    canais[0x7] = nullptr;        // 8bits duplex
    canais[0x8] = nullptr;        // Perfuradora rapida de Fita de papel
    canais[0x9] = nullptr;        // Leitora de Cartões
    canais[0xA] = new TeleType(); // DECWriter III (Digital Equipment Corp.)
    canais[0xB] = new TeleType(); // TTY (Teletype da TeleType corp.)
    canais[0xE] = nullptr;        // Leitora de Fita de papel

    initial_printer_commands();

    for (int i = 0; i < RAM_SIZE; i++)
    {
        RAM[i] = 0;
    }
    reset_CPU();
    // load_example_hardcoded_program();

    //     // DUMP RAM in file
    //     FILE *f = fopen("ram.bin", "wb");
    //     fwrite(RAM, 1, RAM_SIZE, f);
    //     fclose(f);

    // Load Program into RAM
    // FILE *f = fopen("bin/hello.bin", "rb");
    FILE *f = fopen("bin/prog.bin", "rb");
    if (f == NULL)
    {
        printf("Error: Could not open file\n");
        return;
    }

    unsigned char buffer[RAM_SIZE];
    for (int i = 0; i < RAM_SIZE; i++)
    {
        buffer[i] = 0;
    }

    fread(buffer, 1, RAM_SIZE, f);
    fclose(f);

    // int offset = 0x03;
    int offset = 0xE00;
    for (int i = offset; i < RAM_SIZE; i++)
    {
        RAM[i] = buffer[i - offset];
    }

    CI(0xE00);
    // CI(0x06);
}

void Machine_ControlLoop()
{
    while (running)
    {
        read_inputs();
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

void Machine_EmulationLoop()
{
    while (running)
    {
        emulator_loop();
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        // std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}
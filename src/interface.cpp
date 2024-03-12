#include "interface.h"
#include <iostream>

using namespace std;

InterfaceIO *canais[16];

// IO
InterfaceIO::InterfaceIO()
{
    this->registrador = 0x00;
    this->controle = false;
    this->estado = READY;
    this->pedido_de_interrupcao = false;
    this->permite_interrupcao = false;
}

void InterfaceIO::write(uint8_t value)
{
    this->registrador = value;
}

uint8_t InterfaceIO::read()
{
    return this->registrador;
}

TeleType::TeleType()
{
}

void TeleType::write(uint8_t value)
{
    this->registrador = value;

    if (value == '\n')
    {
        printer_writeByte('\r');
    }

    printer_writeByte(value);
}

uint8_t TeleType::read()
{
    int temp;
    unsigned char c;
    c = '1';

    // Calcula paridade de 1 em c
    c = c | 0x80;

    // Complemento de char

    temp = ~c;
    return temp;
}

/*
 * ###################### PRINTING TO THE TELETYPE #####################
 */
void printer_writeByte(char c)
{
    cout << c;
}

#define INIT_PRINTER_DEFAULTS '@'
#define BOLD 'E'
#define LEFT_MARGIN 'l'
#define RIGHT_MARGIN 'Q'

void escape_code(char code)
{
    printer_writeByte(27);
    printer_writeByte(code);
}

void set_immediate_print_mode()
{
    escape_code('i');
    printer_writeByte(1);
}

void left_margin(char n_chars)
{
    escape_code(LEFT_MARGIN);
    printer_writeByte(n_chars);
}

void right_margin(char n_chars)
{
    escape_code(RIGHT_MARGIN);
    printer_writeByte(n_chars);
}

void printer_writeString(char *str)
{
    for (char *ptr = str; *ptr; ptr++)
    {
        printer_writeByte(*ptr);
    }
}
/*
 * ################################ END ##################################
 */

void initial_printer_commands()
{
    escape_code(INIT_PRINTER_DEFAULTS);

    // set unit of line spacing to the minimum vertical increment necessary
    //...

    // set the printing area
    left_margin(5);
    right_margin(5);

    // select the font
    escape_code(BOLD);

    // set the printing position
    //...

    // send one line's print data + CR + LF
    printer_writeString("IMPRESSORA OK\r\n");

    // end page with FF
    //... escape_code(FORM_FEED);

    // end printing with '@'
    // escape_code(INIT_PRINTER_DEFAULTS);
}

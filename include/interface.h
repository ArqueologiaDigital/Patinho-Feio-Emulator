#pragma once

#include "stdint.h"

#define READY true
#define BUSY false

void initial_printer_commands();
void printer_writeByte(char c);

class InterfaceIO
{
private:
    bool controle;
    bool estado;
    bool pedido_de_interrupcao;
    bool permite_interrupcao;

public:
    InterfaceIO();
    virtual void write(uint8_t value);
    virtual uint8_t read();
    uint8_t registrador;
};

class TeleType : public InterfaceIO
{
public:
    TeleType();
    void write(uint8_t value);
    uint8_t read();
};

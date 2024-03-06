// Pragma once to avoid multiple inclusion
#pragma once
class Interface
{
private:
    bool controle;
    bool estado;
    bool pedido_de_interrupcao;
    bool permite_interrupcao;

public:
    Interface();
    virtual void write(uint8_t value);
    uint8_t read();

    uint8_t registrador;
};

#define READY true
#define BUSY false

class TeleType : public Interface
{
public:
    TeleType();
    void write(uint8_t value);
};

void Machine_setup();
void Machine_loop();
void read_inputs();
void printer_writeByte(char c);
void emulator_loop();
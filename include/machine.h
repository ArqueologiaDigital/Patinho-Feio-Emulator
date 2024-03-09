// Pragma once to avoid multiple inclusion
#pragma once

#define INDEX_REG 0
#define EXTENSION_REG 1
#define RAM_SIZE 256
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
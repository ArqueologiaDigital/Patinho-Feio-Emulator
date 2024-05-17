# Patinho Feio Tools

🚧 Projeto em construção 🚧

Este é um emulador / Dev tools feito para o patinho feio.
A intenção é você poder executar, debugar e criar programas para o patinho feio.

## CONTROLES DE FLUXOS

| CODINOME        | DESCRICAO                                                                        | LUGAR NO ARRAY |
| --------------- | -------------------------------------------------------------------------------- | -------------- |
| RE              | 12-bit "Registrador de Endereço" ou "Endereço da memoria" = Address Register     |                |
| CI              | 12-bit "Contador de Instrução" ou "Endereço da instrucao" = Instruction Counter  |                |
| DADOS_DO_PAINEL | 12-bit "Dados do Painel" = data provided by the user via panel toggle-switches   |                |
| RD              | 8-bit "Registrador de Dados" ou "Dados da Memoria" = Data Register               |                |
| RI              | 8-bit "Registrador de Instrução" ou "Codigo da instrução" = Instruction Register |                |
| ACC             | 8-bit "Acumulador" = Accumulator Register                                        |                |

## INSTRUÇÕES

| MNEMÔNICO | ENDEREÇO | ARGUMENTOS | DESCRIÇÃO                                  | COMENTÁRIO                   | REFERENCIA |
| --------- | -------- | :--------: | ------------------------------------------ | ---------------------------- | ---------- |
| LIMPO     | 0x80     |     0      | Limpa o AC (acumulador) e faz T = Operando |                              | 1          |
| TRI       | 0x9E     |     0      | Troca com índice                           |                              | 1          |
| CARX      | 0x50     |     1      | Carrega o índice                           |                              | 1          |
| SAI       | 0xCA     |     1      | Saida de dados p/ dispositivo N            | Usa o argumento 0x60         | ?          |
| SAL       | 0xCA     |     1      | Salto tipo "I" p/dispositivo N             | Usa o argumento 0x21         | ?          |
| PLA       | 0x00     |     1      | Pulo incondicional                         |                              | 1          |
| INC       | 0x85     |     0      | Incrementa o AC (acumulador)               |                              | 1          |
| ARM       | 0x20     |     1      | Armazena                                   |                              | 1          |
| SOM       | 0x60     |     1      | Soma                                       |                              | 1          |
| PLAN      | 0xA0     |     1      | Pula se negativo                           |                              | 1          |
| PARE      | 0x9D     |     0      | Pare                                       | Parece ser um halt           | 1          |
| TRE       | 0x99     |     1      | Troca com o acumulador com extensão        |                              | 1          |
| INIB      | 0x9A     |     0      | Inibe interrupção                          |                              | 1          |
| PUG       | 0xF0     |     1      | Pula e guarda                              |                              | 2          |
| CARI      | OxDA     |     1      | Carrega imediato                           | Adiciona valor ao acumulador | 2          |
| CMP1      | 0x82     |     0      | Completa 1 no acumulador e limpa V         |                              | 2          |

| SH/RT

### REFERENCIAS

1. Pode ser encontrada as informações na pagina 178-179 do livro "Projeto de minicomputador digital"
2. Pode ser encontrada as informações na pagina 172 do livro "Projeto de minicomputador digital"

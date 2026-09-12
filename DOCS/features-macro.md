# Macro de agua

Este modulo concentra a entrada de eventos e o ownership dos controladores da
macro de agua. Ele nao altera IDs de item, coordenadas, opcodes ou formatos de
packet herdados.

- `MacroMsg` recebe a intencao originada em `TMFieldScene` e seleciona o nivel.
- `MacroFactory` cria e conserva uma unica estrategia para cada nivel valido.
- `MacroLevel` ainda permanece na raiz porque contem o envio de packets e
  depende de varias fachadas monoliticas. Sua migracao exige um lote proprio,
  com verificacao dos contratos client/server envolvidos.

Os ponteiros retornados por `MacroFactory` sao emprestados. A factory e a unica
proprietaria das estrategias e as libera no encerramento do Singleton.

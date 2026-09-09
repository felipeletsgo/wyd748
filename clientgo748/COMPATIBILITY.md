# Matriz de compatibilidade do client Go 7.48

Esta matriz mede a entrega por capacidade observável, não a quantidade de
funções nativas catalogadas. `Research` é a evidência nativa/Ghidra;
`Contract` é o wire, recurso ou lifecycle decidido; `Go` é a implementação;
`Tests` são testes automatizados; `Client` exige execução no executável Go.

| Capacidade | Research | Contract | Go | Tests | Client |
| --- | --- | --- | --- | --- | --- |
| Janela, WGL e teardown | CLIENT_TESTED | CONTRACT | IMPLEMENTED | AUTOMATED_TESTED | CLIENT_TESTED |
| Assets WYT e pacote protegido | CONTRACT | CONTRACT | IMPLEMENTED | AUTOMATED_TESTED | CLIENT_TESTED |
| Framing e sessão | CONTRACT | CONTRACT | IMPLEMENTED | AUTOMATED_TESTED | PENDENTE |
| Login visual | LOCATED | PENDENTE | PENDENTE | PENDENTE | PENDENTE |
| Lista de personagens | CONTRACT | CONTRACT | IMPLEMENTED | AUTOMATED_TESTED | PENDENTE |
| Entrada no mundo | CONTRACT | CONTRACT | PENDENTE | PENDENTE | PENDENTE |
| Mundo diagnóstico | PENDENTE | PENDENTE | PENDENTE | PENDENTE | PENDENTE |
| Movimento | CONTRACT estreito | PENDENTE | PENDENTE | PENDENTE | PENDENTE |
| Combate e skills | UNMAPPED | PENDENTE | PENDENTE | PENDENTE | PENDENTE |
| Inventário e itens | UNMAPPED | PENDENTE | PENDENTE | PENDENTE | PENDENTE |
| Teleporte | CONTRACT estreito | PENDENTE | PENDENTE | PENDENTE | PENDENTE |

`PENDENTE` não significa falha; significa que ainda não há evidência
suficiente para declarar a coluna concluída. A matriz deve ser atualizada junto
com cada ficha de fluxo, sem transformar um build verde em `CLIENT_TESTED`.

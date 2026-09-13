# Corpus de packets 7.48

O corpus canônico compartilhado fica em `../../testdata/protocol/`. Ele contém
somente frames plaintext cuja origem e layout foram confirmados para a
fronteira exercida. Consumidores removem comentários e espaços antes de
decodificar o hexadecimal.

- `send_item_0x182_24.hex`: frame `MSG_SendItem` de 24 bytes, validado contra o
  contrato nativo 7.48 e consumido pelos testes Go e C++ para impedir drift.

O corpus de captura de movimento que ainda possui consumidor exclusivo do
servidor permanece em `wydgo748/internal/game/testdata/packets/` até a migração
do consumidor. Não duplicar o mesmo fixture nas duas sources.

Não adicionar dumps vindos depois de uma bridge `PacketProtocolV754` como se
fossem wire bruto do client. Em particular, `MSG_AttackOne` nativo e
`p39D_754` usam offsets diferentes em algumas árvores de referência. Uma nova
captura de combate só entra aqui depois de ser comparada com o construtor do
`WYD.exe` atualmente distribuído.

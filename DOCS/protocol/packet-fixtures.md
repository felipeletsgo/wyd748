# Corpus de packets 7.48

Este diretório contém somente capturas plaintext cuja origem e layout foram
confirmados para a fronteira que o teste exerce. O teste remove comentários e
espaços antes de decodificar o hexadecimal.

- `move_0x366_52.hex`: captura real do client 7.48; trava o eixo Y e o término
  da rota `32`.

Não adicionar dumps vindos depois de uma bridge `PacketProtocolV754` como se
fossem wire bruto do client. Em particular, `MSG_AttackOne` nativo e
`p39D_754` usam offsets diferentes em algumas árvores de referência. Uma nova
captura de combate só entra aqui depois de ser comparada com o construtor do
`WYD.exe` atualmente distribuído.

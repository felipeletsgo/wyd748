# Celestial: Spirit's Seal e extração

Implementação do fluxo nativo da 7.48/7.54, portado do W2PP sem copiar
layouts de memória.

## Fluxo

- O item `3443` (`Spirit's_Seal`) só pode ser usado por Celestial/SubCelestial.
- O selo precisa estar vazio (`EF_VOLATILE=206` e `EF[0].cEffect=0`).
- O servidor gera um ID de 16 bits, remove uma unidade do item, grava o snapshot
  da ficha em `Account.CelestialCapsules` e coloca no Cargo um novo 3443 com
  `EF[0]=59`, `EF[1]=ID alto`, `EF[2]=59`, `EF[3]=ID baixo`.
- A conta, o Cargo e a remoção do charstate do `CharacterUID` são confirmados
  numa única operação PostgreSQL (`SavePlayerState`). Em falha nada é consumido.
- Depois do commit, o jogador sai do mundo e recebe novamente a seleção de
  personagens; não existe cadáver, clone ou personagem ativo duplicado.

## Consulta e extração

- Ao passar o cursor no selo, o client envia `0x2CD` (16 bytes). O servidor
  confere o ID contra o item e responde `0xDC3` com o `MSG_CAPSULEINFO` compacto
  de 52 bytes que o `WYD.exe` 7.48 realmente recebe. O `0xD1F` de 76 bytes das
  fontes posteriores é interno ao DB/TMSrv e não possui handler neste client.
- Para extrair, o client envia `0x3CC` (52 bytes), com origem no Carry/Cargo e
  nome ASCII. O servidor valida vida, alcance lógico do mapa, nome, UID do item,
  snapshot e slot livre antes de mutar.
- A ficha criada recebe novo `CharacterUID`, nasce em `(2100,2100)`, preserva
  rosto/capa e a progressão Celestial, mas limpa equipamento restante, Carry,
  Gold e Guild como o `DBOutCapsule` nativo. O selo e o snapshot são removidos
  somente após a gravação da conta.
- O selo preenchido pode circular entre o Carry visível e o Cargo e pode ser
  negociado diretamente com outro jogador. No commit do trade, item e snapshot
  Celestial mudam de conta na mesma transação; se o ID local colidir no destino,
  o servidor remapeia o ID e atualiza o selo antes de confirmar aos clientes.
- Operações que destruiriam ou separariam o item do snapshot continuam
  bloqueadas: drop, venda a NPC, exclusão, `/limparinv` e anúncio em Loja
  Fantasma. Extração direta do Cargo exige proximidade de um Cargo Keeper.

## Limites de fonte

`3455` (`Extraction_of_Magical_Power`) compartilha o código 206 no itemlist,
mas não é aceito pelo `_MSG_PutoutSeal.cpp` do W2PP. Por isso permanece
`no_direct_use`: sua utilização direta não consome nem cria estado inventado.

## Testes

`internal/game/celestial_capsule_test.go` cobre encapsulamento, vínculo
UID/item, rollback lógico, projeção 7.48, crescimento do slice de personagens,
trade atômico, colisão de ID, rollback do trade e extração em slot reutilizado.
Os testes de modelo rejeitam selo órfão/duplicado e aceitam a movimentação
Carry/Cargo.
Ainda é necessária a validação visual no client 7.48 (hover do selo, tela de
seleção e reentrada da ficha).

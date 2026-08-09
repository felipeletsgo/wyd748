# Contratos confirmados do emulador WYD-Go

## Indice

- Arquitetura e ownership
- Fronteiras de protocolo 7.48
- NPCs, lojas e crafting
- IA, movimento e mundo
- Personagem, skills e combate
- Inventario e persistencia
- Validacao operacional

## Arquitetura e ownership

- `model`: estado puro; `wire`: bytes; `net`: transporte; `store`: persistencia; `data`: loaders; `game`: coordenacao.
- Uma goroutine de `World` altera gameplay. Sessoes apenas entregam comandos e enviam pela propria fila.
- Separar features por responsabilidade. Handlers decodificam e roteiam; servicos de dominio validam e mutam.
- `ExtendedScore` e autoritativo; o score WORD legado nasce apenas na borda do protocolo.

## Fronteiras de protocolo 7.48

- Header tem 12 bytes. Finalizar/cifrar uma copia nova por envio; nunca reutilizar buffer mutado em broadcast.
- Rejeitar frame fora de 12..8192 bytes e checksum invalido ainda no socket. No game loop, exigir `Header.Size == len(pkt)`, tamanho exato dos layouts confirmados e opcode compativel com a fase da sessao.
- Cliente e apenas intencao: pacote nunca autoriza dano, critico, coordenada, slot, preco, skill aprendida ou transicao de login.
- Limitar flood por conexao e contabilizar recusas com log rate-limited; desconectar reincidencia sem permitir que o proprio log vire DoS.
- Confirmar no source do client as acoes cuja UI decide o fluxo antes de enviar pacote.
- `0x213 CharacterLogin` do client 7.48 tem 36 bytes: header(12), `Slot@12`,
  `Force@16` e `SecretCode[16]@20`. Nao reduzir o tamanho ao prefixo de 16 bytes
  apenas porque o emulador atualmente consome somente o slot.
- `0x277 ApplyBonus` tem 20 bytes: `BonusType@12`, `Detail@14` e
  `TargetID@16`. O alvo e indispensavel no fluxo tipo 2 da loja de skills; 18
  bytes e apenas a soma sem considerar o tamanho/alinhamento real do pacote.
- `0x27B` (16 B) pede loja; `0x28B` (20 B) e clique/quest com `ClickOk@16`.
- Interacao exige NPC existente, visivel e Chebyshev <= 8.
- Movimento do player chega em `0x366`; seu `Route[24]` e uma rota planejada
  completa e pode ser repetida enquanto o client interpola. Nao aplicar ao
  player o limite `2*Speed` dos segmentos gerados para mobs; limitar speedhack
  pelo budget temporal server-side, rota, terreno e altura.
- Visibilidade e bidirecional: create/remove, movimento, HP, morte, respawn e affects atualizam observers.
- `CreateMob` materializa uma entidade ao entrar na visao e carrega posicao. Mudanca visual de player ja visivel usa `0x36B UpdateEquip` de 60 bytes (`ItemEff[16]@12` + `AnctCode[16]@44`) para dono e observadores; os 16 bytes finais sao indispensaveis para tintura. Recriar durante movimento causa snap/teleporte.

## NPCs, lojas e crafting

- Merchant fica no nibble baixo do score enviado. `Merchant != 0` identifica funcao de NPC e nunca combate.
- Skill master usa ShopType 3; loja comum usa ShopType 1. O client usa 24 skills uteis nos 27 slots da grade.
- Algumas janelas de craft abrem inteiramente no client e nao enviam `0x28B`. Quando o opcode de composicao chegar, resolver o artesao esperado, visivel e proximo; nao depender somente de `Player.CraftNPC`.
- Substituir contexto antigo de outro artesao pelo esperado no grid local.
- Seletor client-side: Aylin head 55 e Agatha head 56 abrem em qualquer regiao; Tiny head 68 e fallback global; Ehre head 68 exige chunk `(19,15)` para o modo correto; Lindy head 67 exige `(13,13)`; Compositor head 54 exige `(19,13)`; Odin head 67 + Merchant 8 exige `(25,13)`.
- Enviar mensagem textual e depois `0x3A7`: `0` invalida, `1` sucesso, `2` falha.
- Textos adotados: `Combinacao incorreta.`, `Processamento concluido.`, `Falha na composicao.`
- Fluxo transacional: validar snapshot/receita, copiar estado, mutar, persistir, sincronizar itens/score, enviar mensagem/resultado. Restaurar todos os campos em erro.

## IA, movimento e mundo

- Movimento de player valida origem/destino, bytes de rota, terreno, altura e um budget temporal derivado do RunSpeed server-side. `ActionStop`/`MoveStop` nao podem corrigir posicao alem de poucos tiles.
- Ataque atraves de terreno bloqueado pode ser recusado por linha transitavel como hardening adicional do emulador; documentar que essa regra nao e universal nas sources nativas.
- Mobs fora do raio de ativacao dormem. Ativos adquirem alvo em raio 4, retem ate 7 e respeitam leash 16.
- Percepcao pode rodar a 1 s; deadline de ataque controla DPS separadamente.
- Todo caminho de ataque revalida `Def.IsMonster()`. `activeMobs` expulsa NPC, merchant, morto ou entidade invalida.
- Movimento nativo transmite rota de ate `2*Speed`; speed de mob limitado a 4. Action por coordenada causa teleportes e reinicia passos.
- HeightMap e `int8`: caminhada exige delta `<8`; reposicionamento de spawn aceita `<50`.
- Usar grid espacial para wake/sleep, proximidade, visibilidade e interacao. Nao varrer todos os geradores em ticks.

## Personagem, skills e combate

- Recalcular `ExtendedRuntime` depois de equipar, remover, dropar, refinar, buffar ou alterar stat.
- Dano, HP/MP e stats wide permanecem `uint32`; projecoes legadas nunca voltam ao calculo.
- Skill valida aprendizado, mastery, mana, cooldown, alvo, alcance e tipo antes de mutar mana.
- Affect usa lifecycle central: aplicar/substituir, recalcular, publicar, tickar, expirar e persistir quando necessario.
- Floating damage e separado do dano aplicado. Preservar o hit calculado mesmo quando supera o HP restante.
- Summons ficam fora da party e seguem ownership explicito.

## Inventario e persistencia

- Inventario: 64 slots no ABI e 63 utilizaveis. Cargo: 128 no ABI e 120 utilizaveis.
- Nunca aceitar preco, item, efeito ou slot do client sem comparar com estado autoritativo.
- Compra/venda revalidam NPC, merchant, visibilidade e distancia a cada pacote. Venda comum nao aceita Equip/Face; drop so e publicado depois de persistir a remocao do inventario.
- Autosave de 3 s reduz rollback; economia, craft e trade exigem save sincrono anti-dupe.
- Trade entre duas contas exige persistencia multi-conta atomica; nunca degradar silenciosamente para dois saves sequenciais.
- Save atomico usa temporario, sync e rename. Transacoes multi-arquivo usam journal tipado.
- Posicao atual nao define reentrada; personagens entram no ponto configurado.

## Validacao operacional

Executar na raiz `wyd-go`:

```powershell
$env:GOCACHE = Join-Path (Get-Location) '.gocache'
go test ./...
go vet ./...
go build -o tm-check.exe ./cmd/server
git diff --check
```

Remover `.gocache` e `tm-check.exe` depois. Nao apagar artefatos do usuario ou limpar mudancas alheias.

Quando uma acao nao fizer nada, registrar opcode, tamanho e campos essenciais, reproduzir uma vez e comparar os handlers do client e W2PP. Logs devem explicar recusas sem despejar structs nem gerar flood por tick.

---
id: field-scene-rebuild-after-server-move
title: Rebuild FieldScene after server or channel migration
subsystem: lifecycle
status: CONTRACT
native_sha256: 8AA2F918844BCE3AFE21F1204F69757A443E32EB2F2F616936B1D9BFE215F593
updated: 2026-08-31
---

# Rebuild FieldScene after server or channel migration

## Pergunta

Como o client nativo 7.48 preserva uma confirmação de remoção/migração de
servidor, reconecta, refaz o login e força a reconstrução da `FieldScene` sem
reutilizar um ticket obsoleto?

## Fronteira de evidência

- Executável/hash: `client748/wyd.exe nativo+patches/WYD.exe`, SHA-256 do
  frontmatter.
- Projeto/corpus Ghidra: `WYD748Native_20260821.gpr`; exports focados
  `field-scene-rebuild-server-move-focused.tsv` e
  `objectmanager-slots-14-20-traversals-focused.tsv`.
- Assets: N/A; a transição não seleciona recurso visual novo.
- Source atual: `Basedef.h`, `TMScene.cpp`, `TMFieldScene.cpp` e
  `ObjectManager.cpp` em `client-source/tmproject/Projects/TMProject/`.
- Servidor: a busca no WYD-Go não localizou implementação de `0x52A` ou
  `RemoveServer`; esta ficha não projeta nem autoriza um packet server-side
  novo.

## Fluxo nativo 7.48

### Entrada observável

- Evento: a `FieldScene` ativa recebe opcode `0x52A` para o personagem atual,
  seguido pela indisponibilidade/fechamento do socket durante a migração.
- Precondições e estado inicial: cena Field ativa, ID do packet igual ao ID do
  personagem e imagem wire de `0x50` bytes; o callback de desconexão só aceita
  replay dentro de 15.000 ms.
- Saída observável: o client reconecta e envia login 7.48 imediatamente quando
  o socket já está fechado, ou preserva o packet e o reproduz uma vez no
  callback de desconexão; a confirmação posterior força uma nova `FieldScene`.

### Callers

- `FUN_00492E7D:0x00493037 -> FUN_00484D44` é o dispatch concreto do packet na
  cena Field.
- `FUN_0049889A` é a entrada observada do callback de desconexão que consome o
  replay pendente.
- `FUN_004B3500` recebe o estado sintético `9` no fim da migração e executa a
  criação genérica da nova cena.

### Função principal

`FUN_00484D44` valida o personagem ativo. Com socket fechado, monta e envia o
login opcode `0x20D`, de `0x74` bytes, com versão `0x2EC` (748). Com socket
aberto, copia exatamente `0x50` bytes para `scene+0x28BBC` e grava
`scene+0x26E18 = 1`, adiando o mesmo packet até a desconexão.

### Callees

- `FUN_0049889A` testa a janela `0x3A98` (15.000 ms), o replay pendente e o
  tipo Field `0x7530`; somente nessa cena chama o slot virtual `+0x04` com
  opcode `0x52A` e o packet preservado.
- `FUN_004B3500` normaliza `9 -> 0`, contorna a igualdade com o estado Field
  corrente e percorre o caminho já documentado de construção, inicialização e
  anexação da `FieldScene`.
- `FUN_004B29B9` e `FUN_004B2D35` fecham os traversals adjacentes do
  `ObjectManager`; suas semânticas estão registradas na ficha geral de cenas e
  não alteram o wire desta transição.

### Saídas e erros

- Packet de outro personagem é rejeitado antes de mutar o estado de migração.
- Fora da janela de 15 segundos, ou sem replay pendente, não há reprodução.
- Em cena diferente de Field, o packet não é entregue a outro override; o
  timestamp e a flag pendente são consumidos para impedir replay tardio.
- Falha de conexão/login permanece no fluxo normal de mensagens e seleção de
  servidor da source; esta ficha não redefine política server-side.

## Estado e lifecycle

### Matriz de transições

| Evento/estado | Precondição | Função/call | Estado resultante | Side effects | Erro/saída |
| --- | --- | --- | --- | --- | --- |
| `0x52A`, socket fechado | ID ativo | `FUN_00484D44` | login de migração em andamento | envia `0x20D/0x74`, versão 748 | rejeita ID divergente |
| `0x52A`, socket aberto | ID ativo | `FUN_00484D44` | replay pendente | copia `0x50`; seta `+0x26E18` | rejeita ID divergente |
| socket fecha em até 15 s | replay pendente; Field | `FUN_0049889A -> slot +0x04` | packet reaplicado uma vez | entrega `0x52A`; limpa tempo/flag | fora de Field apenas consome |
| confirmação de login/migração | estado sintético `9` | `FUN_004B3500` | estado `0`, nova Field | cria, inicializa e anexa cena | usa falha padrão de initializer |

### Vtables, vptrs e receptores

O replay usa o receptor da própria cena e seu slot de packet `+0x04`. O guard
de tipo `0x7530` impede cast/dispatch de Field sobre outra classe. A
reconstrução usa o `ObjectManager` de vptr efetivo `0x005A45FC`, cujo slot
`+0x54` é `FUN_004B3500`.

### Ownership

A imagem pendente pertence à cena em `scene+0x28BBC`; timestamp e flag ficam na
mesma instância (`+0x26E14/+0x26E18`). O packet é copiado, não retém ponteiro do
buffer de transporte. A nova cena segue o ownership da raiz do
`ObjectManager`, documentado em `scene-transition.md`.

### Falha parcial

O estado só fica pendente depois da cópia completa de `0x50` bytes. O callback
consome timestamp e flag mesmo quando a cena já não é Field, evitando que uma
instância futura reutilize credenciais/ticket da migração anterior.

### Cleanup e teardown

O replay é one-shot: `FUN_0049889A` zera `scene+0x26E14` e
`scene+0x26E18`. A cena anterior segue a marca, coleta e deleting destructor já
fechados na ficha geral; nenhum owner novo é introduzido por este contrato.

### Shutdown

N/A: o contrato termina na reconstrução pós-migração e não reivindica o
shutdown global da aplicação. Fechamento de janela/socket fora dessa transição
continua na ficha geral de lifecycle.

### Logout e relogin

O relogin específico da migração está coberto pelo `0x20D/0x74`, versão 748, e
pela reconstrução `9 -> 0`. N/A para logout explícito e para um ciclo completo
de saída/entrada de conta, que permanecem como frente separada.

## Wire, ABI e recursos

- `MSG_CNFRemoveServer`: direção servidor -> client, opcode `0x52A`, tamanho
  `0x50`, `AccountName` em `+0x0C` e `TID` em `+0x1C`.
- Login emitido após a migração: client -> servidor, opcode `0x20D`, tamanho
  `0x74`, `ClientVersion = 0x2EC`/748.
- A source fixa tamanho e offsets de `MSG_CNFRemoveServer` com `static_assert`.
- Packing, IDs de personagem e buffer completo são preservados; não se copia
  layout de TMProject posterior nem se cria recurso/asset.

## Mapeamento atual

### Source recompilável

- `Basedef.h` fixa tamanho e offsets do packet 7.48.
- `TMFieldScene::OnPacketEvent` deixa o handler base observar desconexão mesmo
  com buffer nulo e roteia `0x52A` para `OnPacketCNFRemoveServer`.
- `TMFieldScene::OnPacketCNFRemoveServer` valida o ID, reconecta/login quando o
  socket está fechado ou preserva a imagem wire completa quando está aberto.
- `TMScene::OnPacketEvent` aplica a janela de 15 segundos, restringe o replay a
  Field e sempre consome o estado pendente.
- `ObjectManager::SetCurrentState` converte `TM_FIELD2_STATE` para
  `TM_FIELD_STATE` antes do equality guard, forçando nova `TMFieldScene`.

### WYD-Go

Nenhum handler `0x52A`/`RemoveServer` foi localizado. A decisão é preservar o
contrato nativo no client e não inventar protocolo no Go sem uma transição
server-side observável e testável.

## Matriz de delta

| Claim | Nativo 7.48 | Source atual | TMProject | WYD-Go | Decisão |
| --- | --- | --- | --- | --- | --- |
| Wire de migração | `0x52A`, `0x50`, offsets comprovados | struct com asserts | layout posterior não é autoridade | handler não localizado | portar contrato 7.48 |
| Socket já fechado | reconecta e envia `0x20D/0x74`, versão 748 | caminho preservado | sem autoridade de ABI | valida login existente | manter adaptado |
| Socket ainda aberto | copia packet completo e adia | `memmove` + flag | sem autoridade de lifecycle | N/A | portar comportamento |
| Callback | 15 s, Field-only, consumo one-shot | mesmo guard e consumo | estrutura posterior compatível | N/A | modernizar internamente sem mudar fronteira |
| Reconstrução | estado sintético `9 -> 0` | `FIELD2 -> FIELD` antes do guard | nome moderno é só pista | N/A | portar sem importar layout |

## Decisões

- Promover somente esta transição estreita a `CONTRACT`; a ficha geral de
  cenas permanece `LOCATED`.
- Manter a estrutura recompilável atual onde ela é mais clara, desde que
  preserve wire, janela, ownership e consumo one-shot comprovados no 7.48.
- Não adicionar handler Go para `0x52A` sem evidência da origem server-side.
- Classificar a entrega como `STATICALLY VERIFIED`; build não equivale a teste
  de migração real nem autoriza `CLIENT_TESTED`.

## Lacunas

- Executar migração real entre canais/servidores no `client748/project.exe` e
  registrar servidor de origem/destino, timestamps e hash do candidato.
- Correlacionar a origem server-side de `0x52A` antes de qualquer alteração do
  protocolo Go.
- Logout explícito, relogin integral e shutdown global estão fora deste
  contrato e continuam na frente geral de lifecycle.

## Validação

- Pesquisa: os dois exports focados embutem o hash nativo correto, não contêm
  `SCRIPT ERROR` e têm SHA-256 respectivamente
  `0915A41D0E59658EF713780AA08D48F9130423BD0CAB9FA5978A1FD6A07BDFCC` e
  `DBDF4B418A5D59757046FC5BC4089F1A9CB3E852949E1CA64D16180895035421`.
- Automação: pendente nesta edição documental.
- Client real: não executado; `CLIENT_TESTED` não é alegado.

---
id: premium-firework-display
title: Exibição do Premium Firework recebido do servidor
subsystem: ui
status: CONTRACT
native_sha256: 8AA2F918844BCE3AFE21F1204F69757A443E32EB2F2F616936B1D9BFE215F593
updated: 2026-09-01
---

# Exibição do Premium Firework recebido do servidor

## Pergunta

Como o client nativo 7.48 recebe o desenho 10x10 do Premium Firework, cria o
efeito no humano indicado pelo servidor, anima e renderiza suas partículas, e
libera o objeto durante expiração, troca de cena, logout e shutdown?

## Fronteira de evidência

- Executável: `client748/wyd.exe nativo+patches/WYD.exe`, SHA-256 acima.
- Projeto Ghidra: `WYD748Native_20260821.gpr`.
- Corpus auxiliar:
  `%USERPROFILE%\Tools\GhidraAnalysis\20260821\decompiled`.
- Funções nativas: `FUN_00529B07`, `FUN_004D01B0`, `FUN_004D04B3`,
  `FUN_004D0651`, `FUN_004D0A4D` e `FUN_004D12E4`.
- Source atual: `Basedef.h`, `WYD748Compat.cpp`, `TMHuman.h`, `TMHuman.cpp`,
  `TMEffectFirework.h` e `TMEffectFirework.cpp`.
- Servidor: `internal/game/premium_firework.go`,
  `internal/game/premium_firework_test.go`, `internal/wire/codec.go` e
  `internal/wire/premium_firework_test.go`.

O packet, seu receptor, a construção do efeito, seu container proprietário e
o ciclo completo até a remoção foram correlacionados. O layout wire possui
asserts no client e teste byte-level no servidor; por isso a ficha está em
`CONTRACT`. Ainda não há execução do fluxo no `client748/project.exe`.

## Fluxo nativo 7.48

### Entrada observável

`CONFIRMED`: o servidor envia opcode `0x3CA` em um packet de 36 bytes. O header
ocupa os primeiros 12 bytes, há oito bytes reservados em `+0x0C` e o bitmap de
16 bytes começa em `+0x14` (offset decimal 20). Somente os primeiros 100 bits
representam a grade 10x10, em ordem LSB-first dentro de cada byte.

O `ID` do header identifica o humano sobre o qual o efeito será criado. Ao
receber o packet, `FUN_00529B07` instancia `TMEffectFireWork` com tipo `6`, na
posição do humano acrescida da altura usada pelo efeito, aplica o bitmap por
`FUN_004D0651` e anexa o objeto ao container de efeitos da cena.

### Callers

`CONFIRMED`: o dispatcher de packets do humano seleciona `0x3CA` e chama
`FUN_00529B07`. O lookup anterior resolve o receptor pelo `ID` do header; assim,
dono e observadores executam o mesmo handler sobre a instância de humano que
representa quem usou o item. O fluxo não consulta a posição enviada pelo client
para criar o efeito visual: usa a posição corrente do humano receptor.

### Função principal

`FUN_00529B07` é a fronteira packet -> efeito. `FUN_004D01B0` inicializa cem
partículas, os quatro vértices do billboard, tempo inicial, tipo, cor e estado
da explosão, e toca o som `315` de lançamento.

`FUN_004D0651`, `SetCustomFireWork`, expande os 100 bits LSB-first para a grade
10x10. Quando nenhum bit está ativo, ativa o primeiro pixel, evitando divisão
por zero e preservando uma forma mínima. Os cem pontos são distribuídos entre
os pixels ativos; a primeira cópia usa a orientação nativa da grade e as cópias
adicionais recebem o jitter nativo. Uma das seis cores é escolhida para o
efeito customizado.

`FUN_004D0A4D`, `FrameMove`, mantém o lançamento nos primeiros três segundos,
toca o som `316` uma única vez na explosão, expande a forma customizada e
atualiza cor, posição, maturidade e tamanho. Após 10.000 ms solicita a exclusão
do objeto ao ObjectManager.

`FUN_004D12E4`, `Render`, orienta o billboard pela câmera, usa a textura de
efeito `(7, 360000)` e desenha cada partícula viva em duas passagens: a cor
calculada do fogo e um núcleo menor `0xAAAAAAAA`. Ao terminar, restaura os
estados de iluminação, source blend, alpha test, Z-write e culling que o fluxo
nativo restaura; não inventa restores de `DESTBLEND` ou `ALPHAOP`.

### Callees

- `FUN_004D01B0`: construtor do efeito e som `315`.
- `FUN_004D04B3`: inicialização individual das cem partículas.
- `FUN_004D0651`: expansão e distribuição do bitmap customizado.
- `FUN_004D0A4D`: animação, explosão, som `316` e expiração.
- `FUN_004D12E4`: billboard e duas passagens de render.
- inserção `TreeNode::AddChild`: transfere o efeito para o container da cena.
- exclusão pelo ObjectManager: desliga e destrói o nó expirado com segurança.

### Saídas e erros

- packet válido e cena ativa: um efeito tipo `6` é anexado e retorna sucesso;
- bitmap vazio: o primeiro pixel é materializado e o efeito continua válido;
- humano, cena ou container ausente na source: o handler ignora o efeito sem
  dereferenciar ponteiro nulo;
- após 10 segundos: o próprio efeito solicita exclusão e deixa de renderizar;
- packet forjado ou item inválido: o WYD-Go rejeita antes da publicação e
  ressincroniza o slot quando possível.

## Estado e lifecycle

### Matriz de transições

| Evento/estado | Precondição | Função/call | Estado resultante | Side effects | Erro/saída |
| --- | --- | --- | --- | --- | --- |
| `0x3CA` recebido | humano e cena correntes | dispatcher -> `FUN_00529B07` | efeito tipo `6` anexado | som `315`; cem partículas | ausência de cena/container encerra sem efeito |
| bitmap aplicado | efeito recém-criado | `FUN_004D0651` | vetores da forma preenchidos | escolhe cor | bitmap vazio ativa pixel zero |
| fase de lançamento | tempo menor que 3 s | `FUN_004D0A4D` | partículas sobem | atraso aleatório curto | nenhuma publicação externa |
| explosão | tempo normalizado chega a zero | `FUN_004D0A4D` | forma 10x10 expande | som `316` uma vez | `m_bBomb` evita repetição |
| render da partícula | maturidade menor que 1 | `FUN_004D12E4` | duas passagens desenhadas | altera e restaura estados D3D nativos | recurso/device ausente retorna sem desenho |
| expiração | elapsed maior que 10.000 ms | `FUN_004D0A4D` -> ObjectManager | objeto marcado para exclusão | nó sai do container | não retém callback ou timer próprio |
| troca de cena/logout | cena antiga desmontada | destruição do effect container | todos os efeitos filhos destruídos | referências deixam de existir | nenhum efeito migra para a nova cena |
| relogin | nova cena e novos humanos | próximo `0x3CA` | novo efeito pertence à nova cena | nenhum estado visual anterior é reutilizado | sem packet não há recriação |

### Vtables, vptrs e receptores

O dispatcher chega ao receptor concreto `FUN_00529B07` do humano selecionado
pelo header. O efeito é um `TreeNode`; suas entradas virtuais de frame e render
são `FUN_004D0A4D` e `FUN_004D12E4`. O ObjectManager percorre o container de
efeitos da cena e chama essas entradas enquanto o nó estiver vivo.

### Ownership

O handler aloca o efeito e o entrega imediatamente ao `m_pEffectContainer` da
cena. Depois de `AddChild`, o container/ObjectManager possui o lifecycle do nó.
O humano não guarda ponteiro para o efeito e o packet não permanece referenciado
após `SetCustomFireWork` copiar seus bits.

### Falha parcial

A source valida humano, cena e container antes de alocar. O bitmap é copiado
para armazenamento próprio; `nullptr` equivale a bitmap vazio e ativa a forma
mínima. Não há alocação secundária por partícula. No servidor, layout, posição,
slot, item, catálogo, bits reservados e cooldown são validados antes de mutar;
falha de persistência restaura o item e impede publicação aos observadores.

### Cleanup e teardown

A expiração normal chama `DeleteObject(this)` após 10 segundos. A desmontagem
da cena destrói o effect container e seus filhos, cobrindo efeitos ainda vivos.
Como o humano e o packet não possuem o nó depois da inserção, não há referência
externa que precise ser invalidada.

### Shutdown

O shutdown global desmonta a cena e seu effect container antes de finalizar o
ObjectManager e o dispositivo. O efeito não cria thread, timer independente,
callback ou recurso heap externo; portanto não pode reentrar depois do teardown.

### Logout e relogin

Logout e troca de cena descartam o container antigo e todos os fogos pendentes.
Relogin constrói outra cena, outro container e outros humanos; um novo efeito só
existe se um novo `0x3CA` for recebido. Cor, bitmap e relógio nunca atravessam a
fronteira da cena anterior.

## Wire, ABI e recursos

Contrato S->C nativo:

```text
offset  tamanho  campo
0x00    12       MSG_STANDARD (Size, Key, CheckSum, Type=0x3CA, ID)
0x0C     8       reservado, zerado pelo servidor
0x14    16       bitmap 10x10 LSB-first; últimos 28 bits reservados
total   36
```

`sizeof(MSG_PremiumFirework) == 36` e
`offsetof(MSG_PremiumFirework, Bitmap) == 20` são impostos por `static_assert`
na build Win32. O WYD-Go constrói exatamente o mesmo layout e possui teste
byte-level para tamanho, opcode, ID, bytes reservados e bitmap.

O fluxo completo começa no `0x3C9` C->S de 52 bytes: o bitmap fica em offset
34. O servidor aceita somente o item premium registrado para desenho, consome
e persiste antes de publicar `0x3CA` ao dono e observadores visíveis. O recurso
visual usado pelo efeito é a textura `(7, 360000)` e os sons são `315/316`.

## Mapeamento atual

### Source recompilável

- `Basedef.h`: pacote tipado `MSG_PremiumFirework` e opcode `0x3CA`.
- `WYD748Compat.cpp`: asserts de packet e offsets das estruturas do efeito.
- `TMHuman.cpp`: dispatch tipado e `OnPacketPremiumFireWork`.
- `TMEffectFirework.cpp`: construtor, partículas, bitmap, frame, render e
  teardown reconstruídos a partir das seis funções nativas.

### WYD-Go

- `0x3C9` possui gate exato de 52 bytes e parser dos campos reservados.
- item, posição, catálogo e bitmap são revalidados de forma autoritativa;
- o consumo segue validar -> mutar -> persistir -> publicar;
- falha de save faz rollback e não publica desenho;
- cooldown de 200 ms bloqueia replay e flood de persistência;
- `0x3CA` é enviado ao dono e aos observadores na visão, nunca a outsiders.

## Matriz de delta

| Claim | Nativo 7.48 | Source atual | TMProject | WYD-Go | Decisão |
| --- | --- | --- | --- | --- | --- |
| packet de apresentação | `0x3CA`, 36 B, bitmap `+20` | layout e asserts equivalentes | stub incompleto antes da adaptação | encoder byte-level | `PARIDADE_NATIVA` |
| forma customizada | 10x10 LSB-first, tipo `6` | reconstruída com forma mínima | classe existia incompleta | preserva 100 bits | `PARIDADE_NATIVA` |
| animação/render | 10 s, sons `315/316`, duas passagens | constantes e estados nativos | métodos eram stubs | não aplicável | `PARIDADE_NATIVA` |
| consumo/publicação | validação server-side não é ABI do client | envia intenção e apresenta resultado | fluxo `0x3C9` existente | autoritativo e persistente | `MODERNIZACAO_COMPATIVEL` |

## Decisões

- Conservar o fluxo autoritativo já implementado no WYD-Go e restaurar somente
  a apresentação nativa que faltava na source.
- Não transportar layouts internos do objeto nativo; somente o contrato wire e
  o comportamento observável foram portados para os tipos C++ atuais.
- Manter o bitmap de 16 bytes para compatibilidade exata, embora apenas 100 bits
  sejam visíveis.
- Não promover para `CLIENT_TESTED` até o fluxo real ser executado.

## Lacunas

- Testar no client real o desenho de mais de um padrão e o fallback vazio.
- Confirmar dono e observador vendo a mesma forma e outsiders sem publicação.
- Observar sons, duas passagens, expiração em 10 s e ausência de efeito stale.
- Repetir após troca de cena, logout e relogin.

## Validação

- Pesquisa: packet, funções, callers/callees, ownership, teardown e wire
  correlacionados no executável do SHA-256 registrado.
- Automação: asserts C++ e testes Go de layout, consumo, persistência, rollback,
  validações, cooldown e visibilidade.
- Client real: não executado; estado máximo `CONTRACT`.

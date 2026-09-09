# UI, HUD e grids do client WYD 7.48

Referência focada para impedir regressões ao portar a UI do TMProject para o
client 7.48. Deve ser lida junto de `ghidra-client748.md`; não substitui o gate
Ghidra nem transforma relato visual em contrato.

## Escopo e fontes

Ordem de decisão para UI:

1. lifecycle, IDs e cálculos do binário histórico
   `client748/wyd.exe nativo+patches/WYD.exe` no Ghidra;
2. recursos efetivamente carregados pelo 7.48;
3. screenshot do client original para composição visual;
4. log runtime do `project.exe` para confirmar materialização e posição;
5. TMProject 7.59 apenas como referência de implementação.

Referências visuais ficam em `client748/screenshot/`. A imagem
`Capture0084.jpg` é a âncora atual para HUD e inventário. O log
`client748/client-debug.log` é diagnóstico volátil: confirmar que pertence ao
hash do executável em teste antes de promover seus valores.

## Evidência nativa confirmada

Exportações Ghidra citadas abaixo são descobertas pelo procedimento de
`ghidra-client748.md` e ficam fora do Git.

| Função 7.48 | Contrato recuperado |
| --- | --- |
| `FUN_0040d13e` | Construtor nativo de `SGridControlItem`; meshes e UI legada usam caixa de 24 por dimensão lógica. Sprites UI2 usam 23 para itens 5000..5102 e 32 para os demais, sempre multiplicados por `g_pItemGridXY`. |
| `FUN_0040e817` | Ao adicionar item à grid ordinária, limita a escala a `(altura_lógica × 0.3) / mesh->m_fMaxZ` quando a malha excede o alvo. A comparação histórica com o binário patchado mostra altura lógica 1 sem alteração do renderer; esse fato deve ser adaptado na source. |
| `FUN_0040dd00` | Atualiza frame/posição do item e centraliza a representação na célula/região nativa. |
| `FUN_00435b13` | Materializa os seis textos do HUD compacto nos IDs 1029, 1030, 1031, 1032, 1033 e 1040; vincula Skill comum (1905), Skill Apprentice (1889), Inventory (257), os seis ItemMix e Party (root 1857, lista 1863, botão 5742); recompõe em runtime os layouts responsivos desses roots. |
| `FUN_004431e4` | Atualiza EXP atual/limiar absoluto do próximo nível, ATT/dano da skill selecionada, DEF/defesa base-companheira, progresso de EXP dividido em quatro quartos e os campos Character Att Speed/C.POINT/HOLD/Kingdom. |
| `FUN_0044da6f` | Toggle central do painel Party: alterna o root 1857 e mantém a seleção do botão 5742 inversa à visibilidade. |
| `FUN_00492e7d` | Dispatcher FieldScene que encaminha `0x37F`, `0x37D` e `0x37E` aos handlers de convite, inclusão e remoção Party. |

Antes de alterar esse fluxo, abrir a função e seus callers/callees novamente.
Os nomes acima são âncoras do hash legado registrado em
`ghidra-client748.md`, não API estável entre binários.

## Contrato do HUD compacto

IDs nativos:

```text
1029  ATT primário
1030  ATT companion (dano da skill selecionada)
1031  EXP atual
1032  limiar absoluto do próximo nível
1033  DEF primária
1040  DEF companion (defesa base/ability nativa)
1171  progresso dentro do quarto atual da EXP
1172  indicador do primeiro quarto concluído
1173  indicador do segundo quarto concluído
1174  indicador do terceiro quarto concluído
```

No snapshot runtime de 23/08/2026, sob o parent 5716:

```text
1032 x=110.4 y=16.6
1031 x=42.4  y=16.6
1040 x=150.7 y=38.4
1033 x=118.8 y=38.4
1030 x=58.4  y=38.4
1029 x=24.4  y=38.4
```

Essas posições confirmam o recurso carregado; não devem virar coordenadas
hardcoded se o controle nativo já existe. IDs modernos 5718–5720 não podem ser
reaproveitados: no grafo 7.48 eles podem identificar filhos não relacionados e
deslocar/corromper ATT, DEF ou EXP.

`Capture0084.jpg` mostra EXP como par textual absoluto, por exemplo
`139188261 / 140000000`, e a linha inferior como pares de ATT/DEF. A função
nativa divide o intervalo entre o nível atual e o próximo em quatro partes:
1172–1174 recebem a textura concluída conforme os quartos completos e 1171
recebe máximo/valor apenas do quarto corrente. Não substituir esse contrato por
progress bar ou labels exclusivos da 7.59.

O parent 5716 nasce com largura aproximada de 196,3 no recurso 1024×768. Se o
background for estendido para preencher o centro inferior, a mesma transformação
horizontal deve ser aplicada aos filhos 1029–1040 e ao progresso 1171. Alargar
somente parent/background mantém os filhos nas coordenadas estreitas originais e
amontoa EXP/ATT/DEF no lado esquerdo. Os ornamentos 1168 e 1172–1174 permanecem
ancorados à direita, preservando o tamanho nativo.

## Contrato dos campos de estado em Character

`FUN_00435b13` vincula e `FUN_004431e4` atualiza os controles nativos abaixo:

```text
1110  Att Speed
1168  ornamento HOLD, visível somente com Hold/FakeExp positivo
1376  texto C.POINT no formato da mensagem 304
1377  emblema Kingdom, visível somente com manto/reino válido
```

O grupo moderno `65768/65771..65774` não substitui `1377` no recurso 7.48.
HOLD e Kingdom devem partir ocultos durante a construção e ser recalculados
em cada atualização de estado. Textos de tooltip modernos são opcionais;
ausência deles não pode bloquear nem derrubar a atualização nativa.

A ficha `flows/ui/character-stat-fields-update.md` fecha callers, callees,
ownership, teardown e relogin. CP/Chaos não ocupa `MSG_UpdateEtc::Hold`: esse
campo transporta a dívida Held EXP persistida, enquanto CP continua na borda
legada de `CreateMob`. Hold zero significa somente ausência de dívida; Hold
positivo torna `1168` visível e `1376` mostra valor/percentual sobre 10% do
intervalo do nível atual. O lifecycle completo está em
`flows/combat/pvp-death-held-exp-lifecycle.md`.

## Contrato contextual dos itens

O footprint lógico e a escala visual são conceitos diferentes.

| Contexto | Ocupação lógica | Render e drag |
| --- | --- | --- |
| Inventário | 1×1 | Centralizado e contido numa célula 24×24. |
| Cargo | 1×1 | Mesmo contrato do inventário. |
| Loja de NPC | Grade ordinária da loja; para os itens adaptados, 1×1 | Centralizado na célula, sem invadir colunas vizinhas. |
| Equipamento | Região irregular do slot | Item grande, ajustado à região inteira. |
| Drag sobre inventário/cargo/loja | Footprint do destino | Visual usa escala do destino e highlight de uma célula. |
| Drag sobre equipamento | Região do slot de destino | Visual e highlight cobrem toda a região de equipamento. |

No executável stock, a fórmula nativa usa `meshMaxZ` e a altura do footprint
lógico original:

```text
targetExtent = max(itemCellHeight, 1) × 0.3
scale = min(1, targetExtent / meshMaxZ)
```

O `WYD.exe` histórico patchado deste repositório muda as 16 entradas de
`g_pItemGridXY` para `{1,1}` e reduz cada máscara de `g_pItemGrid` à primeira
célula. A comparação byte a byte comprova que ele não altera o renderer. Assim,
em inventário, cargo e loja ordinária a altura lógica que chega a
`FUN_0040e817` já é 1. Essa é a fórmula nativa recuperada:

```text
targetExtent = 0.3
scale = min(1, targetExtent / meshMaxZ)
```

`FUN_0040e6aa` (slot virtual `+0x8c`) e `FUN_0040e817` (slot virtual `+0x90`)
executam a mesma regra de inserção e escala. Ambas consultam exclusivamente
`mesh+0x354` (`MaxZ`); o client nativo não usa AABB, raio nem exceção por família
de item. A source ativa mantém essa fórmula única para grids de células,
mantém equipamento em escala própria `1.0` e recalcula o contexto durante drag.

Os tamanhos `23`, `24` e `32` são coordenadas lógicas da UI 800×600. Na source,
a caixa visual (`m_nWidth`/`m_nHeight` e `m_GCObj.m_fWidth`/`m_fHeight`) deve
aplicar `RenderDevice::m_fWidthRatio`/`m_fHeightRatio`, assim como a grid. Isso
preserva o resultado em 800×600 e evita ícones fisicamente pequenos dentro de
slots ampliados em resoluções maiores sem alterar `fScale`.

`FUN_0040fc3e` calcula a origem com `largura/colunas` e `altura/linhas`.
`FUN_0040dd00` centraliza usando metade da largura e metade da altura da região;
essa centralização foi confirmada visualmente no client atual. Isso não prova,
sozinho, drag, equipamento ou todas as famílias de malha. O redimensionamento
dos itens ainda exige execução real documentada antes de receber
`CLIENT-TESTED`.

Equipamento não deve reutilizar a escala reduzida do inventário. Recalcular a
escala quando o item muda de contexto, inclusive durante drag. A área azul
indica destino aceito; vermelha indica destino bloqueado. No equipamento, a
cor deve cobrir a região inteira, não somente o quadrado sob o cursor.

A diferença binária histórica que comprovou o footprint 1×1 altera
inventário/cargo; ela não autoriza reduzir o visual equipado nem mudar o
packet/ABI. Esse comportamento deve existir integralmente na source e nos
assets consumidos pelo `project.exe`; aplicar ou verificar o patcher legado é
proibido.

## AutoTrade e seleção de itens para venda

O fluxo nativo recuperado em `FUN_004110f5` materializa somente:

```text
626  painel de entrada
627  edit de preço
630  caption
667  botão de confirmação
```

Não existe seletor de chat nesse fluxo do 7.48. `m_pChatSelectPanel` veio da
source 7.59 e pode ser nulo por contrato; proteger todos os acessos — abertura,
preço inválido, cancelamento, fechamento, reentrada e paths compartilhados de
input — sem criar controle moderno para satisfazer o ponteiro.

A blacklist nativa comprovada para adicionar item à loja é:

```text
508, 509, 522, 526–537, 747, 3200–3299
```

Não importar blacklist de versão mais nova. Em particular, o item `4905` não
pertence à regra 7.48 e não deve receber a mensagem de item impossível de
vender. O servidor continua obrigado a revalidar item, slot, quantidade e preço;
a permissão no client não o torna autoritativo.

Crash confirmado no dump `client748/client-crash-20260826-154302.dmp`:
`SGridControl::TradeItem`, RVA `0x718AC`, ao desreferenciar
`m_pChatSelectPanel`. Corrigir somente o primeiro acesso não basta; procurar o
símbolo em todo o lifecycle e proteger cada path executável.

Aceitação mínima do AutoTrade:

1. adicionar item permitido e informar preço válido;
2. rejeitar preço inválido sem crash e conservar foco/estado útil;
3. cancelar e fechar por todos os caminhos nativos;
4. abrir a loja e publicar a oferta;
5. tentar item realmente bloqueado e o item `4905`;
6. repetir/reentrar sem mensagem indevida, item fantasma ou crash.

## ItemMix e NPCs artesãos nativos

O client 7.48 possui seis painéis ItemMix distintos. Todos são utilizados pelo
fluxo nativo; portanto nenhum deles é uma UI livre para um sistema novo.

| ItemMix | Painel / slots / executar | NPC e condição nativa | Opcode ativo |
| --- | --- | --- | --- |
| 1 | 1360 / 1367–1374 / 1375 | Compositor, cabeça 54; chunk 19,13 usa modo 0 e chunk 25,13 usa modo 1 | `0x3A6` |
| 2 | 6110 / 6112–6119 / 6111 | Aylin, cabeça 55 | `0x3B5` |
| 3 | 6145 / 6147–6152 / 6146 | Agatha, cabeça 56 | `0x3BA` |
| 4 | 6432 / 6436–6438 / 6434 | Tiny, cabeça 68 fora do chunk 19,15 | `0x3C0` |
| 5 | 6481 / 6483–6489 / 6482 | Lindy no chunk 13,13 e Odin no chunk 25,13 com Merchant baixo = 8 | `0x2C3` Lindy; `0x2D2` Odin |
| 6 | 6512 / 6516–6518 / 6514 | Ehre, cabeça 68 no chunk 19,15 | `0x2D3` |

`FUN_00435b13` centraliza cada um dos seis roots ItemMix separadamente e
posiciona Inventory à direita por
`centerX + Inventory.width*0.5 + 10`, com ambos centralizados verticalmente.
Como Shop, Trade e AutoTrade movem o mesmo root 257, essa composição deve ser
reaplicada ao abrir qualquer artesão; a posição serializada ou residual não é
o layout final nativo. Os toggles/cleanups correspondentes são, em ordem,
`FUN_00449384`/`FUN_004487E2`, `FUN_00449632`/`FUN_004489C5`,
`FUN_004498E0`/`FUN_00448C38`, `FUN_00449B8E`/`FUN_00448E0B`,
`FUN_00449E3C`/`FUN_00448FDE` e `FUN_0044A0FB`/`FUN_004491B1`.

ItemMix2 também possui quatro grids de resultado, IDs 6129–6132. A ordem das
condições em `MouseClick_MixNPC` faz parte do contrato: Ehre deve ser testada
antes do fallback Tiny e os casos específicos da cabeça 67 antes do fallback.
Não centralizar artesãos por nome ou cabeça sem conservar chunk e Merchant.

O ItemMix5 possui ainda o modo nativo `0x2C4`, selecionado pela cabeça 67 no
chunk 28,24 quando o NPC não tem ID 1033. A source atual do servidor não possui
handler nem receita autoritativa para esse opcode. Esse ramo é `DORMANT`, não
uma autorização para inventar uma receita ou reutilizar o painel. O ItemMix5
continua compartilhado por Lindy e Odin.

Os controles modernos 65857/65861+, o botão 81923 e o caminho 86019 pertencem
à source mais nova e não substituem os IDs acima. Como os valores numéricos de
grid foram reaproveitados na source moderna, resolver os destinos ItemMix pelo
painel visível e pelos IDs/ponteiros exatos; não renumerar a ABI nem deixar os
ramos de quickslot capturarem as grids nativas.

## Painel Party nativo

O 7.48 usa root `1857`, lista `1863` e botão inferior `5742`.
`FUN_00435b13` coloca root e botão em `x=0` e
`y=viewportHeight-root.height-165`. `FUN_0044da6f` é o toggle compartilhado
por botão, tecla `P`, packets, `Esc` e fechamento por UI concorrente; o botão
fica selecionado quando o painel está oculto e não selecionado quando visível.

Os packets `0x37F`, `0x37D` e `0x37E` alimentam convite, inclusão e remoção;
o click esquerdo no convite envia `0x3AB`. A lista também trata
`Ctrl`+click para remoção e click direito para skill de suporte. O container é
dono dos controles e a lista dos itens; esconder Party não encerra o grupo.
No teardown, o deleting destructor da FieldScene converge em `FUN_004358da`.

A source compatível deve vincular `1857/1863/5742`, reaplicar a posição ao
abrir e centralizar a seleção do botão no mesmo helper de visibilidade. Os
controles modernos `475136/475138/65799` podem permanecer no layout posterior,
mas não substituem os IDs nativos. AutoParty moderno é opcional no modo 7.48 e
não justifica fabricar controles ausentes. A ficha `TRACED` é
`flows/ui/party-panel-layout-lifecycle.md`.

## Painel Quest nativo

O 7.48 materializa o toggle `315`, root `320`, listas/conteúdos
`321/322`, `325/326`, `327/328` e `335/334`, fechar `323`, título `324`, abas
`329/330/331/333` e memo `332`. `FUN_00441823` centraliza `320` exatamente em
`(viewport-panel)/2` nos dois eixos e carrega `QuestSubjects.txt` até
`QuestSubjects4.txt`, além de `QuestMessage.txt`.

`FUN_004662C5` centraliza toggle, abas, seleção das listas, memo e fechamento.
Os conteúdos passam por `FUN_0049E50F`, que rejeita destino nulo. A tecla `X`
deve convergir no toggle nativo `315`; `65793` é o ID importado e não substitui
o callback clássico. `FUN_00453C59` encaminha `Esc` para `FUN_0044DF53`, e o
deleting destructor da FieldScene converge em `FUN_004358DA`.

A source deve reaplicar a centralização no open, vincular todo o grupo
`320..335`, selecionar somente um par lista/conteúdo e proteger controles
opcionais. Click, tecla `X`, botão fechar, `Esc` e AirMove precisam compartilhar
o mesmo helper de visibilidade. Após troca de cena ou relogin, todos os
bindings e textos são recriados; nenhum ponteiro antigo pode sobreviver. A
ficha `TRACED` é `flows/ui/quest-panel-layout-lifecycle.md`.

## Lifecycle e fechamento

Para cada janela, recuperar no 7.48:

- função de criação e IDs realmente presentes no recurso;
- ação de abrir, tecla/click e callback;
- estado modal e ordem de fechamento/destruição;
- owner de ponteiros e proteção para controles opcionais ausentes.

`Esc`, botão `X` e botão textual de cancelamento/fechamento devem convergir no
mesmo caminho nativo de transição. Não ocultar apenas o desenho deixando modal,
captura de mouse ou ponteiro ativo. O menu de sistema deve usar o
posicionamento central do recurso/lifecycle nativo, não coordenada absoluta do
canto superior esquerdo.

Roots 1905 e 1889 possuem lifecycles distintos no 7.48. A tecla/botão Skill
alterna somente 1905. O root 1889 é a janela Skill Apprentice controlada pela
interação com NPC e deve iniciar oculto; nunca vinculá-lo ao mesmo toggle da
Skill comum.

Durante a interação com o Skill Apprentice, `FUN_0044c15c` abre os dois roots:
1889 centralizado e 1905 à direita em
`centerX + skillWidth*0.5 + 10`. O fechamento oculta o par e devolve 1905 ao
centro para seu toggle independente. A grade do mestre em `FieldScene2.bin` é
1894 (`type=16`, parent 1889, `31,63`, `191x241`, `8x4`); 6128 é o controle de
descrição `TML_SKILLM_DESC`, não uma grade.

## Mensagens, login e seleção de servidor

O 7.48 reutiliza recursos próprios para login, notice, confirmação e
notificação. Antes de alterar translucidez, tamanho ou posição:

1. identificar o painel nativo, sua textura e alpha;
2. identificar a função que escreve o texto e controla visibilidade;
3. confirmar botões e callbacks materializados no recurso;
4. comparar com screenshot original equivalente;
5. testar fechamento por click e `Esc` sem deixar input capturado.

Na seleção de servidor/canal, a seleção deve aparecer pelo overlay/estado do
controle nativo. O bypass de disponibilidade de canal, quando necessário para
desenvolvimento, não pode substituir o estado visual de seleção nem relaxar a
validação autoritativa depois da conexão.

No branch Scene2 de `FUN_004A8F14`, o root nativo `4622` é centralizado em X e
Y por `(viewport-root)/2`. A tradução da source é `P_SERVER_SEL` (`65537`). Os
filhos dinâmicos, textos, listas e hitboxes acompanham esse root; não aplicar
offset vertical adicional nem corrigir cada filho separadamente. Os ajustes
independentes já comprovados são largura de canal 140, Server `y += 5`, Channel
`x += 8`, `y += 5` e os branches de logo para 1024/1280/1600. A ficha
`TRACED` é `flows/ui/server-selection-layout-lifecycle.md`.

## Checklist de aceitação ainda não provado

Os itens abaixo vieram de regressões observadas e precisam de teste real. Eles
não são `CLIENT-TESTED` apenas por estarem documentados:

- seleção de servidor/canal mostra overlay translúcido e nomes alinhados;
- login, notice, informação e mensagens de erro usam painel translúcido 7.48;
- `Esc` fecha todas as UIs abertas conforme prioridade nativa;
- `X`, Confirm e Cancel executam o callback correto;
- Character não contém barra preta e mostra Skill Mastery, Chaos Point e
  Attack Speed nos controles existentes do 7.48;
- grid 1×1 mantém item contido e centralizado em inventário, cargo e loja;
- equipamento e drag de equipamento usam a região inteira e highlight completo;
- item pode ser movido, equipado e jogado no chão com feedback correto;
- compra sem gold, preço, gold e confirmação de venda aparecem corretamente;
- menu de venda de NPC e menu de sistema têm layout/lifecycle nativos;
- Party abre por botão e `P`, mantém lista/callbacks funcionais e fecha por
  `Esc`/UI concorrente sem perder membros; repetir após troca de cena/relogin;
- montarias KR importadas são renderizadas sem corromper ABI 7.48;
- barra inferior exibe EXP/ATT/DEF nos IDs nativos e ocupa a composição correta.

## Validação mínima

1. Confirmar hashes dos executáveis.
2. Compilar e confirmar que o build instalou `client748/project.exe`
   automaticamente.
3. Registrar hash do candidato e limpar/rotacionar log diagnóstico.
4. Executar o fluxo real em resolução de referência.
5. Capturar screenshot comparável ao original.
6. Testar click, drag, destino válido/inválido, `X`, `Esc`, erro e relogin.
7. Inspecionar log e minidump se houver crash.
8. Classificar cada item individualmente como `IMPLEMENTED`,
   `STATICALLY VERIFIED`, `AUTOMATED TESTED` ou `CLIENT-TESTED`.

Paridade só é concluída quando o fluxo e o visual forem testados no client; uma
compilação sem erro não fecha este checklist.

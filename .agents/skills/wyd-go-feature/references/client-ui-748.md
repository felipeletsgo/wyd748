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
| `FUN_0040d13e` | Construtor nativo de `SGridControlItem`; célula visual de 24×24 e largura/altura lógicas vindas de `g_pItemGridXY`. |
| `FUN_0040e817` | Ao adicionar item à grid ordinária, limita a escala a `(altura_lógica × 0.3) / mesh->m_fMaxZ` quando a malha excede o alvo. A comparação histórica com o binário patchado mostra altura lógica 1 sem alteração do renderer; esse fato deve ser adaptado na source. |
| `FUN_0040dd00` | Atualiza frame/posição do item e centraliza a representação na célula/região nativa. |
| `FUN_00435b13` | Materializa os seis textos do HUD compacto nos IDs 1029, 1030, 1031, 1032, 1033 e 1040; também vincula separadamente a Skill comum (root 1905) e o Skill Apprentice de NPC (root 1889). |
| `FUN_004431e4` | Atualiza EXP atual/limiar absoluto do próximo nível, ATT/dano da skill selecionada, DEF/defesa base-companheira e o progresso de EXP dividido em quatro quartos. |

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

Entretanto, a source recompilável precisou de uma adaptação delimitada porque
malhas não-ovo com origem/extremos assimétricos escapavam da célula quando
avaliadas apenas por `MaxZ`. O contrato atual separa explicitamente:

- `egg001..egg014` (meshes 300–303 e 937–946 comprovadas pelo ItemList 7.48):
  usa exatamente `min(1, 0.3 / MaxZ)`, evitando que os ovos fiquem minúsculos;
- demais itens em grids de uma célula: contém a diagonal completa do AABB em
  `0.27`, reservando borda conservadora sem ampliar malhas pequenas;
- equipamento: mantém escala própria `1.0`, sem herdar o fit de grid;
- drag: recalcula conforme o contexto de destino.

A fórmula dos ovos é comportamento nativo recuperado; a contenção por AABB é
uma adaptação deliberada da source atual, não uma alegação de que o binário
stock fazia o mesmo. Não generalizar a exceção de ovos nem trocar novamente a
métrica sem comparar famílias representativas de armas, armaduras e itens
assimétricos.

`FUN_0040fc3e` calcula a origem com `largura/colunas` e `altura/linhas`.
`FUN_0040dd00` centraliza usando metade da largura e metade da altura da região;
essa centralização foi confirmada visualmente no client atual. Isso não prova,
sozinho, drag, equipamento ou todas as famílias de malha. O redimensionamento
dos ovos ainda exige execução real documentada antes de receber
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

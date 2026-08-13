# Cadeia de patches do client 7.48

O `WYD.exe` em uso **não** é o executável original: ele é o resultado de sete
scripts aplicados em ordem. Cada elo é verificado por SHA‑256, e a cadeia
inteira reproduz o binário em uso **bit a bit**.

```
WYD.original.exe              B545EA10…619C   client 7.48 de 23/04/2007
  └─ Patch-WYD748.ps1                         câmera/mouse, cores de dano e linha-base
WYD.pre-extended-stats.exe    2AA1773A…21EE   ← LINHA-BASE
  └─ Patch-WYD748-ExtendedStats.ps1           stats 32-bit, seção .xstat
  (intermediário)             4724B76F…45D6
  └─ Patch-WYD748-Bypass.ps1                  versão + 3 checksums
  (intermediário)             E704DA0A…E20D
  └─ Patch-WYD748-Macro.ps1                   rotação de skills + buffs
WYD.exe pré-Lindy             4E916C1F…9BA2
  └─ Patch-WYD748-Lindy.ps1                  validação 4010 -> 3448
WYD.exe pós-Lindy             9762B1AC…7F18
  └─ Patch-WYD748-Costumes.ps1                 coleção de trajes do KR
WYD.exe pós-trajes            4A2AA372…16DE
  └─ Patch-WYD748-KRMounts.ps1                 montarias visuais do KR
WYD.exe pós-montarias         F6F99CC0…E72D   ← binário atualmente versionado
```

## Reaplicar

```powershell
cd client748
.\Apply-WYD748.ps1
```

A ordem **importa**: o orquestrador identifica o SHA atual, executa somente os
elos restantes e confere a saída de cada um. Para uma instalação nova execute
`Apply-WYD748.ps1 -ResetFromOriginal`; para retomar um processo interrompido,
execute o mesmo comando novamente. O reset cria `WYD.before-reset.exe` antes
de substituir o executável. A opção
`-VerifyOnly` não escreve o executável. Os scripts individuais continuam na
pasta como implementações auditáveis e pontos de rollback; não devem ser
chamados diretamente no uso normal.

O primeiro elo também registra o byte de título em `0x1C5069` (`69 -> 78`).
Esse byte já existia na linha-base `WYD.pre-extended-stats.exe`; incluí-lo no
script elimina a última diferença manual entre o original e a cadeia
reproduzível.

O script de stats também restaura a linha‑base sozinho quando o SHA de entrada
não bate, então rodá-lo duas vezes é seguro.

## Os quatro bypasses — leia antes de mexer

`Patch-WYD748-Bypass.ps1` concentra os quatro saltos necessários para liberar
versão, SkillData e as duas validações do ItemList. A cadeia anterior tinha
somente três; o checksum inicial do ItemList ficou descoberto e era a causa do
erro ao regenerar `ItemList.bin`.

| offset | o que faz | sintoma se faltar |
| --- | --- | --- |
| `0x07E44A` | ignora a checagem de **versão** do client | não conecta |
| `0x14F9E8` | ignora o **checksum** do SkillData | não carrega |
| `0x14FAD0` | ignora o checksum inicial do **ItemList** | ItemList regenerado é rejeitado |
| `0x14FBFC` | ignora o checksum final do **ItemList/ExtraItem** | ExtraItem/ItemList editado é rejeitado |

Nos três saltos curtos de `+4`, um `74 04` (JZ) vira `EB 04` (JMP). O
checksum inicial do ItemList usa `74 07 -> EB 07`, porque o bloco de falha é
maior. Em todos os casos o salto passa a ser incondicional e o `33 C0`
(`xor eax,eax`) logo à frente — o caminho de falha — nunca executa.

O quarto salto foi descoberto ao comparar a falha de carregamento com a
linha‑base; sem este script, qualquer regeneração do `ItemList.bin` voltaria a
ser rejeitada em silêncio.

## Fix de macro (aplicado em 25/07/2026)

`Patch-WYD748-Macro.ps1` corrige a rotação do macro (teclas A/D): o cooldown por
skill era **lido** em `0x00493959` mas nunca **escrito**, então o primeiro slot
ficava sempre "pronto", disparava todo frame, e a rotação nunca avançava até os
buffs. Sintoma: *só 1 skill, sem rotação, sem buff*.

Ele grava o cast em `DAT_0092eaf8[skillID*4]`. Ataques registram o instante
atual; buffs registram `agora + 150000 ms - cooldown`, de forma que a próxima
checagem automática aconteça exatamente aos 150 segundos — 30 segundos antes
do fim nominal de 180 segundos. O clique manual não passa por esse hook.

O macro segue para a rotina nativa `0x004595EC`, que também atende o cast
manual e escolhe o packet por `SkillData.MaxTarget`: `0x39D/48`, `0x39E/52` ou
`0x36C/96`. O script possui assertions para a `CALL` e para os dois builders
internos. Essas guardas não alteram o hash final; impedem aplicar a code cave
sobre uma linha-base que voltou a montar `0x39D/96` como single-target.

A implementação atual usa a área executável livre da seção `.xstat`
(`0x013C0207`) e uma tabela de 96 bits com os buffs reconhecidos pelo macro. O
hook em `0x00493970` retorna exatamente para `0x00493975`. Isso substitui a
primeira code cave encadeada, cujo retorno incorreto para `0x00497975` fazia o
macro mágico tratar o mob ID `1000` como ponteiro e crashar em `0x004979D3`.

SHA-256 atual do `WYD.exe`:
`4E916C1FD94D60D5EF7F8914B621BAB3787E7BF5460FB251C59F71BCC4D9BA2F`.

Estava **ausente** do binário em uso — os bytes em `0x093970` eram idênticos aos
do original. O script existia e estava bem documentado, mas perdeu-se numa
regeneração anterior, quando a cadeia ainda não estava escrita. Foi o que
motivou este arquivo.

## Limitação: o C.C. do 7.48 é um macro improvisado

O macro nativo do 7.48 é bem mais pobre que o **C.C. do client W2PP**, que tem
um subsistema completo (`ThreadMacro`, em `MacroPerga.cpp` + `TMFieldScene`):

| | 7.48 (nosso) | W2PP |
| --- | --- | --- |
| poção | limiar **fixo** em 50% | `r_AutoMacroHP`/`MP` configurável, passos de 10% |
| posição | sempre livre | `r_AutoMacroMove` + `r_AutoMacroX/Y` (fixa ou livre) |
| modos | A = físico, D = magia | `r_AutoMacro` com 4 modos |
| execução | dentro do loop de frame | thread dedicada |
| UI | nenhuma | painel de configuração |

Portar o C.C. completo **não é um patch de bytes**: exige injetar thread, estado
de configuração e UI. É um projeto próprio, não um elo desta cadeia. O fix acima
apenas faz o macro que já existe funcionar como deveria.

## Fix da Lindy (último elo)

Na rotina de combinação do client 7.48, em VA `0x00413FB7` / file offset
`0x13FB7`, o primeiro material era comparado com `4010`:

```text
81 FA AA 0F 00 00    cmp edx,4010
```

O servidor e a receita 7.54 usam dois pacotes `3448` com `EF_AMOUNT=10`. O
script `Patch-WYD748-Lindy.ps1` altera somente essa constante:

```text
81 FA 78 0D 00 00    cmp edx,3448
```

Ele exige o SHA-256 pré-Lindy completo, confirma os seis bytes originais antes
de escrever, cria `WYD.pre-lindy.exe` sem sobrescrever um backup divergente,
confirma os seis bytes novos e imprime a SHA-256 final. Os compares próximos
com `4127`, `413` e `4141` pertencem a outros ramos e permanecem inalterados.
Na cópia de teste do binário versionado, a saída foi
`9762B1AC6EFB4AB3C800877DE1DA048DD43EA407FCEEA945C755DF6986607F18`.

O script local recebido como `Patch-WYD748-Lindy-Local.ps1` não é o elo
versionado: ele aceita qualquer executável com os seis bytes antigos. Para
reprodução pública da cadeia, use o script estrito acima.

## Coleção de trajes do client KR

O sexto elo instala a coleção cujo conjunto completo de dependências existe nos
clients KR fornecidos. São `135` itens, `129` renderers, `176` texturas de
traje e `856` arquivos de mesh/textura. O caminho vivo do 7.48 confirmou
`Equip[13]` (`Pos=8192`) como o slot correto; nenhum struct ou offset de outra
versão foi portado. A seção PE dedicada `.costkr` contém somente as tabelas e os
três adapters necessários nos pontos nativos do 7.48.

O executável original possui uma barreira anterior ao seletor: somente IDs
`4151..4200` alcançam o caminho nativo de traje. Por isso os primeiros trajes
importados apareciam e os itens modernos eram equipados sem alterar a mesh. O
adapter de admissão consulta a lista exata dos 135 itens importados antes dessa
barreira; itens que não pertencem ao manifesto continuam obedecendo às
comparações originais. Como o visual enviado ao mundo conserva somente os 12
bits baixos do item, o gate e o selector reconstroem o namespace `4xxx` antes
da consulta exata; assim o mesmo traje aparece na seleção e depois do
`EnterWorld`. Partes vazias são omitidas sem deslocar as partes seguintes.

O manifesto `Costumes-KR.json` preserva a classificação corporal extraída do
client: `EF_CLASS=5` para TK/BM, `EF_CLASS=10` para FM/HT e `EF_CLASS=15`
somente para os renderers realmente dinâmicos. Cada uma das seis partes mantém
sua própria combinação de mesh e textura.

No fluxo nativo, a primeira iteração carrega `ch0101<tipo>.msh`; portanto a
parte 0 pode conter geometria real. O seletor marca exclusivamente os trajes
KR com o bit `0x4000` de `m_nCosType`; o renderer remove essa marca antes de
consultar a tabela. Isso permite carregar `part0` sem interceptar personagens
ou NPCs nativos que reutilizam o mesmo número de tipo. Quando a entrada
`part0` está explicitamente vazia, a face nativa continua sendo preservada.

`Patch-WYD748-Costumes.ps1` possui guarda de SHA e assertions dos três hooks.
O renderer KR também possui uma fronteira seletiva de culling. O 7.48 usa
`D3DCULL_CW/CCW` nas skins nativas e o renderer KR atual desenha certas meshes
esqueletais com `D3DCULL_NONE`. A seleção usa a mesma marca exclusiva e nunca
modifica skins nativas apenas pelo número do renderer.

O SHA final suportado é:

```text
4A2AA37228A720ED389F5AC8A5978329855932B93E54FA0501B51A3A23316DEF
```

`Patch-WYD748-CostumeItems.ps1` materializa os 135 registros no `ItemList.bin`,
reutilizando o ícone 36 do Uniform. `Patch-WYD748-CostumeTextures.ps1` registra
as texturas e seus modos na tabela fixa do 7.48. Copiar um `.wys` sem esse
registro deixa a geometria visível, mas sem as cores. O importador lê o modo do
registro KR de 528 bytes em `@510`; copiar o byte `@255` do KR grava parte do
segundo path no lugar do modo. Os materiais `A`, `C`, `N` e `a` comprovados em
`@510` são preservados. A tabela contém 226 pathnames únicos, sendo 50 de
montarias. O teste
`Test-WYD748-Costumes.ps1` confere seção, tabelas, registros e todos os assets.

O catálogo KR possui outros `96` itens, mantidos no manifesto como
`available=false`: os dois clients fornecidos não contêm todas as dependências
que o próprio executável deles referencia. Eles não são expostos nas lojas,
pois habilitá-los produziria partes ausentes. Os trajes importados permanecem
permanentes até existir calendário autoritativo de expiração no servidor.

## Montarias visuais do client KR

O sétimo elo materializa as `47` aparências que o `SetMountCostume` do client
KR atual realmente resolve: itens `4190..4235` e `4241`. Elas são montarias
independentes do 7.48 baseadas no registro `342` (Shire): mesmo ícone, mesmo
slot `Equip[14]` e mesmos efeitos. Somente a malha, a textura e a escala mudam.
O sistema moderno de traje temporário de montaria não foi importado.

`Patch-WYD748-KRMounts.ps1` instala a seção `.mountkr` e sete adapters no caminho
nativo 7.48: um para a materialização completa e outro para o `UpdateEquip 0x36B`.
Assim, equipar ou desequipar uma montaria atualiza o personagem imediatamente,
sem depender de uma reconstrução visual posterior. O terceiro aplica os
offsets de assento comprovados para os tipos modernos 29, 48, 49/52 e 50.
O quarto marca somente o `TMSkinMesh` cuja assinatura visual completa existe
na tabela KR e reutiliza o `D3DCULL_NONE` seletivo do elo anterior. Assim faces
internas/externas das meshes modernas deixam de desaparecer sem modificar o
estado de culling das montarias antigas.
Os três adapters restantes ampliam o mesmo predicado nativo usado pela tecla
`R`: toggle, estado do botão e velocidade/animação. Uma montaria importada só
é admitida quando `m_cMount == 1` e tipo, escala e os três pares mesh/skin
coincidem integralmente com uma entrada da tabela `.mountkr`; as quatro
famílias nativas continuam no caminho original byte a byte.
`Patch-WYD748-KRMountItems.ps1` clona a Shire no `ItemList.bin` e
`Patch-WYD748-KRMountAssets.ps1` instala os skeletons/animações ausentes sem
sobrescrever assets antigos. `Test-WYD748-KRMounts.ps1` confere os 47 registros,
a seção PE, as tabelas e os arquivos necessários.

Os tuples visuais vêm dos cases nativos `11..56` e `62` dos dois executáveis
KR, que produzem a mesma combinação de tipo, escala, mesh, skin e sanction. As
associações antigas por semelhança de nome foram removidas. Dependências
ausentes nos dois pacotes fornecidos não são escondidas: `hs010117.wys`
(Arvak) usa `hs010115.wys`, da mesma geometria `hs01/mesh14`; o par ausente
`KK010112.msh/.wys` (case 56) usa o par `KK010113` do mesmo skeleton. Esses
fallbacks são declarados no instalador e devem ser substituídos pelos arquivos
KR autênticos quando forem recuperados.

SHA-256 final suportado:

```text
F6F99CC0405654629D9867C84F6587B2064B30D58F67A2151E1ACD36F394E72D
```

Os outros `46` itens não vazios do intervalo moderno não possuem case visual
no executável KR atual; por isso não foram associados a meshes por suposição.
As montarias entregues estão em `ShopKRMt01` e `ShopKRMt02` (27 + 20 itens).

## Arquivos

| arquivo | papel |
| --- | --- |
| `WYD.exe` | em uso; após o último elo, SHA final impressa pelo script |
| `WYD.original.exe` | original intocado |
| `WYD.pre-extended-stats.exe` | linha‑base (entrada do patch wide) |
| `WYD.patched-wide.exe` | cópia de segurança do estado atual |
| `WYD.pre-lindy.exe` | backup da entrada imediata do patch da Lindy |
| `WYD.pre-costumes-kr.exe` | backup da entrada imediata do patch da coleção KR |
| `WYD.pre-mounts-kr.exe` | backup da entrada imediata do patch de montarias KR |

**Regra:** todo patch novo no executável vira script com guarda de SHA. Edição
manual não documentada é exatamente o que custou esta investigação — e o fix de
macro, que se perdeu sem ninguém notar.

## Compatibilidade de clique do Warrior's Seal

O servidor continua lendo `data/itemlist.csv` com `EF_VOLATILE=199` para o
item `4146` e o handler server-side continua sendo `grant_counter` (fama).
O problema ocorria antes do servidor: as rotinas antigas de inventario do
client 7.48 consultam somente a allow-list de volatiles clicaveis. Como `199`
nao esta nessa lista, o botao direito nao chama a rotina nativa `UseItem` e
nenhum pacote `0x373` e enviado.

`Patch-WYD748-ClientItemUse.ps1` corrige somente o marcador local do registro
4146 no `ItemList.bin`: preserva o tipo `EF_VOLATILE` e altera o valor visual
do client de `199` para o marcador clicavel nativo `1`. O servidor nao recebe
esse valor como regra de jogo; ele recebe o slot/item e resolve novamente
`4146 -> 199` de forma autoritativa. O script exige `6500 x 140 + 4` bytes,
confirma o `EF_VOLATILE` no registro correto, cria
`ItemList.pre-warrior-seal.bin` e recusa sobrescrever uma lista em estado
desconhecido. O carimbo de quatro bytes no final do arquivo e preservado: o
reader nativo apenas o le, enquanto os dois checksums `BASE_GetSum2` ja sao
ignorados pelos bypasses da cadeia.

O ajuste e aplicado automaticamente por `Apply-WYD748.ps1` mesmo quando o
`WYD.exe` ja esta na SHA final. Para conferir sem escrever:

```powershell
cd client748
.\Apply-WYD748.ps1 -VerifyOnly
```

## Water: automação somente server-side

O antigo sexto elo `Patch-WYD748-WaterMacro.ps1` foi removido. Os
comandos locais `/macropergaon` e `/macropergaoff`, o hook de chat, o
scanner de Carry e a chamada automática da rotina nativa `UseItem` não
fazem mais parte do executável suportado.

O cliente versionado termina no hash pós-montarias KR
`F6F99CC0405654629D9867C84F6587B2064B30D58F67A2151E1ACD36F394E72D`.
`Apply-WYD748.ps1` reconhece os hashes antigos do WaterMacro, preserva
uma cópia `WYD.pre-server-water.exe` e reconstrói o executável desde
`WYD.original.exe` pelos sete elos atuais.

O encadeamento automático da Water pertence ao servidor: quando uma
sala concede e persiste o próximo pergaminho no Carry do líder, uma
Silver Angel (`3914`) ativa no `Equip[13]` autoriza o uso automático
daquele **UID exato**. Inventário cheio conserva o drop no chão e não
avança; falha do segundo commit conserva o pergaminho durável. Nenhum
pacote `0x373` é fabricado pelo servidor.

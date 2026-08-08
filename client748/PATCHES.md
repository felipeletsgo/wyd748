# Cadeia de patches do client 7.48

O `WYD.exe` em uso **não** é o executável original: ele é o resultado de cinco
scripts aplicados em ordem. Cada elo é verificado por SHA‑256, e a cadeia
inteira reproduz o binário em uso **bit a bit**.

```
WYD.original.exe              B545EA10…619C   client 7.48 de 23/04/2007
  └─ Patch-WYD748.ps1                         câmera/mouse, cores de dano
WYD.pre-extended-stats.exe    2AA1773A…21EE   ← LINHA-BASE
  └─ Patch-WYD748-ExtendedStats.ps1           stats 32-bit, seção .xstat
  (intermediário)             4724B76F…45D6
  └─ Patch-WYD748-Bypass.ps1                  versão + 2 checksums
  (intermediário)             F2B8CDEB…0C5B
  └─ Patch-WYD748-Macro.ps1                   rotação de skills + buffs
WYD.exe pré-Lindy             B2678AB9…2F94   ← binário atualmente versionado
  └─ Patch-WYD748-Lindy.ps1                  validação 4010 -> 3448
WYD.exe pós-Lindy             4643F1B6…0512
  └─ Patch-WYD748-WaterMacro.ps1             macro local de pergaminhos
WYD.exe pós-WaterMacro        SHA impressa pelo script
```

## Reaplicar

```powershell
cd client748
Copy-Item WYD.pre-extended-stats.exe WYD.exe -Force
.\Patch-WYD748-ExtendedStats.ps1
.\Patch-WYD748-Bypass.ps1
.\Patch-WYD748-Macro.ps1
.\Patch-WYD748-Lindy.ps1
.\Patch-WYD748-WaterMacro.ps1
```

A ordem **importa**: cada script valida o SHA de entrada e recusa rodar fora de
posição. O fix da Lindy é o penúltimo elo e foi escrito contra a saída do
macro. Para reaplicar, o primeiro passo deve restaurar
`WYD.pre-extended-stats.exe` em `WYD.exe`; o script da Lindy mantém
`WYD.pre-lindy.exe` como backup da entrada imediata.

O script de stats também restaura a linha‑base sozinho quando o SHA de entrada
não bate, então rodá-lo duas vezes é seguro.

## Os três bypasses — leia antes de mexer

`Patch-WYD748-Bypass.ps1` existe porque estes três bytes estavam no executável
em uso e **em nenhum script**. Foram editados à mão e nunca documentados.

| offset | o que faz | sintoma se faltar |
| --- | --- | --- |
| `0x07E44A` | ignora a checagem de **versão** do client | não conecta |
| `0x14F9E8` | ignora o **checksum** de um arquivo de dados | não carrega |
| `0x14FBFC` | ignora o **checksum** de outro arquivo de dados | ItemList editado é rejeitado |

Nos três, um `74 04` (JZ) vira `EB 04` (JMP): o salto passa a ser incondicional
e o `33 C0` (`xor eax,eax`) logo à frente — o caminho de falha — nunca executa.

Foram descobertos ao reconstruir a linha‑base: eram a **única** diferença entre
o binário em uso e a saída do patch wide. Sem este script, qualquer regeneração
do client os perderia em silêncio.

## Fix de macro (aplicado em 25/07/2026)

`Patch-WYD748-Macro.ps1` corrige a rotação do macro (teclas A/D): o cooldown por
skill era **lido** em `0x00493959` mas nunca **escrito**, então o primeiro slot
ficava sempre "pronto", disparava todo frame, e a rotação nunca avançava até os
buffs. Sintoma: *só 1 skill, sem rotação, sem buff*.

Ele grava o cast em `DAT_0092eaf8[skillID*4]`. Ataques registram o instante
atual; buffs registram `agora + 150000 ms - cooldown`, de forma que a próxima
checagem automática aconteça exatamente aos 150 segundos — 30 segundos antes
do fim nominal de 180 segundos. O clique manual não passa por esse hook.

A implementação atual usa a área executável livre da seção `.xstat`
(`0x013C0207`) e uma tabela de 96 bits com os buffs reconhecidos pelo macro. O
hook em `0x00493970` retorna exatamente para `0x00493975`. Isso substitui a
primeira code cave encadeada, cujo retorno incorreto para `0x00497975` fazia o
macro mágico tratar o mob ID `1000` como ponteiro e crashar em `0x004979D3`.

SHA-256 atual do `WYD.exe`:
`B2678AB927F03BF0F3114F36AE682025A9C732D2A59B55FB5B26DECEE07F2F94`.

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

## Fix da Lindy (penúltimo elo)

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
`4643F1B6B8E67F375955D2F57AEFD0A6997E41DDB5F2B3CE7E2B5C401D200512`.

O script local recebido como `Patch-WYD748-Lindy-Local.ps1` não é o elo
versionado: ele aceita qualquer executável com os seis bytes antigos. Para
reprodução pública da cadeia, use o script estrito acima.

## Arquivos

| arquivo | papel |
| --- | --- |
| `WYD.exe` | em uso; após o último elo, SHA final impressa pelo script |
| `WYD.original.exe` | original intocado |
| `WYD.pre-extended-stats.exe` | linha‑base (entrada do patch wide) |
| `WYD.patched-wide.exe` | cópia de segurança do estado atual |
| `WYD.pre-lindy.exe` | backup da entrada imediata do patch da Lindy |
| `WYD.pre-water-macro.exe` | backup da entrada pós-Lindy do patch Water |

**Regra:** todo patch novo no executável vira script com guarda de SHA. Edição
manual não documentada é exatamente o que custou esta investigação — e o fix de
macro, que se perdeu sem ninguém notar.

## Water Scroll Macro (elo independente)

`Patch-WYD748-WaterMacro.ps1` é aplicado **depois** do elo da Lindy. Ele exige
exatamente a SHA pós-Lindy
`4643F1B6B8E67F375955D2F57AEFD0A6997E41DDB5F2B3CE7E2B5C401D200512`, verifica
os bytes originais dos dois hooks e usa somente a faixa zero livre
`0x1D3243..0x1D37FF` da seção `.xstat`. A cave existente do macro de skills em
`0x1D3207` não é tocada.

O gerador `tools/watermacrotable` carrega `data/volatiles.json` pelo loader Go
autoritativo e materializa, em ordem determinística, os 27 tickets Water e suas
`EntryAreas`. O patch não mantém IDs ou coordenadas duplicados. Cada registro
contém `item:uint16`, `areaCount:uint16` e retângulos inclusivos
`minX,minY,maxX,maxY:uint16`.

O hook de chat em `VA 0x004678E4` reconhece apenas
`/macropergaon` e `/macropergaoff` sem diferenciar maiúsculas/minúsculas. A
mensagem é local (`Water scroll macro enabled.`/`disabled.`) e o texto não é
transmitido ao servidor. Todo outro texto segue o formatter original.

O hook do tick em `VA 0x004779A7` chama o scanner no mesmo thread do cliente e
depois chama o macro A/D nativo. O scanner percorre `Carry[0..62]` (a grade
real é 8 páginas de 9 células), usa a posição local refletida pelo servidor,
seleciona o primeiro Water Scroll cuja área contém a posição e chama
`FUN_00465F85` — a mesma rotina de uso manual — sem criar packet próprio. A
janela local de 250 ms reduz spam; as validações de item, área, sequência,
instância e persistência continuam exclusivamente no servidor.

Para verificar a cadeia sem alterar o binário versionado:

```powershell
cd client748
.\Test-WYD748-WaterMacro.ps1
```

O teste aplica Lindy + Water em uma cópia temporária, confirma os dois CALLs,
os destinos dentro da cave, os dois saltos para o retorno tratado do parser,
a chamada da rotina nativa de `UseItem`, as strings locais e a cave, e imprime
a SHA final. A saída observada nesta revisão foi
`ED90138BC55EBE731CABCFCFF4F73918EBECFC16EBE554DA103B0633DFB8CF0E`.

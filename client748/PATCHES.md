# Cadeia de patches do client 7.48

O `WYD.exe` em uso **não** é o executável original: ele é o resultado de uma
cadeia de scripts aplicada em ordem. Cada elo valida o estado de entrada antes
de escrever. Patches novos no executável devem continuar seguindo esta regra.

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
  └─ Patch-WYD748-Lindy.ps1                   validação 4010 -> 3448
WYD.exe pós-Lindy             (o script imprime a SHA-256 resultante)
```

## Reaplicar

```powershell
cd client748
Copy-Item WYD.pre-extended-stats.exe WYD.exe -Force
.\Patch-WYD748-ExtendedStats.ps1
.\Patch-WYD748-Bypass.ps1
.\Patch-WYD748-Macro.ps1
.\Patch-WYD748-Lindy.ps1
```

A ordem **importa**: cada script valida o SHA de entrada e recusa rodar fora de
posição. O fix da Lindy é o último elo atual e foi escrito contra a saída do
macro.

O script de stats também restaura a linha-base sozinho quando o SHA de entrada
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

Foram descobertos ao reconstruir a linha-base: eram a **única** diferença entre
o binário em uso e a saída do patch wide. Sem este script, qualquer regeneração
do client os perderia em silêncio.

## Fix da Lindy — material hardcoded no executável

A janela de combinação da Lindy não obtém toda a receita de um arquivo de dados.
Parte da validação está compilada diretamente no `WYD.exe`.

Na descompilação do client 7.48, em `VA 0x00413FB7` / file offset `0x13FB7`, a
rotina compara o primeiro material com **4010**:

```text
81 FA AA 0F 00 00    cmp edx,4010
```

A receita implementada no servidor e a referência-alvo 7.54 usam **dois pacotes
3448 com EF_AMOUNT=10**. Portanto o cliente recusava a combinação correta antes
de ela chegar ao servidor.

`Patch-WYD748-Lindy.ps1` altera somente a constante:

```text
81 FA 78 0D 00 00    cmp edx,3448
```

O script:

- exige a SHA-256 pré-Lindy `B2678AB927F03BF0F3114F36AE682025A9C732D2A59B55FB5B26DECEE07F2F94`;
- confirma os seis bytes originais antes de escrever;
- cria `WYD.pre-lindy.exe` como backup;
- confirma os seis bytes novos após escrever;
- imprime a SHA-256 final.

Os compares próximos encontrados na mesma rotina — `4127`, `413` e `4141` —
**não são alterados**. Eles pertencem a outros ramos e não há evidência de que
sejam defeitos.

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

SHA-256 pré-Lindy do `WYD.exe` após o macro:
`B2678AB927F03BF0F3114F36AE682025A9C732D2A59B55FB5B26DECEE07F2F94`.

O macro estava **ausente** do binário em uso — os bytes em `0x093970` eram
idênticos aos do original. O script existia e estava bem documentado, mas
perdeu-se numa regeneração anterior, quando a cadeia ainda não estava escrita.
Foi o que motivou este arquivo.

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

## Arquivos

| arquivo | papel |
| --- | --- |
| `WYD.exe` | binário versionado, ainda pré-Lindy até regeneração local |
| `WYD.original.exe` | original intocado |
| `WYD.pre-extended-stats.exe` | linha-base (entrada do patch wide) |
| `WYD.patched-wide.exe` | cópia de segurança do estado atual, quando presente |
| `WYD.pre-lindy.exe` | backup criado pelo patch da Lindy |

**Regra:** todo patch novo no executável vira script com guarda de SHA. Edição
manual não documentada é exatamente o que custou esta investigação — e o fix de
macro, que se perdeu sem ninguém notar.

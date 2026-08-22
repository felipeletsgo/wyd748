# Ghidra — client nativo WYD 7.48

Esta referência governa qualquer port do executável nativo para
`client-source/tmproject`. O objetivo é reproduzir o comportamento comprovado do
7.48 sem transportar cegamente ABI de versões posteriores.

## Artefatos canônicos

O binário de referência é:

```text
client748/WYD.exe
SHA-256: 8AA2F918844BCE3AFE21F1204F69757A443E32EB2F2F616936B1D9BFE215F593
```

Os artefatos Ghidra ficam fora do Git e devem ser descobertos no perfil do
usuário, sem gravar caminho absoluto da máquina no código:

```powershell
$ghidraProject = Get-ChildItem "$env:USERPROFILE\Tools\GhidraProjects" `
    -Filter 'WYD748Native_*.gpr' | Sort-Object LastWriteTime -Descending | `
    Select-Object -First 1

$decompRoot = Get-ChildItem "$env:USERPROFILE\Tools\GhidraAnalysis" `
    -Directory | Sort-Object Name -Descending | ForEach-Object {
        Join-Path $_.FullName 'decompiled'
    } | Where-Object { Test-Path $_ } | Select-Object -First 1
```

O export atual contém `functions.tsv` e um `.c` por função. Use o projeto Ghidra
quando xrefs, tipos, stack, calling convention ou call graph forem necessários;
use o export para busca rápida em massa.

## Procedimento obrigatório

1. Calcular o SHA-256 de `client748/WYD.exe` e comparar com o valor acima. Se
   divergir, não reutilizar endereços silenciosamente: reanalisar o novo binário.
2. Localizar o comportamento por string, opcode, tamanho, constante, import ou
   símbolo relacionado no export e no Ghidra.
3. Abrir a função nativa e conferir callers e callees, não apenas o bloco que
   parece relevante.
4. Mapear estado inicial, mutações, side effects, lifecycle e condições de erro.
5. Para wire/ABI, registrar tamanho, offsets, signedness, packing e direção
   C→S/S→C. Confirmar com `static_assert` ou teste byte a byte na source.
6. Comparar com TMProject/W2PP. Aproveitar melhorias semânticas da versão mais
   nova somente quando elas puderem ser projetadas com segurança no contrato
   comprovado do 7.48.
7. Portar o comportamento para código legível. Não copiar nomes temporários,
   endereços virtuais ou pseudocódigo desestruturado como arquitetura final.
8. Inserir comentário explicativo junto a todo código editado, registrando por
   que a compatibilidade 7.48 exige aquele comportamento.
9. Compilar com `client-source/tmproject/Build-Client.ps1`, instalar como
   `client748/project.exe` e executar o fluxo real afetado.

## Evidência mínima por área

### Packets e handlers

- validador nativo de opcode/tamanho;
- construtor ou parser nativo;
- direção do packet;
- transição de estado que habilita o handler;
- bytes esperados e `static_assert`/teste correspondente.

### UI e input

- função que cria os controles;
- IDs dos controles e estado inicial visível/oculto;
- função de input/click/tecla;
- callback e condição que abre/fecha a janela;
- ownership e ordem de destruição para evitar crash.

### Personagem, item e render

- struct realmente consumida pelo 7.48;
- selector de mesh/face/item;
- recursos e índices carregados;
- renderer e condições de fallback;
- lifecycle de criação, atualização e destruição.

## Funções já identificadas

Use estes pontos como âncoras, sempre reconfirmando no projeto Ghidra:

```text
FUN_0055890a  validador nativo de opcode e tamanho
FUN_00524bbb  construção do movimento C→S 0x366
FUN_00520216  construção/validação da rota local
FUN_0055f2dd  throttle/envio
FUN_00492e7d  input e dispatch de campo
FUN_00489a3e  dispatch de packets recebidos
FUN_00441823  criação/inicialização principal da UI de campo
```

## Proibições

- Não declarar paridade com base apenas em compilação ou imagem semelhante.
- Não portar `sizeof`, packing, offsets ou IDs do TMProject 7.59 sem prova 7.48.
- Não corrigir UI apenas deslocando coordenadas se o lifecycle/ID do controle
  ainda divergir.
- Não aceitar packet moderno no servidor só para mascarar um construtor errado
  no client recompilado.
- Não apagar compatibilidade do client distribuído antes do `project.exe` passar
  pelo mesmo fluxo in-game.

## Saída esperada da investigação

Antes do patch, deixar uma matriz concisa:

```text
comportamento | função 7.48 | contrato nativo | TMProject atual | correção
```

Depois do patch, classificar o estado como:

```text
IMPLEMENTED
STATICALLY VERIFIED
AUTOMATED TESTED
CLIENT-TESTED
```

Somente usar `CLIENT-TESTED` depois da execução real do fluxo afetado.

# Ghidra — client nativo WYD 7.48

Esta referência governa claims de paridade e mudanças que interceptam uma
fronteira legada em `client-source/tmproject`. O objetivo é reproduzir o que
precisa permanecer 7.48 sem impedir modernizações internas ou extensões
coordenadas client/server.

## Artefatos canônicos

Os executáveis têm papéis distintos:

```text
client748/wyd.exe nativo+patches/WYDoriginal.exe  stock histórico imutável
client748/wyd.exe nativo+patches/WYD.exe          referência histórica Ghidra
client748/project.exe                            único candidato executável e de validação
```

Hashes confirmados no snapshot de 23/08/2026:

```text
WYDoriginal.exe  B545EA104DE50641E820F00B6BC54E4B2B14583ED75C7DCEC06F50BA5042619C
WYD.exe          8AA2F918844BCE3AFE21F1204F69757A443E32EB2F2F616936B1D9BFE215F593
project.exe      variável por build; registrar o hash em cada validação
```

Os scripts e patches em `client748/wyd.exe nativo+patches/` são somente material
histórico. Não executá-los nem editá-los. O build de `client-source/tmproject`
deve ser copiado/renomeado diretamente para `client748/project.exe`, sem patch;
toda alteração ativa pertence à source ou aos assets.

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

1. Conferir o fingerprint registrado de
   `client748/wyd.exe nativo+patches/WYD.exe`. Reutilizar o SHA-256 confirmado se
   caminho, tamanho e mtime não mudaram; recalculá-lo quando esses inputs
   mudarem ou o registro for insuficiente. Se divergir, não reutilizar endereços
   silenciosamente: reanalisar o novo binário.
2. Localizar o comportamento por string, opcode, tamanho, constante, import ou
   símbolo relacionado no export e no Ghidra.
3. Abrir a função nativa e conferir callers e callees, não apenas o bloco que
   parece relevante.
4. Mapear estado inicial, mutações, side effects, lifecycle e condições de erro.
5. Para wire/ABI, registrar tamanho, offsets, signedness, packing e direção
   C→S/S→C. Confirmar com `static_assert` ou teste byte a byte na source.
6. Comparar com a source atual e TMProject. Preservar melhorias manuais e
   adotar estrutura posterior superior quando ela mantiver a fronteira legada
   ou quando client e servidor definirem deliberadamente um contrato novo.
7. Portar o comportamento para código legível. Não copiar nomes temporários,
   endereços virtuais ou pseudocódigo desestruturado como arquitetura final.
8. Inserir comentário apenas junto a decisões não óbvias de contrato,
   compatibilidade ou ownership.
9. Compilar com `client-source/tmproject/Build-Client.ps1`, confirmar a
   instalação e o hash automáticos de `client748/project.exe` e executar o fluxo
   real afetado.

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

Além do código, confirmar que cada ID é materializado no recurso de UI carregado.
Um membro herdado do TMProject 7.59 pode ficar legitimamente nulo no 7.48. Nesse
caso, proteger todos os acessos e preservar o lifecycle principal; não criar um
widget moderno apenas para satisfazer o ponteiro.

### Personagem, item e render

- struct realmente consumida pelo 7.48;
- selector de mesh/face/item;
- recursos e índices carregados;
- renderer e condições de fallback;
- lifecycle de criação, atualização e destruição.

## Funções já identificadas

Use estes pontos como seeds. Reconfirmar no projeto Ghidra quando o binário,
corpus, função candidata ou claim dependente tiver mudado:

```text
FUN_0055890a  validador nativo de opcode e tamanho
FUN_00524bbb  construção do movimento C→S 0x366
FUN_00520216  construção/validação da rota local
FUN_0055f2dd  throttle/envio
FUN_00492e7d  input e dispatch de campo
FUN_00489a3e  dispatch de packets recebidos
FUN_00441823  criação/inicialização principal da UI de campo
```

## Triagem de crash com minidump e ASLR

Para mapear um endereço carregado ao endereço preferido usado no Ghidra:

```text
RVA = endereço_da_falha - base_carregada_do_módulo
VA_preferido = ImageBase_preferido + RVA
```

Registrar dump, exceção, módulo, base carregada, RVA, image base e hash exato do
executável. Depois desassemblar a instrução, identificar o registrador/ponteiro
inválido e mapear o acesso à linha da source; não concluir apenas pelo endereço.

Exemplo confirmado em 23/08/2026: `0x009A1F46` com módulo carregado em
`0x008B0000` resulta em RVA `0x000F1F46` e VA preferido `0x004F1F46`. A instrução
fazia chamada virtual com `ecx == 0`, correspondente a acesso direto a
`pFScene->m_pMHPBar` durante materialização do personagem.

Exemplo confirmado em 26/08/2026: o dump
`client748/client-crash-20260826-154302.dmp` falhou em
`SGridControl::TradeItem`, RVA `0x000718AC`, ao desreferenciar
`m_pChatSelectPanel`. A UI 7.48 do AutoTrade usa painel 626, edit 627, caption
630 e botão 667, sem o seletor de chat herdado da 7.59. A regra extraída desse
caso é global: depois de identificar um controle opcional ausente, localizar
todos os seus acessos, inclusive erro, reentrada e fechamento; proteger somente
o frame que apareceu no primeiro dump deixa crashes equivalentes ativos.

Os controles modernos `T_CURRENT_MHP=65618`, `T_MAX_MHP=65619`,
`P_MHP_PROGRESS=65625` e `P_MHP_PROGRESS_TR=65626` não existem no dump atual da
UI 7.48, nem como IDs truncados para 16 bits (`82`, `83`, `89`, `90`). Portanto,
os bindings podem ser nulos por contrato: updates de score/equip/montaria devem
testá-los sem impedir a entrada no mundo ou a criação do personagem.

Ausência comprovada de um controle moderno não autoriza fabricar ID, truncar o
ID para 16 bits ou instanciar um widget substituto. Preservar a transição nativa
principal e tornar todos os paths auxiliares tolerantes a nulo.

## Proibições

- Não declarar paridade com base apenas em compilação ou imagem semelhante.
- Não portar `sizeof`, packing, offsets ou IDs do TMProject 7.59 sem prova 7.48.
- Não remover função, asset ou estrutura manual apenas porque não existe no
  nativo 7.48; ausência pode representar uma extensão intencional.
- Não exigir equivalente nativo para contrato novo explicitamente coordenado;
  exigir em seu lugar especificação e teste client/server.
- Não corrigir UI apenas deslocando coordenadas se o lifecycle/ID do controle
  ainda divergir.
- Não aceitar packet moderno no servidor só para mascarar um construtor errado
  no client recompilado.
- Não promover uma adaptação antes do `project.exe` passar pelo fluxo in-game
  afetado; o binário histórico permanece somente referência e nunca fallback.
- Não executar nem editar o patcher binário legado; ele é evidência histórica.

## Saída esperada da investigação

Antes da alteração, deixar uma matriz concisa:

```text
modo | comportamento | função/fronteira 7.48 | source atual | contrato server | decisão
```

Depois da alteração, classificar o estado como:

```text
IMPLEMENTED
STATICALLY VERIFIED
AUTOMATED TESTED
CLIENT-TESTED
```

Somente usar `CLIENT-TESTED` depois da execução real do fluxo afetado.

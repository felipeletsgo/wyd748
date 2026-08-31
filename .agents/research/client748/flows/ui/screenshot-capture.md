---
id: screenshot-capture
title: Captura de tela por Print Screen em JPG
subsystem: ui-input
status: TRACED
native_sha256: 8AA2F918844BCE3AFE21F1204F69757A443E32EB2F2F616936B1D9BFE215F593
updated: 2026-08-31
---

# Captura de tela por Print Screen em JPG

## Pergunta

Como o client 7.48 trata Print Screen, escolhe o nome do arquivo e materializa
a captura final na pasta `ScreenShot`?

## Fronteira de evidência

- Executável: `client748/wyd.exe nativo+patches/WYD.exe`, identificado pelo
  SHA-256 do frontmatter.
- Projeto/corpus Ghidra: `WYD748Native_20260821.gpr` e
  `%USERPROFILE%\Tools\GhidraAnalysis\20260821\decompiled`.
- Raiz nativa: `FUN_00427AA8`, decompilada em
  `00427aa8_FUN_00427aa8.c`.
- Caller: `FUN_0055DAB8`, dispatcher de mensagens da aplicação.
- Source atual: `NewApp.cpp` e `D3DDevice.cpp` em
  `client-source/tmproject/Projects/TMProject/`.
- Referência oficial recuperada pelo scraping:
  `https://www.wyd2.co.kr/guide/firstguide02_05.asp`, sintetizada em
  `DOCS/GUIA/GUIAS_JOGABILIDADE.md`.

O guia confirma o resultado observável, mas não substitui o fluxo nativo. Não
há packet, ABI, recurso de layout ou regra server-side neste recorte.

## Fluxo nativo 7.48

### Entrada observável

`FUN_0055DAB8` recebe `WM_KEYUP` (`0x101`). Quando a tecla é `VK_SNAPSHOT`
(`0x2C`) e a aplicação possui o receptor ativo, chama `FUN_00427AA8`. A source
recompilável conserva a mesma entrada em `NewApp.cpp` e chama
`D3DDevice::CaptureScreen()`.

### Callers

O inventário Ghidra e o corpus registram um único caller direto:
`FUN_0055DAB8`. Não há callback de controle, timer, worker ou packet no fluxo.

### Função principal

`FUN_00427AA8` começa em `ScreenShot\Capture0000.jpg` e testa a existência do
arquivo. Enquanto o nome já existe, fecha o handle, incrementa o contador e
forma o próximo nome até o limite observado. Para capturar, também forma um
nome BMP temporário, obtém o backbuffer do dispositivo, salva a superfície e
chama `BMP2JPG`; o contrato final exposto ao jogador é o arquivo JPG.

O guia oficial confirma a pasta `ScreenShot` e o padrão `capture0000.jpg`. A
diferença de maiúsculas é indiferente no filesystem Windows suportado pelo
client.

### Callees

- `FUN_0058F078`: formata os nomes JPG e BMP.
- `FUN_0058FB07` / `FUN_0058F75F`: abre e fecha o candidato para detectar
  colisão.
- `FUN_0055FE5B`: salva o backbuffer no BMP temporário.
- `BMP2JPG`: converte o temporário para o JPG final.
- `FUN_00431159`: registra `Can't Capture Screen` quando a gravação falha.
- A virtual `+0x40` do dispositivo obtém a superfície; a virtual `+0x08` da
  superfície libera a referência.

### Saídas e erros

- Sucesso: cria o primeiro `ScreenShot\Capture%04d.jpg` ainda inexistente.
- Colisão: incrementa o contador e não sobrescreve o JPG anterior.
- Falha de gravação: registra a mensagem de erro e ainda libera o backbuffer.
- A pasta precisa existir; nem o nativo nem a source atual criam o diretório
  dentro da função de captura.

## Estado e lifecycle

### Matriz de transições

| Evento/estado | Precondição | Função/call | Estado resultante | Side effects | Erro/saída |
| --- | --- | --- | --- | --- | --- |
| `WM_KEYUP/VK_SNAPSHOT` | aplicação e dispositivo ativos | `FUN_0055DAB8 -> FUN_00427AA8` | captura concluída | cria JPG numerado | log em falha de gravação |
| nome já existe | JPG anterior presente | open/close e incremento | próximo nome selecionado | nenhum overwrite | busca limitada |
| superfície salva | backbuffer adquirido | save, conversão e release | arquivo final disponível | libera referência D3D | erro é registrado |

### Vtables, vptrs e receptores

O receptor é a aplicação em `FUN_0055DAB8`; a obtenção do backbuffer ocorre
pela virtual `+0x40` do dispositivo e o release pela virtual `+0x08` da
superfície. A adaptação usa a API tipada do Direct3D 9 já adotada pela source e
não transporta offsets ou vptrs nativos.

### Ownership

O dispositivo pertence a `D3DDevice`. A superfície retornada por
`GetBackBuffer` é uma referência temporária, liberada no mesmo ciclo. O nome é
buffer local e o arquivo passa a pertencer ao filesystem; não há estado de
gameplay ou ownership server-side.

### Falha parcial

A função não publica estado intermediário na aplicação. Na implementação
nativa, o BMP é transitório antes da conversão. A source pode produzir o JPG
diretamente com `D3DXSaveSurfaceToFile`, evitando esse intermediário sem mudar
nome, formato final, input ou ownership.

### Cleanup e teardown

O handle usado na detecção de colisão é fechado antes de cada incremento e o
backbuffer é liberado após a tentativa de gravação. Não há callback registrado
nem objeto persistente a desmontar.

### Shutdown

Não aplicável: a captura é síncrona e não conserva recurso após retornar. O
dispatcher só a chama enquanto a aplicação e o dispositivo estão ativos.

### Logout e relogin

Não aplicável: a numeração depende apenas dos arquivos já presentes e funciona
independentemente da cena, conta ou personagem.

## Wire, ABI e recursos

Não existe wire/ABI de rede neste fluxo. `ScreenShot\Capture%04d.jpg` é o
contrato de filesystem confirmado pelo nativo e pelo guia oficial. O formato
JPG já é suportado pelo D3DX usado pela source (`D3DXIFF_JPG`).

## Mapeamento atual

### Source recompilável

`NewApp.cpp` já reproduz `WM_KEYUP/VK_SNAPSHOT`. `D3DDevice::CaptureScreen`
selecionava o primeiro nome BMP livre e gravava `D3DXIFF_BMP`, deixando o
resultado divergente do contrato final nativo. A API atual permite gravar o
JPG diretamente, sem introduzir conversor ou dependência adicional.

### WYD-Go

Não participa da captura. Nenhum dado do screenshot é aceito como autoridade
ou enviado ao servidor.

## Matriz de delta

| Claim | Nativo 7.48 | Source atual | Guia KR | WYD-Go | Decisão |
| --- | --- | --- | --- | --- | --- |
| entrada | `WM_KEYUP/VK_SNAPSHOT` | igual | Print Screen documentado | N/A | manter |
| pasta/nome | `ScreenShot\Capture%04d.jpg` | mesmo padrão, extensão BMP | `ScreenShot/capture0000.jpg` | N/A | portar JPG |
| encoder | BMP temporário + `BMP2JPG` | D3DX grava BMP | só define resultado | N/A | modernizar para D3DX JPG direto |
| colisão | testa JPG existente | testa BMP existente | não detalha | N/A | portar busca por JPG |

## Decisões

- Classificar o resultado JPG e a busca pelo primeiro nome livre como
  `PARIDADE_NATIVA`.
- Classificar a gravação direta `D3DXIFF_JPG` como
  `MODERNIZACAO_COMPATIVEL`: preserva o contrato externo e remove apenas o BMP
  temporário e o conversor intermediário.
- Manter input, pasta, contador, ownership e tratamento de erro existentes.
- Não adicionar criação automática da pasta neste delta porque isso não é
  necessário para corrigir a divergência comprovada e ampliaria o lifecycle.

## Lacunas

- Executar o `project.exe`, pressionar Print Screen duas vezes e confirmar
  `Capture0000.jpg`/`Capture0001.jpg` sem sobrescrita.
- Abrir os arquivos e confirmar dimensões e conteúdo do backbuffer.
- Confirmar o log quando a pasta estiver ausente ou sem permissão de escrita.

## Validação

- Pesquisa: `FUN_0055DAB8`, `FUN_00427AA8`, seus cinco callees internos,
  `BMP2JPG`, entrada, colisão, erro e release rastreados no corpus/inventário
  Ghidra do hash registrado.
- Guia oficial: resultado JPG confirmado na síntese versionada do scraping.
- Validador documental: exit 0; nove fichas válidas (`CONTRACT=4`,
  `LOCATED=3`, `TRACED=2`).
- Build oficial: `client-source/tmproject/Build-Client.ps1` concluiu com zero
  warnings e zero erros e instalou o candidato em `client748/project.exe`.
- SHA-256 do candidato compilado:
  `221AB0DB498D7193A64F884A32F2A7A0086A5CEBE352F3EEED918CE6F7FC4D09`.
- Estado máximo alegado: `STATICALLY VERIFIED`; o build não promove a ficha a
  `CLIENT_TESTED`.
- Client real: não executado; `CLIENT_TESTED` não é alegado.

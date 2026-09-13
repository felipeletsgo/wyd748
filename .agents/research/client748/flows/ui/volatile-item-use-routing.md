---
id: volatile-item-use-routing
title: Uso de volatiles na Field nativa e rotas do catalogo integrado
subsystem: ui
status: CONTRACT
native_sha256: 8AA2F918844BCE3AFE21F1204F69757A443E32EB2F2F616936B1D9BFE215F593
updated: 2026-09-13
---

# Uso de volatiles no TMProject 7.48

## Pergunta

Por que Scroll encerra o client e como encaminhar os itens do catalogo
autoritativo sem confundir consumo direto, alvo, confirmacao e interacao?

## Fronteira de evidência

- Native: `references/client748/WYD.exe`, hash acima; Ghidra
  `C:/Users/felipe/Tools/GhidraProjects/WYD748Native_20260821.gpr`.
- Corpus: `C:/Users/felipe/Tools/GhidraAnalysis/20260821/decompiled`, exports
  `0041ef0f_FUN_0041ef0f.c`, `00465f85_FUN_00465f85.c`,
  `00435b13_FUN_00435b13.c`, `004658a7_FUN_004658a7.c`,
  `00465b05_FUN_00465b05.c`, `004662c5_FUN_004662c5.c`,
  `00416196_FUN_00416196.c` e `00418828_FUN_00418828.c`.
- Source: `tmproject/TMProject748/internal/ui/SGrid.cpp`,
  `internal/app/scenes/TMFieldScene.cpp` e `internal/application/NativeVolatileRoutes.h`.
- Server: `wydgo748/data/volatiles.json`, `data/instances.json`, `data/itemlist.csv`,
  `internal/game/consumables.go`, `item_instances.go`, `celestial_capsule.go`,
  `handlers.go` e testes dos pacotes game/data/wire.
- Reutilizados: fichas `application-close-global-shutdown`, `req-teleport`,
  `special-potion-shortcut` e `equipped-item-matched-consumable-shortcut`.

## Fluxo nativo 7.48

### Callers

Clique direito entra em `FUN_0041ef0f`; abertura do nome chama `FUN_004658a7`.
O callback de confirmacao e `FUN_004662c5`. O initializer da Field e
`FUN_00435b13`; drop/realce de alvo passam por `FUN_00416196`/`FUN_00418828`.

### Callees

- `FUN_0041ef0f` despacha clique direito: Recall 11/13, Portal 195, confirmacao
  187/206/211, desenho 3442 e filtro de consumo. O envio direto chama
  `FUN_00465f85`: `0x373`, origem Carry 1, slot row-major, coordenadas do humano,
  destino mount 0/14 para alimento 15, consumo visual e cooldown de 200 ms.
- `FUN_00435b13` cria os cem botoes do desenho, filhos de 8705, com IDs
  8706..8805, textura 375, passo 25 e tamanho 24; nao sao filhos preexistentes
  no recurso. Modal 5; ownership pertence ao container da Field.
- Capsula com primeiro efeito 59: `FUN_0041ef0f -> FUN_004658a7` abre nome;
  confirmacao `FUN_004662c5 -> FUN_00465b05`. Edit 627, label 630; nome precisa
  de 4..12 bytes, validacao de string e envio `0x3CC` antes da remocao visual.
- `FUN_00416196`/`FUN_00418828` distinguem refinacao 4/5 de repliction 190.
  Nao converter automaticamente o volatile 5 em 190 na Field nativa.

## Estado e lifecycle

Recall mantem o atraso e a intencao `m_stUseItem`; ack `0x3AE` so pode postar
fechamento com saida local pendente. Ver ficha de shutdown atualizada.
Fogos inicializam os cem controles uma vez no initializer ativo, sem ownership
paralelo; abertura/envio toleram recursos ausentes. No envio, a celula deve
conter o item 3442. Capsula revalida celula, tipo 3443, marcador de preenchimento
e cooldown; nome invalido conserva o prompt. Envio invalida o slot pendente.
O inventario continua sendo corrigido pelo estado autoritativo recebido.

## Wire, ABI e recursos

Sem novos opcodes ou layouts: UseItem `0x373` (36 bytes), desenho `0x3C9`
(52 bytes), nome da capsula `0x3CC` (52 bytes), DelayStart/ack `0x3AE` (16 bytes).
O client envia intencoes; servidor resolve item, alvo, requisitos, efeito,
consumo e persistencia. Itens de NPC/comando nao viram consumo de clique.

## Mapeamento atual

`NativeVolatileRoutes.h` classifica UI em Direct, Target, Recall, Portal,
Confirm, Capsule, CustomFirework e Interaction. Overrides por item precedem
o codigo, preservando caixas volatile 0, fogos 3442 e selos sem uso direto.
Codigo desconhecido conserva o filtro anterior. O filtro e aplicado apenas
na Field compativel; as rotas especiais existentes precedem consumo direto.
O teste Go le essa mesma tabela C++ e cruza o catalogo real carregado com
instances: 355 itens, 124 codigos, incluindo regras sobrescritas por item.

## Matriz de delta

| Claim | Defeito confirmado | Decisao |
| --- | --- | --- |
| Scroll / ack | qualquer ack fechava a aplicacao | exigir intencao local de sair |
| filtro | caixas de volatile 0 bloqueadas; classificacao nao verificavel contra catalogo | tabela testavel com overrides e fallback |
| desenho 3442 | initializer compativel retornava antes de criar os botoes | compartilhar inicializacao nativa |
| capsula | minimo de nome invertido, format string e modal descartado na rejeicao | minimo 4/maximo 12, copia literal, conservar prompt |
| refinacao | volatile 5 convertido em repliction 190 no inventario | preservar identidade no modo 7.48 |

## Decisões

- `PARIDADE_NATIVA`: construcao do desenho e limites do nome da capsula,
  respaldados nas funcoes acima. IDs explicitos 627/630 tambem eliminam
  dependencia do alias moderno; o alias ja existia, nao era um crash provado.
- `MODERNIZACAO_COMPATIVEL`: guardas, identidade do item pendente, copia segura,
  preservacao do prompt, ack condicionado e politica integrada ao catalogo.
  Para caixas/ingressos expostos pelo catalogo atual, a autoridade e o contrato
  implementado/testado no servidor, nao uma alegacao nova de clique nativo.
- Nao mudar handlers autoritativos nem tornar itens de alvo/NPC consumiveis
  diretos. Refinacao por drop conserva os requisitos de destino existentes.

## Lacunas

Validar em jogo T e `-`, Scroll/warp, Portal, caixas, transformacao com/sem anel,
refinacao no alvo apropriado, fogos (desenhar/cancelar/enviar) e Selo da Alma
(nome invalido/valido, confirmar/cancelar), incluindo rejeicao e relogin.
Cobertura da tabela nao prova que os 355 efeitos foram exercitados no client.

## Validação

- `STATICALLY VERIFIED`: fronteiras acima comparadas ao corpus e source.
- `AUTOMATED TESTED`: `go test -count=1 ./internal/data ./internal/game ./internal/wire`;
  a primeira execucao detectou 11 ingressos classificados pela regra base em
  vez do override de instances. Corrigida a classificacao, `./internal/data`
  passou; resultados anteriores de game/wire continuam validos, sem mudancas.
- `Build-Client.ps1 -Configuration Release`: 35439 checks C++ PASS e build
  Release/Win32 aprovado; warnings preexistentes C4018/C4305/C4309, sem erros.
  Instalado `tmproject/client748/project.exe`, igualdade SHA-256 com artefato:
  `71CD0587610CFF8EEEBA2D13CC03A15099C70BBE5B17C0ED545EB12897DAE3EA`.
- `CLIENT-TESTED`: pendente; nao executar ou alegar consumo real de personagens
  para substituir a validacao manual. Nenhum arquivo removido; dumps preservados.

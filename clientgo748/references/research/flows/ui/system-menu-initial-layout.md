---
id: ui-system-menu-initial-layout
title: Posição inicial e toggle do menu inferior direito 7.48
subsystem: ui-layout
status: TRACED
native_sha256: 8AA2F918844BCE3AFE21F1204F69757A443E32EB2F2F616936B1D9BFE215F593
updated: 2026-09-02
---

# Posição inicial e toggle do menu inferior direito 7.48

## Pergunta

Como o root `292` do menu de sistema é posicionado em relação ao botão
inferior direito `5744`, e por que deslocá-lo uma largura inteira para a
esquerda não corresponde ao client nativo?

## Fronteira de evidência

- Native: `WYD.exe` do hash do frontmatter, `FUN_00435B13` e handler de clique
  `FUN_004662C5` no projeto `WYD748Native_20260821.gpr`.
- Recurso: `client748/UI/FieldScene2.bin`, root `292/0x124` e botão
  `5744/0x1670`.
- Source: `TMFieldScene::InitializeCompatFieldScene()` e case `5744` do
  dispatcher de clique.
- Captura fornecida em 2026-09-02: flyout do candidato em posição diferente da
  referência original.

## Fluxo nativo 7.48

### Entrada observável

Durante a construção da FieldScene, `FUN_00435B13` resolve primeiro o botão
`5744` e depois o painel `292`. O botão é ancorado ao canto inferior direito;
o painel preserva o X materializado pelo recurso com um ajuste de dois pixels,
é colocado imediatamente acima do botão e inicia oculto.

### Callers

`FUN_00435B13` é chamado pela inicialização da FieldScene por dispatch de cena
resolvido no projeto Ghidra. `FUN_004662C5` é o callback de clique da mesma
cena, alcançado pelo dispatcher virtual de controles. O branch observado para
`param_1 == 0x1670` é a entrada concreta do usuário para abrir/fechar o menu;
por isso a ausência de caller `CALL rel32` no corpus não é uma lacuna de
lifecycle.

### Funções e fórmula

Em `FUN_00435B13`:

```text
menu.x = menu.resourceX - 2
menu.y = button5744.y - menu.height
menu.visible = false
```

O código nativo grava Y a partir de `button+0x50` e `menu+0x58`, subtrai a
constante `2.0f` do X já existente em `menu+0x4C` e chama o slot virtual
`+0x60` para ocultar. Ele não calcula `button.x - menu.width`.

No branch `0x1670` de `FUN_004662C5`, o slot `+0x60` do root `292` alterna a
visibilidade e o estado selecionado do botão é sincronizado com o campo de
visibilidade do painel.

### Callees

O initializer usa o lookup virtual `container+0x48` para os IDs, os helpers
`FUN_00401304`, `FUN_0040137C` e `FUN_004013C2` para os anchors do botão e o
slot `control+0x60` para visibilidade. O handler de clique usa o mesmo slot de
visibilidade e grava o estado selecionado do botão; não chama rede, gameplay
ou persistência.

### Saídas e falhas

Com os dois controles presentes, o menu fica oculto acima do botão e abre no
mesmo local ao clicar. Se um controle faltar, a source não dereferencia o
ponteiro; o menu permanece indisponível sem bloquear o restante da FieldScene.

## Estado e lifecycle

| Evento | Precondição | Transição | Resultado | Falha/saída |
| --- | --- | --- | --- | --- |
| construir FieldScene | 292 e 5744 carregados | ancora botão, posiciona menu e oculta | estado inicial nativo | controle ausente é ignorado |
| clicar 5744 fechado | cena ativa | alterna painel e seleciona botão | menu visível | sem packet |
| clicar 5744 aberto | menu visível | alterna painel e limpa seleção | menu oculto | sem estado residual |
| trocar resolução/cena | controles rematerializados | anchors e fórmula são reaplicados | posição acompanha viewport | sem coordenada persistida |
| logout | FieldScene destruída | root e botão são liberados com a árvore | nenhum ponteiro retido | N/A |
| relogin | nova FieldScene | estado inicial oculto é reconstruído | toggle limpo | mesmos guards |

## Wire, ABI e recursos

Não há wire nem ABI nova. IDs, dimensões e X inicial continuam vindo de
`FieldScene2.bin`; somente a semântica de composição do executável foi portada.
Offsets `+0x4C/+0x50/+0x58/+0x60` são evidência do nativo e não são usados na
source tipada.

## Mapeamento atual

### Source recompilável

Classificação: `PARIDADE_NATIVA`.

`InitializeCompatFieldScene()` liga `5744` e `292`, ancora o botão à direita e
ao rodapé, aplica `menu.x - 2` e `button.y - menu.height` e oculta o root. O
case `5744` encontra `292`, alterna a visibilidade e chama
`SButton::SetSelected()` com o mesmo valor.

### WYD-Go

Não aplicável. O menu apresenta comandos locais; cada comando que produzir
intenção continua seguindo seu handler e validação server-side próprios.

## Matriz de delta

| Claim | Nativo 7.48 | Source anterior | Source atual | Decisão |
| --- | --- | --- | --- | --- |
| X inicial | `resourceX - 2` | `buttonX - menuWidth` | `resourceX - 2` | restaurar fórmula |
| Y inicial | acima de 5744 | acima de 5744 | acima de 5744 | preservar |
| estado inicial | oculto | oculto | oculto | preservar |
| toggle | visibilidade + seleção do botão | handler compatível | handler compatível | manter simetria |

## Decisões

- Preservar o X do recurso e aplicar somente o ajuste nativo de dois pixels.
- Usar o botão apenas como âncora vertical e de viewport.
- Manter o menu oculto na construção e sincronizar seleção no toggle.
- Não introduzir persistência ou mensagem de rede.

## Lacunas

Falta testar clique, hitbox, fechamento, mudança de resolução e reconstrução
por logout/relogin em `800x600`, `1024x768` e `1280x960`.

## Validação

- `TRACED`: initializer, callback, branches, receptores, teardown e relogin
  foram correlacionados.
- `IMPLEMENTED`: posição inicial e toggle estão na source ativa.
- `validate_research.py --repo .` e `git diff --check` passaram em 2026-09-02;
  este último emitiu somente avisos LF/CRLF da worktree existente.
- `Build-Client.ps1` passou em `Release|Win32` v145 com zero erros e 21
  warnings preexistentes. Output e `client748/project.exe` têm SHA-256
  `AD624D96A9F6DE7024D383594E9778586B20A113A9683150D9EDB1584CECCFEC`.
- Não é `CLIENT-TESTED`: ainda falta aprovação visual do candidato deste lote.

---
name: wyd-go-feature
description: Implementar, corrigir ou auditar o WYD-Go e o client 7.48. Para client, protocolo, ABI, UI, input, render ou assets, exige consultar a descompilação Ghidra do WYD.exe 7.48 antes de editar; W2PP, Secrets, Micronics e TMProject são referências semânticas secundárias.
---

# WYD-Go feature

Para qualquer mudança que atravesse o client ou o contrato que ele observa,
`wyd-client748-catalog` e `wyd-client748-research` são gates anteriores, não
referências opcionais: o catálogo escolhe e ordena a raiz; a pesquisa fecha a
transição. A skill desta página implementa somente o que a ficha 7.48 já tornou
verificável; ela não deve transformar uma semelhança do TMProject em contrato.

## Roteamento e autoridade

Escolher o menor conjunto de referências que cobre o risco real. O fato de uma
mudança compilar não permite pular a evidência específica da área.

| Escopo | Leitura/skill obrigatória | Evidência que decide |
| --- | --- | --- |
| Servidor Go, regra ou persistência | esta skill + `AGENTS.md` scoped | código, testes e dados atuais |
| Packet, wire, struct ou ABI 7.48 | `wyd-client748-catalog` → `wyd-client748-research` + `references/ghidra-client748.md` | ficha `CONTRACT`, binário/Ghidra 7.48 + contrato server-side |
| UI, input, grid, inventário ou render | `wyd-client748-catalog` → `wyd-client748-research` + Ghidra + `references/client-ui-748.md` | ficha `TRACED` e lifecycle, recursos e função nativa 7.48 |
| Asset sob `client748/` | `wyd-client748-catalog` → `wyd-client748-research` + referências acima + `client748/skills/wyd-client-assets/SKILL.md` | consumidor real, manifest/hash e teste visual |
| Auditoria | `references/audit.md` | fluxo vivo, callers/callees e testes atuais |
| Hook/plugin Micronics | `add-hook`; `build-deploy` ao compilar/implantar | exe 7.54 exato + bytes e runtime do plugin |
| Conhecimento histórico | `wyd-dev-knowledge` por último | hipótese revalidada na versão alvo |

Hierarquia de evidência: código atual → testes atuais → `data/` autoritativo →
Ghidra/binário 7.48 → W2PP → Secrets 7.54 → Micronics → base histórica. Uma
fonte inferior pode explicar uma intenção, mas não contradizer silenciosamente
uma fonte superior nem fornecer ABI para outra versão.

## Fluxo

1. Respeitar `wyd-go/AGENTS.md` e qualquer `AGENTS.md` scoped; não reler a
   referência detalhada inteira por padrão.
2. Para a campanha de mapeamento ou uma nova raiz, executar o triador de
   `wyd-client748-catalog`, registrar sua lane/motivo e formar um grupo pequeno
   de funções ligado a uma entrada observável. O triador não promove maturidade.
3. Em continuação entre sessões, ler
   [`references/session-continuity.md`](references/session-continuity.md) e
   somente o handoff do escopo. Comparar seus fatos com `git status`, código e
   hashes atuais antes de reutilizá-los.
4. Localizar o caminho vivo com `rg` e preservar mudanças alheias.
5. Reproduzir o bug e identificar a fronteira: `wire`, `net`, `data`, `game`,
   `model`, `store` ou `client-source`.
6. Se a tarefa tocar client, protocolo, packet, ABI, struct, UI, input, render,
   assets ou comportamento observado do executável, ler primeiro
   `wyd-client748-research/SKILL.md`, criar/atualizar a ficha de fluxo e então
   ler [`references/ghidra-client748.md`](references/ghidra-client748.md) e
   consultar a função nativa correspondente na descompilação Ghidra do
   `WYD.exe` 7.48. Essa consulta é obrigatória mesmo quando TMProject/W2PP já têm
   código similar.
7. Se o escopo incluir HUD, janelas, input, mensagens, grid, inventário,
   equipamento ou drag, ler também
   [`references/client-ui-748.md`](references/client-ui-748.md).
   Se alterar asset sob `client748/`, ler também
   `client748/skills/wyd-client-assets/SKILL.md` e o `client748/AGENTS.md`.
8. Para regra nativa ainda incerta depois da evidência 7.48, pesquisar somente
   o necessário: W2PP → Secrets 7.54 → Micronics → `wyd-dev-knowledge`.
9. Portar semântica, nunca layout, offset ou endereço de outra versão. Layout,
   offset, opcode, tamanho e lifecycle do client precisam vir do 7.48.
   `client-source/tmproject` é uma source de versão única: depois de confirmar o
   equivalente nativo, remover o caminho exclusivo de 7.59+ em vez de manter
   branches de compatibilidade entre versões. Dados novos só podem permanecer
   quando traduzidos para uma representação comprovadamente suportada pelo 7.48.
10. Implementar no arquivo da feature; manter handlers como roteadores.
11. Comentar no próprio código todo trecho editado ou implementado, explicando
   intenção, contrato ou motivo técnico. Não usar comentário redundante que
   apenas repita a instrução executada.
12. Testar a transição real, inclusive falha, rollback e relogin quando aplicável.
13. Rodar testes, vet, build proporcional ao escopo e `git diff --check`.
14. Se o trabalho continuar em outra sessão, atualizar somente o handoff do
    escopo com fatos verificáveis, estado de validação e próximo passo.

Antes da primeira edição no escopo client, registrar no trabalho o caminho da
ficha e sua maturidade. `TRACED` é o mínimo para comportamento, UI, input,
render, lifecycle e assets. `CONTRACT` é obrigatório para packet, wire, ABI,
struct, offset, packing, signedness ou loader. Uma ficha `LOCATED` autoriza
somente investigação/documentação; ela não é um "quase aprovado".

## Condições de parada

Não editar ainda quando faltar qualquer prova necessária para decidir o
contrato: hash divergente sem nova análise, função/lifecycle 7.48 não
identificado, offset/packing/signedness incerto, recurso ou ID não materializado,
ou regra server-side sem fonte autoritativa, ou quando a ficha permanecer
`LOCATED`. Continuar a investigação por
strings, xrefs, call graph, dados e testes; se a lacuna permanecer, registrar
`não confirmado` e entregar a limitação, sem fabricar compatibilidade.

Também parar antes de uma mutação destrutiva ou deploy quando o alvo exato não
estiver resolvido. Worktree suja não é motivo para descartar mudanças: isolar o
diff do escopo e preservar tudo que não foi criado nesta tarefa.

## Artefatos do client

- `client748/wyd.exe nativo+patches/WYDoriginal.exe`: stock histórico imutável.
- `client748/wyd.exe nativo+patches/WYD.exe`: referência histórica Ghidra.
- `client-source/tmproject/build/<Configuration>/WYD.exe`: saída transitória da
  compilação.
- `client748/project.exe`: único candidato executável e de validação.

Nunca executar ou editar os scripts e patches do diretório histórico. Toda
alteração ativa do client deve ser feita em `client-source/` ou nos assets e a
compilação deve passar por `client-source/tmproject/Build-Client.ps1`, que
instala e confere automaticamente `client748/project.exe`, sem patch. Não
aceitar build bem-sucedido que deixe o candidato ausente ou desatualizado.

## Gate obrigatório do client 7.48

Antes de alterar código relacionado ao client, registrar na análise de trabalho:

- ficha `.agents/research/client748/flows/...` e maturidade aplicável;
- executável e SHA-256 usados como referência;
- funções Ghidra consultadas;
- callers/callees relevantes;
- offsets e tamanhos comprovados, quando houver wire/ABI;
- diferença observada entre o nativo 7.48 e o TMProject recompilável;
- o que foi portado como comportamento e o que deliberadamente não foi portado.

Não editar por tentativa visual quando o fluxo pode ser recuperado da
descompilação. Se a função não estiver identificada, localizar por strings,
xrefs, opcode, tamanho, constantes, imports ou call graph antes de implementar.
Se ainda não houver evidência suficiente, marcar como `não confirmado` e não
transformar a hipótese em contrato.

O corpus exportado é auxiliar: ausência textual, caller sintático ausente ou
função não indexada nunca prova ausência no binário. Reabrir a função e seus
xrefs no projeto Ghidra. O TMProject 7.69+ pode sugerir semântica, mas não pode
fornecer ID, recurso, packet, offset, endereço, layout ou lifecycle do 7.48.

## Invariantes

- `World` é o único escritor de gameplay.
- `Score` é autoritativo; wire legado nunca volta ao cálculo.
- Cliente é não confiável; revalidar tudo server-side.
- Anti-dupe: validar → mutar cópia → persistir → publicar.
- Sincronizar dono, observers, visibilidade e persistência na mesma feature.
- Consultas locais usam índice espacial.
- `Merchant != 0` nunca entra em combate.
- Inventário: 64/63; cargo: 128/120.
- A source recompilável possui somente ABI, UI, protocolo e lifecycle 7.48;
  referências mais novas não permanecem como implementação alternativa.
- Controle herdado da 7.59 sem ID/recurso no 7.48 é opcional: todos os acessos
  devem tolerar ausência e a transição principal não pode depender dele.

## Referências sob demanda

- Para qualquer trabalho no client compilável ou no ABI 7.48, ler
  `references/ghidra-client748.md`; esta referência não é opcional nesse escopo.
- Para UI, HUD, grid, inventário, equipamento, drag, mensagens ou lifecycle,
  ler `references/client-ui-748.md` depois do guia Ghidra.
- Ler `references/emulator-contracts.md` apenas para o assunto afetado
  (protocolo, NPC/craft, IA, combate ou persistência).
- Em auditoria de código morto/duplicação, ler `references/audit.md`.
- Para contratos detalhados que não caibam nas referências focadas, localizar o
  heading com `rg -n '^#' references/repository-contracts.md` e ler somente a
  seção necessária. O `AGENTS.md` da raiz prevalece em caso de conflito.
- Para continuidade, ler `references/session-continuity.md` e um único arquivo
  em `.agents/handoffs/`; handoff não é evidência autoritativa.
- Para protocolo, fórmulas ou funções nativas ausentes, usar
  `wyd-dev-knowledge`; não carregar essa base para mudanças puramente locais.
- Para hooks no plugin Micronics, usar `add-hook` e `build-deploy`.
- Para crash do `project.exe`, usar o procedimento de minidump/ASLR e o contrato
  de controles ausentes descritos em `references/ghidra-client748.md`.

## Conclusão

Concluir quando a regra for autoritativa, o feedback do client estiver correto,
falha/relogin preservarem estado, observers estiverem sincronizados e a
validação relevante passar. Atualizar somente a documentação afetada. Para
trabalho incompleto, deixar handoff compacto; para conhecimento estável, mover
o contrato à referência apropriada em vez de perpetuá-lo no handoff.

Aplicar validação proporcional e declarar cada camada separadamente:

| Area | Mínimo antes de concluir |
| --- | --- |
| Servidor Go | teste focado de sucesso/rejeição/rollback, persistência/relogin quando aplicável, `go test`, `go vet` e build do alvo |
| Client source | build por `Build-Client.ps1`, candidato instalado e hasheado, mais fluxo real para qualquer alegação comportamental |
| UI/render/assets | fluxo real, resolução de referência, screenshot comparável, input/fechamento/erro e inspeção de log/dump |
| Packet/ABI | teste dos dois lados, tamanhos/offsets documentados, rejeição inválida e relogin |
| Hook legado | build Win32, hash do deploy, startup/log e gatilho real com caminho de recuperação |

`IMPLEMENTED`, `STATICALLY VERIFIED`, `AUTOMATED TESTED` e `CLIENT-TESTED` não
são uma escada global. Classificar cada fluxo individualmente; um screenshot de
um item, um build verde ou um teste isolado não prova toda a feature.

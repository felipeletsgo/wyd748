---
name: wyd-go-feature
description: Implementar, corrigir ou auditar WYD-Go e o client customizado 7.48, escolhendo entre paridade nativa, modernização compatível e extensão coordenada client/server.
---

# WYD-Go feature

Implemente a menor mudança que fecha o fluxo real. Para client ou contrato
observado por ele, use antes `wyd-client748-research`; use o catálogo apenas ao
abrir nova raiz ou quando a fila perdeu validade.

## Escolha do modo

Antes de decidir ou implementar um delta, avalie e registre as fontes confiáveis
disponíveis: descompilação/Ghidra e binário nativo 7.48, assets 7.48, source
atual, WYD-Go/testes, TMProject e guias. Marque cada fonte como `UTILIZADA`,
`NÃO APLICÁVEL` ou `CONTRADITÓRIA`, com justificativa. W2PP, Secrets e
Micronics estão excluídos: são fontes bugadas e não validam, contradizem ou
orientam código e contratos.
Para comportamento, wire, ABI, UI, input, render, assets ou lifecycle, o estudo
da descompilação e do binário 7.48 é obrigatório antes da adaptação.

A evidência nativa 7.48, especialmente a descompilação estudada no Ghidra,
prevalece sobre referências posteriores. O TMProject serve como implementação
candidata e comparação secundária, nunca como prova do comportamento nativo.

- `PARIDADE_NATIVA`: reproduz comportamento legado. Exige ficha `TRACED`, ou
  `CONTRACT` para packet/wire/ABI/struct/offset/loader/recurso.
- `MODERNIZACAO_COMPATIVEL`: adota estrutura interna superior sem alterar a
  fronteira observável. Reutilize o contrato já comprovado e valide o delta.
- `EXTENSAO_COORDENADA`: cria comportamento ausente no nativo, com client e
  servidor evoluídos juntos. Exige contrato novo explícito e teste dos dois
  lados, não um equivalente Ghidra inexistente.

Preserve funções, assets e melhorias manuais existentes. Ausência no nativo ou
origem 7.69 não justifica remoção. Escolha entre estruturas por wire/ABI,
assets, lifecycle, ownership, suporte server-side, manutenção e testabilidade;
quando compatível, prefira a tecnicamente superior como único caminho ativo.

## Fast path

1. Em continuação, leia o handoff do escopo, confira `git status` e diff scoped
   e retome do próximo símbolo/comando.
2. Reutilize hash, triagem, ficha, exports e validações quando seus inputs não
   mudaram. Não releia referência nem rerode ferramenta por ritual.
3. Localize o fluxo vivo com `rg`, começando pela source atual e seus testes.
4. Carregue somente a referência da fronteira alterada.
5. Implemente e documente o lote no mesmo ciclo; valide somente os consumidores
   afetados e use a suíte ampla no gate de integração.

## Anti-loop de continuidade

Compactação automática não cria uma nova investigação. Trate o resumo e o
handoff como estado já recuperado e continue do próximo passo executável.

- Faça no máximo uma checagem curta de `git status` e diff scoped por retomada.
- Não liste chats, releia a conversa ou reabra a tarefa original para confirmar
  um handoff que já aponta o arquivo e o símbolo seguintes.
- Limite cada comando ao trecho necessário e evite saídas repetidas; uma saída
  já registrada com os mesmos inputs é reutilizável.
- Após a checagem inicial, execute logo um patch ou um teste focado. Se duas
  chamadas consecutivas não produzirem patch, teste ou evidência nova, pare e
  registre o bloqueio.
- Não declare a mesma intenção de “retomar” mais de uma vez. Mensagens de
  status não substituem avanço técnico.
- Se o usuário interromper a execução, finalize o ciclo. Não aguarde nem
  inicie uma continuação automática; uma nova mensagem explícita inicia uma
  única retomada delimitada.

## Roteamento

| Escopo | Evidência/referência |
| --- | --- |
| Servidor Go, regra ou persistência | código, testes, `data/` e `AGENTS.md` scoped |
| Paridade client ou fronteira legada | ficha nativa + [references/ghidra-client748.md](references/ghidra-client748.md) |
| UI, input, grid, inventário ou render | anterior + [references/client-ui-748.md](references/client-ui-748.md) |
| Asset sob `client748/` | `client748/AGENTS.md` + `client748/skills/wyd-client-assets/SKILL.md` |
| Extensão client/server | contrato novo, colisões na integração legada e testes dos dois lados |
| Auditoria | [references/audit.md](references/audit.md) |
| Histórico/fórmula | `wyd-dev-knowledge` por último |

Hierarquia para claims legados: código/testes atuais → dados do projeto →
descompilação/Ghidra e binário 7.48 → TMProject e guias. Para uma extensão, o contrato
versionado e os testes client/server são a fonte primária; o nativo apenas
delimita a integração.

## Implementação

- Servidor permanece autoritativo; client envia intenção.
- Handlers roteiam e regras ficam no módulo da feature.
- Anti-dupe: validar → mutar cópia → persistir → publicar.
- Sincronizar owner, observers, visibilidade e persistência.
- Validar sucesso, rejeição, rollback, teardown e relogin quando aplicáveis.
- Para contrato novo, definir direção, tamanho, campos, validação, IDs/opcodes,
  capability/fallback quando necessário e testes dos dois lados.
- Não copiar ABI/offset/packet de versão posterior para uma fronteira legada.
- Em arquivo do client criado, convertido, adaptado ou funcionalmente alterado,
  escrever documentação didática em português para responsabilidade da
  unidade, APIs públicas, parâmetros/retornos e ownership, nulabilidade e
  lifecycle relevantes. No corpo, explicar fases, invariantes, efeitos e
  decisões não óbvias; não narrar cada instrução.
- Registrar na ficha ou documento do lote a procedência comprovada (`7.48`,
  `TMProject 7.69+` ou local) e o modo do delta. Marcar stub como pendente em
  vez de inventar comportamento.
- Refatorar somente quando a clareza, o ownership, o lifecycle ou a duplicação
  melhorarem sem alterar comportamento, wire, ABI, recursos ou ordem dos
  efeitos. Tratar mudança observável como delta separado.
- Não descartar ou reformatar mudanças alheias numa worktree suja.

Uma ficha `LOCATED` impede somente código que dependa daquele claim nativo. Se
o lote for uma extensão independente, registre essa separação e prossiga. Pare
quando houver conflito real de ownership, alvo destrutivo incerto ou fronteira
legada necessária ainda não resolvida.

## Client source

Artefatos:

```text
client748/wyd.exe nativo+patches/WYDoriginal.exe  stock histórico imutável
client748/wyd.exe nativo+patches/WYD.exe          referência Ghidra
client-source/tmproject/build/.../WYD.exe         saída transitória
client748/project.exe                            candidato de validação
```

Não executar nem editar patches/binários históricos. Alterações ativas ficam na
source/assets. Compilar apenas com
`client-source/tmproject/Build-Client.ps1`, que instala e confere
`client748/project.exe`.

A source é única, não multi-versão. Uma estrutura 7.69 pode tornar-se o caminho
ativo se adaptada ao ecossistema; não manter branches paralelos apenas por
versão. Controle sem recurso pode ser opcional. Um widget ou asset moderno pode
existir como extensão deliberada com binding/lifecycle completos, nunca só para
mascarar um acesso nulo.

## Validação proporcional

| Área alterada | Mínimo |
| --- | --- |
| Servidor Go | teste focado de sucesso/rejeição/rollback; persistência/relogin quando aplicável; vet/build do alvo |
| Client source | `Build-Client.ps1`, candidato instalado/hasheado e teste do fluxo para alegação comportamental |
| UI/render/assets | resolução/recurso, input, abertura/fechamento/erro e fluxo real para `CLIENT-TESTED` |
| Packet/ABI | asserts/teste byte a byte, validação dos dois lados, rejeição inválida e relogin |
| Documentação/skill | validador aplicável e `git diff --check`; não recompilar produto não alterado |

Sempre rodar `git diff --check`. `IMPLEMENTED`, `STATICALLY VERIFIED`,
`AUTOMATED TESTED` e `CLIENT-TESTED` descrevem camadas distintas. Build verde
não prova paridade ou fluxo in-game.

## Continuidade e referências

Use [references/session-continuity.md](references/session-continuity.md) para
retomada e mantenha um único handoff curto por escopo. Atualize-o apenas quando
houver descoberta, mudança, validação, bloqueio ou novo ponto de retomada.

Leia `references/emulator-contracts.md` somente na seção do domínio afetado e
`references/repository-contracts.md` somente pelo heading localizado com `rg`.
O `AGENTS.md` da raiz prevalece em conflito.

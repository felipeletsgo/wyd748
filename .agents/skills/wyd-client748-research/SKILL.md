---
name: wyd-client748-research
description: Investigar e documentar fluxos do client nativo WYD 7.48 antes de implementar mudanças no WYD-Go ou no client recompilável. Use para protocolo, ABI, UI, input, render, assets, lifecycle, cenas, rede e para auditar heranças 7.59/7.69 do TMProject. Exige hash do binário, funções e xrefs Ghidra, callers/callees, estado, side effects, recursos, equivalente server-side e uma ficha de evidência versionada.
---

# Pesquisa do client WYD 7.48

## Papel

Esta skill adquire conhecimento verificável. Ela não autoriza implementar uma
hipótese. Use-a antes de `wyd-go-feature` sempre que o resultado puder alterar
comportamento do client, protocolo, ABI, UI, input, render, assets, lifecycle ou
o contrato observado pelo servidor. A implementação só começa depois que a
ficha do fluxo atingir a maturidade exigida pelo escopo.

O TMProject importado é uma fonte 7.59/7.69. Ele pode sugerir nomes e intenção,
mas nunca decide o contrato 7.48. Comentários já presentes na source são índices
de busca, não prova. Toda conclusão volta ao binário 7.48 e ao fluxo atual do
WYD-Go.

Para investigação de paridade 7.48, preferir `gpt-5.6-sol` com esforço `xhigh`
quando essa seleção estiver realmente disponível. Caso contrário, usar o maior
nível disponível e registrar a limitação; nunca afirmar que o modelo foi
alterado sem confirmação do ambiente.

## Leituras e artefatos obrigatórios

1. Ler `AGENTS.md`, o `AGENTS.md` scoped e esta skill integralmente.
2. Ler `../wyd-go-feature/references/ghidra-client748.md`.
3. Para UI, HUD, grid, inventário, equipamento ou mensagens, ler também
   `../wyd-go-feature/references/client-ui-748.md`.
4. Verificar `git status --short` e o SHA-256 da referência histórica.
5. Abrir `references/research-method.md` e somente a linha de domínio relevante
   em `references/subsystem-map.md`.
6. Criar ou atualizar uma ficha em `.agents/research/client748/flows/` a partir
   do template. Handoff não substitui ficha de evidência.

Antes de declarar uma pesquisa pronta, deixar na ficha o hash do binário, a
pergunta observável, funções nativas, callers/callees, estado, mutações,
side-effects, erros, recursos/ABI quando aplicável, equivalente na source,
equivalente autoritativo no WYD-Go, delta do TMProject e os testes realmente
executados. Se a investigação terminar incompleta, atualizar também um handoff
compacto com o próximo ponto de entrada; não esconder a lacuna em uma conclusão
genérica.

## Unidade mínima de pesquisa

Uma investigação rastreia uma transição observável completa, não uma função
isolada:

```text
entrada/evento
-> função nativa 7.48
-> callers e callees
-> precondições e estado inicial
-> mutações, globals e side effects
-> saída, erro, fechamento e relogin
-> wire/ABI ou recursos consumidos
-> equivalente atual no client-source
-> equivalente autoritativo no WYD-Go
-> diferença da herança TMProject
-> decisão e lacunas
```

Quando o fluxo for amplo, dividi-lo por transição. Não criar uma ficha genérica
"inventário" que misture abertura, drag, uso, equip, venda e rollback.

## Procedimento

1. Formular uma pergunta concreta e localizar a entrada por string, opcode,
   constante, import, ID de recurso, endereço ou efeito observado.
2. Consultar o corpus exportado com `scripts/query_corpus.py`; ele acelera busca,
   mas seus callers sintáticos não substituem xrefs do projeto Ghidra. Ausência
   no índice textual, inclusive para uma função referenciada pela source, nunca
   prova ausência no binário.
3. Abrir a função no projeto Ghidra e registrar xrefs, callers, callees, dados,
   globals, calling convention e lifecycle. Confirmar callers/callees indiretos,
   slots de vtable e callbacks diretamente no projeto Ghidra; exports são apenas
   aceleradores. Use `scripts/ExportWydFlow.java` para gerar um relatório auxiliar
   reproduzível quando possível. Para callback sem xref tipado, o argumento
   `pointers:<entry>` procura o VA bruto de forma focada; zero hits não prova
   ausência de chamada computada. Para um alvo de código sem caller direto,
   `relatives:<entry>` procura somente instruções decodificadas `CALL E8` e
   `JMP E9`, calcula o destino pelo `rel32` assinado e registra a função
   proprietária. Zero hits nesse modo elimina apenas esses branches relativos;
   não elimina chamada indireta, thunk, bytes não analisados ou tabela runtime.
   Quando o entry point continuar sem caller, usar também `bodyrefs:<entry>` e
   `rawrelatives:<entry>` conforme `references/research-method.md`. O primeiro
   classifica xrefs externos para todo o corpo; o segundo cobre bytes `E8/E9`
   em blocos executáveis inicializados. Validar ambos contra controles positivos
   conhecidos no mesmo binário antes de interpretar um resultado zero.
4. Para wire/ABI, provar direção, opcode, tamanho, offsets, packing e signedness
   no packet final. `sizeof` moderno ou struct homônima não é evidência 7.48.
5. Seguir o fluxo vivo na source e no Go com `rg`, incluindo caminhos de erro,
   teardown, owner, observers, persistência e relogin conforme o domínio.
6. Comparar 7.48, source atual e TMProject por campo/comportamento. Classificar
   cada afirmação como `CONFIRMED`, `PROBABLE`, `HYPOTHESIS` ou `REJECTED`.
7. Reabrir a entrada, ao menos um caller e um callee relevante no projeto Ghidra,
   conferir novamente o hash, atualizar a ficha e executar
   `scripts/validate_research.py`. Só depois entregar um contrato `TRACED` ou
   superior à skill `wyd-go-feature`.

## Maturidade da ficha

| Estado | Significado | Autoriza edição comportamental? |
| --- | --- | --- |
| `UNMAPPED` | domínio inventariado, sem entrada nativa localizada | não |
| `LOCATED` | função/asset/opcode localizado, fluxo ainda incompleto | não |
| `TRACED` | entrada, callers/callees, estado, side effects e erros rastreados | sim, no escopo traçado |
| `CONTRACT` | `TRACED` mais ABI/recursos e delta server/source decididos e testáveis | sim, inclusive wire/ABI coberto |
| `CLIENT_TESTED` | contrato implementado e executado no `project.exe` no fluxo real | já validado no cenário registrado |

Não promover maturidade para contornar uma lacuna. Uma ficha pode conter claims
`HYPOTHESIS`, mas o trecho dependente delas não entra no contrato nem autoriza
edição.

## Gates

- Mudança comportamental no client exige ficha `TRACED` no mínimo.
- Packet, struct, offset, packing, signedness ou loader exige ficha `CONTRACT`.
- UI exige criação, binding, IDs materializados, input, abertura, fechamento e
  ownership; screenshot não substitui lifecycle.
- Ausência de caller/callee confirmado mantém a ficha em `LOCATED`.
- Valor vindo somente do TMProject permanece `HYPOTHESIS`.
- Divergência de hash invalida reutilização silenciosa dos endereços.
- Uma função que não apareceu no export textual deve ser resolvida pelo projeto
  Ghidra ou permanecer `não confirmada`; não criar um stub, endereço ou caller
  para preencher o índice.
- A maturidade é por transição e por claim. Uma ficha `TRACED` de abertura não
  libera automaticamente drag, uso, rollback, relogin ou teardown de outra
  transição do mesmo subsistema.
- Alteração de comportamento bloqueia em `LOCATED`; alteração de packet, ABI,
  struct, offset, packing, signedness ou loader bloqueia até `CONTRACT`.
- Conhecimento estável vai para ficha/referência; estado operacional vai para
  handoff compacto. Não acumular pseudocódigo ou transcrição em handoff.

## Entrega para implementação

Ao concluir a pesquisa, entregar à `wyd-go-feature` o caminho da ficha e uma
decisão explícita por claim: `portar`, `remover`, `manter`, `proteger como
opcional` ou `não implementar`. A entrega deve separar:

- comportamento confirmado que pode ser reproduzido;
- layout/ABI/recursos confirmados, quando houver;
- semântica apenas sugerida pelo TMProject;
- lacunas que continuam bloqueando edição ou teste.

Se o usuário pedir implementação no mesmo turno e o gate ainda estiver em
`LOCATED`, permanecer em investigação/documentação e registrar o bloqueio no
handoff. Não promover a ficha apenas porque o build atual compila.

## Ferramentas

```powershell
python .agents/skills/wyd-client748-research/scripts/query_corpus.py stats --repo .
python .agents/skills/wyd-client748-research/scripts/query_corpus.py flow 0055890a
python .agents/skills/wyd-client748-research/scripts/query_corpus.py search "FieldScene2.bin"
python .agents/skills/wyd-client748-research/scripts/validate_research.py --repo .
```

Passe `--corpus <diretorio>` quando a descoberta automática não encontrar
`functions.tsv`. Leia `references/research-method.md` antes de interpretar a
saída.

## Conclusão

Entregar a ficha atualizada, a maturidade real, as evidências que sustentam cada
claim, as diferenças 7.48/TMProject/WYD-Go e as lacunas restantes. A pesquisa
termina quando outra pessoa consegue reproduzir a conclusão sem confiar na
conversa que a originou. O resultado mínimo de uma sessão incompleta é: ficha
com estado correto, validação executada, hash confirmado e handoff com próximo
comando executável.

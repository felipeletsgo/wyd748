# Auditoria do WYD-Go

Use este roteiro para revisão, diagnóstico, compatibilidade client/servidor e
remoção de código morto. Relatório anterior, comentário e teste isolado são
índices de busca, não evidência.

## 1. Fixar a invariável

Escrever em uma frase o contrato que não pode ser quebrado. Exemplos:

- o servidor deriva o resultado e o client só envia intenção;
- a mesma operação não duplica item após erro/relogin;
- wire 7.48 preserva tamanho e offsets comprovados;
- UI fecha sem manter modal ou captura de input;
- item 1×1 ocupa uma célula no inventário, mas preenche a região equipado.

Sem uma invariável precisa, a auditoria tende a listar diferenças sem decidir
quais são defeitos.

## 2. Mapear todas as representações

Procurar com `rg` e seguir:

```text
wire ↔ parser/handler ↔ model/runtime ↔ store ↔ dados base ↔ client/UI
```

Registrar conversões, cópias, valores derivados, caches e representações
legadas. Para client 7.48, aplicar o gate Ghidra antes de concluir que o
TMProject representa o contrato nativo.

## 3. Enumerar entradas e escritores

Incluir:

- login/relogin e carregamento inicial;
- packet de ação e handlers alternativos;
- tick, IA, timer, callback e hook;
- reload/admin/script/fallback;
- transição de mapa, disconnect e rollback;
- input, click, hotkey, `Esc`, `X` e callback de UI.

Se houver mais de um escritor, demonstrar quem serializa a mutação. Estado de
gameplay deve convergir no `World`.

## 4. Seguir a transição completa

Para cada entrada, verificar:

1. validação de identidade, estado, distância, ownership e limites;
2. mutação em cópia ou estado transacional;
3. persistência e comportamento em falha;
4. publicação para dono e observers;
5. limpeza de estado temporário/modal;
6. resultado após relogin ou repetição do packet.

Procurar retorno antecipado entre mutação, persistência e publicação.

## 5. Procurar bypasses

- handler antigo ainda registrado;
- opcode/tamanho alternativo aceito;
- função exportada chamada fora do fluxo novo;
- mock que evita parser, store ou `World`;
- controle moderno que mascara recurso ausente do 7.48;
- branch de 7.59 ainda alcançável;
- scan global usado como fallback de consulta local;
- erro que deixa item, moeda, modal ou observer parcialmente atualizado.

## 6. Revisar testes e evidência

Abrir o teste e confirmar que ele atravessa o caminho real. Um teste pode passar
enquanto mocka justamente a fronteira defeituosa. Exigir testes focados para:

- sucesso e rejeição;
- repetição/idempotência;
- falha de persistência e rollback;
- disconnect/relogin;
- owner/observer;
- limites e dados malformados;
- layout byte a byte quando houver ABI;
- interação real quando o defeito for visual/input.

Classificar resultado por evidência:

```text
IMPLEMENTED          alteração existe
STATICALLY VERIFIED  inspeção/build estático passou
AUTOMATED TESTED     teste relevante passou
CLIENT-TESTED        fluxo real no client passou
```

Não usar o nível mais alto para inferir os anteriores em áreas diferentes.

## 7. Código morto e duplicação

Executar quando houver mudança Go relevante:

```powershell
go run golang.org/x/tools/cmd/deadcode@latest -test ./...
```

Antes de remover:

- provar ausência de callers estáticos e dinâmicos;
- verificar registro por opcode, tabela, callback, reflection, hook ou script;
- confirmar que não é fallback, migração ou compatibilidade distribuída ativa;
- buscar nomes/IDs equivalentes e side effects não óbvios;
- remover em patch pequeno e executar teste/build afetado.

Classificar `Scaffolding` quando houver TODO/pendência ou sistema incompleto;
classificar `Morto real` somente após provar que o caminho ativo o substituiu.
Não remover stubs sem autorização. `vet` e `deadcode` limpos não detectam
transações duplicadas; procurar grants, saves e rollbacks copiados.

### Símbolos intencionalmente sem uso

- `internal/game/mob_combat.go: World.mobHasActivePlayer`: aguarda rework da IA.
- `internal/game/combat.go: magicDamage`: marcador até portar dano mágico e
  resistências; o caminho vivo usa `skillFinalDamage`/`skillUsesMagic`.
- `cmd/npcconvert/main.go: cstr/indexByte`: utilitários de conversão pontual.

Antes de alterar esta lista, conferir se o complemento já foi implementado.

## Perguntas obrigatórias

- O client envia valor que deveria ser derivado?
- Existe mais de um escritor do mesmo estado?
- A rejeição deixa estado parcial?
- Dono e observers recebem a mesma verdade?
- Relogin restaura o mesmo resultado?
- Consulta local faz scan global?
- O teste evita o fluxo real por mock?
- Há caminho 7.59 ainda executável no produto 7.48?
- A UI fecha lifecycle/input ou só desaparece visualmente?
- Evidência atual contradiz handoff, comentário ou relatório anterior?

## Saída da auditoria

Listar primeiro defeitos concretos, ordenados por severidade. Cada achado deve
conter arquivo/linha, invariante violada, cenário, impacto e evidência. Separar:

```text
CONFIRMADO     fluxo demonstrado no código/dado/binário atual
PROVÁVEL       forte evidência, reprodução ainda pendente
HIPÓTESE       pista que exige investigação adicional
NÃO DEFEITO    diferença explicada pelo contrato da versão
```

Depois dos achados, registrar fluxos auditados e lacunas. Não declarar
“100% compatível” ou “cobertura total” sem enumerar e testar os critérios.

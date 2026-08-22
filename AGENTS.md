# WYD-Go

## Política canônica de modelo

Para tarefas deste repositório, a preferência operacional é `gpt-5.6-luna`
com esforço `max` quando essa seleção estiver disponível na sessão. O modelo e
o nível efetivamente usados continuam sendo definidos pelo ambiente Codex; se
Luna não estiver disponível, usar o maior nível exposto pela sessão e nunca
afirmar que um modelo indisponível foi utilizado.

Skills, subagentes e ferramentas não podem reduzir o nível de raciocínio
exigido pela tarefa nem substituir silenciosamente um workflow obrigatório.

O projeto exige análise técnica profunda. Não priorize velocidade sobre
correção. Em tarefas de código, auditoria, protocolo, persistência ou mecânicas
de gameplay, investigue o fluxo completo antes de concluir.

---

# Uso obrigatório de skills

O diretório canônico de skills gerais versionadas deste projeto é:

```text
.agents/skills
```

Skills estritamente ligadas a um artefato podem viver no próprio subtree e ser
tornadas obrigatórias por um `AGENTS.md` mais específico. O client 7.48 possui:

```text
client748/skills/wyd-client-assets/SKILL.md
```

Resolva esse caminho a partir da raiz do repositório. Não codifique caminhos
absolutos do computador do desenvolvedor em scripts, documentação ou testes.
Além das skills versionadas, considere as skills fornecidas pela sessão Codex
quando a tarefa exigir uma integração, como GitHub.

## Regra obrigatória

Skills fazem parte do procedimento de execução deste projeto e **não são
opcionais**.

Antes de iniciar **qualquer tarefa técnica não trivial**, o agente deve:

1. inspecionar `.agents/skills` e as skills fornecidas pela sessão que possam
   ser aplicáveis;
2. identificar quais skills são aplicáveis à tarefa;
3. abrir e ler o `SKILL.md` atual de cada skill necessária;
4. seguir o workflow, restrições, referências e validações definidos pela skill;
5. somente depois iniciar análise, planejamento, edição ou implementação.

Se existir uma skill aplicável, o agente **DEVE usá-la**.

É proibido:

- ignorar uma skill aplicável e trabalhar apenas com conhecimento próprio;
- considerar uma skill "usada" sem abrir seu `SKILL.md` na tarefa atual;
- substituir silenciosamente o workflow da skill por um procedimento genérico;
- iniciar pesquisa em fontes externas antes de consultar a skill que governa a
  tarefa;
- declarar a tarefa concluída sem executar as validações obrigatórias da skill.

A obrigação vale também para auditorias, revisões, debugging, planejamento,
implementação, testes, GitHub, PRs, CI e patches do client.

## Descoberta de skills

Não assumir que a lista de skills conhecida pelo modelo está completa ou
atualizada.

Para cada nova tarefa técnica, verificar o conteúdo atual de:

```text
.agents/skills
```

e localizar os `SKILL.md` pertinentes.

Quando houver uma skill mais específica e outra mais genérica, usar a específica
para o workflow principal e a genérica apenas quando também for necessária.

Se mais de uma skill cobrir partes diferentes da tarefa, usar todas as skills
necessárias.

## WYD-Go Feature — obrigatória para tarefas de gameplay

Para qualquer tarefa relacionada ao comportamento, implementação, auditoria ou
pesquisa do servidor/client WYD, é obrigatório abrir primeiro:

```text
.agents/skills/wyd-go-feature/SKILL.md
```

Isso inclui, entre outros:

- mecânicas nativas;
- packets e protocolo;
- client WYD 7.48;
- meshes, texturas, skeletons e animações do client;
- trajes, skins e montarias visuais;
- descompilação do client;
- patches, hooks, trampolines e ABI do `WYD.exe`;
- itens e volatiles;
- skills;
- mobs;
- bosses;
- quests;
- instâncias;
- Water;
- Cube;
- Big Cube;
- Nightmare;
- Hell Gate;
- crafting;
- evolução;
- combate;
- affects;
- inventário;
- comércio;
- guild;
- party;
- movimento;
- drops;
- macros;
- comparação com W2PP;
- comparação com Secrets 7.54;
- comparação com Micronics;
- outras sources WYD.

**Nenhuma análise técnica desses assuntos deve começar antes da leitura do
`SKILL.md` atual da `wyd-go-feature`.**

Depois de carregar a skill, consultar somente as referências necessárias ao
problema atual.

## Client assets — skill e agent routing obrigatórios

Para importar, corrigir ou auditar qualquer asset visual sob `client748/`, ler
também:

```text
client748/AGENTS.md
client748/skills/wyd-client-assets/SKILL.md
```

Esse workflow cobre MSH/WYS/BON/ANI, `MeshTextureList.bin`, `ValidIndex.bin`,
selector/renderer de costumes e montarias e patches reprodutíveis do `WYD.exe`.
Separar pesquisa read-only, exploração, implementação e integração conforme os
papéis definidos no `client748/AGENTS.md`.

## GitHub e workflows especializados

Quando a tarefa envolver GitHub, PR, CI, revisão ou publicação, verificar as
skills disponíveis na sessão Codex e no repositório, usando obrigatoriamente a
mais específica.

Exemplos de roteamento:

```text
triagem ou contexto geral de GitHub/PR/issue
→ skill GitHub geral

comentários/review threads de PR
→ skill específica para review comments

GitHub Actions ou checks falhando
→ skill específica de CI

commit/push/publicação de branch ou PR
→ skill específica de publicação
```

Não executar um workflow genérico quando existir uma skill específica para a
operação solicitada.

## Skills e subagentes

Toda delegação mantém estas regras.

Subagentes:

- devem seguir `AGENTS.md`;
- devem usar o modelo e o nível de raciocínio disponíveis na sessão, preferindo
  `gpt-5.6-luna`/`max` somente quando oferecidos pelo ambiente;
- devem inspecionar as skills aplicáveis à própria subtarefa;
- devem abrir os respectivos `SKILL.md`;
- não podem reduzir validações exigidas pela skill principal.

Uma skill ou subagente não pode substituir a política de modelo deste projeto.

## Falha de acesso à skill

Se uma skill aplicável estiver ausente, ilegível ou inacessível:

1. não fingir que ela foi consultada;
2. não declarar a tarefa concluída;
3. informar explicitamente qual skill não pôde ser usada e por quê.

Somente prosseguir sem a skill quando a tarefa puder ser executada de forma
independente sem violar uma exigência obrigatória do projeto, deixando essa
limitação explícita.

## Gate obrigatório de conclusão

Antes de declarar qualquer tarefa técnica concluída, verificar:

```text
[ ] inspecionei .agents/skills
[ ] carreguei qualquer skill scoped exigida pelo AGENTS.md do subtree afetado
[ ] identifiquei todas as skills aplicáveis
[ ] abri o SKILL.md atual de cada skill necessária
[ ] segui o workflow e as restrições das skills
[ ] consultei somente as referências necessárias
[ ] executei as validações exigidas pelas skills
[ ] não substituí evidência por relatório ou suposição
```

Se qualquer item obrigatório estiver pendente, a tarefa **não está concluída**.

---

# Objetivo

Emulador Go server-authoritative para o client WYD 7.48.

O client:

- envia intenções;
- apresenta estado;
- nunca é fonte de verdade.

Toda decisão de gameplay é validada e executada pelo servidor.

---

# Regra fundamental de trabalho

## Comentarios explicativos obrigatorios no codigo

Toda edicao ou implementacao deve deixar no proprio codigo um comentario
explicativo junto ao trecho alterado. O comentario deve registrar a intencao,
o contrato ou o motivo tecnico da mudanca, especialmente quando houver
compatibilidade de versao, protocolo, seguranca, persistencia ou comportamento
nao obvio.

Nao escrever comentarios que apenas repitam literalmente a instrucao. O
comentario precisa ajudar o proximo mantenedor a entender por que o codigo
existe e qual invariante nao deve ser quebrada. Ao editar codigo existente sem
comentario suficiente, adicionar ou atualizar o comentario no mesmo patch.

Nunca considere uma implementação correta apenas porque:

- o usuário informou que foi corrigida;
- outro agente informou que foi corrigida;
- existe comentário dizendo que foi corrigida;
- existe documentação dizendo que foi corrigida;
- existe teste que aparentemente cobre o problema;
- `go test ./...` passou.

Quando a tarefa for revisão, auditoria ou validação:

1. abra a implementação atual;
2. siga o fluxo real de execução;
3. confira callers e callees relevantes;
4. verifique estados de erro e rollback;
5. procure caminhos alternativos que contornem a correção;
6. confira os testes;
7. somente então dê o parecer.

Relatórios de implementação são pistas, não evidência.

---

# Hierarquia de evidência

Ao investigar um comportamento, use esta prioridade:

1. código atual do `wyd-go`;
2. testes atuais do `wyd-go`;
3. dados autoritativos em `data/`;
4. client WYD 7.48 / sua descompilação;
5. W2PP;
6. Secrets 7.54;
7. Micronics e outras referências compatíveis.

Para qualquer tarefa relacionada ao WYD, a leitura de `.agents/skills/wyd-go-feature/SKILL.md` é pré-condição obrigatória, conforme a seção **Uso obrigatório de skills**.

Depois de carregar a skill, consulte apenas as referências relacionadas ao problema atual. Não carregue toda a documentação ou todas as fontes por padrão.

Ao usar implementação de outra versão:

- porte algoritmos;
- porte regras;
- não copie structs;
- não copie offsets;
- não copie endereços;
- não assuma ABI;
- não assuma layout de packet;
- não assuma tamanho de estrutura.

Tudo que depende do client 7.48 deve ser confirmado no próprio client 7.48.

## Gate Ghidra obrigatório para o client 7.48

Toda tarefa que altere `client-source/`, protocolo, ABI, structs, UI, input,
render, assets ou comportamento do executável deve consultar a descompilação
Ghidra do `client748/WYD.exe` antes da primeira edição. O agente deve ler também
`.agents/skills/wyd-go-feature/references/ghidra-client748.md` e registrar as
funções nativas, callers/callees e contratos usados como evidência.

TMProject, W2PP, Secrets e Micronics podem oferecer implementações melhores ou
mais legíveis, mas não substituem a confirmação do fluxo 7.48. É proibido
ajustar esses caminhos por tentativa visual quando a descompilação permite
recuperar IDs, lifecycle, offsets, tamanhos ou condições nativas.

## Referência mais nova e dados mais completos

O TMProject pode ser de uma versão superior (por exemplo, 7.59) e por isso
conter correções de comportamento ou tabelas mais completas. Essa vantagem é
semântica, não uma autorização para transportar ABI. Quando uma tabela nova
for necessária, ela deve ser traduzida para o formato que o executável 7.48
consome, com limites, offsets e tamanho validados no client 7.48. Se não houver
uma projeção comprovadamente segura, usar o dado legado ou manter a feature
desabilitada; nunca enviar uma struct moderna diretamente ao 7.48.

## Source recompilável exclusiva do client 7.48

`client-source/tmproject` não é uma source multi-versão. O produto dessa árvore
é exclusivamente o client WYD 7.48 usado por este repositório.

Depois que o comportamento equivalente for confirmado no `WYD.exe` 7.48 pelo
Ghidra, excluir da source recompilável:

- branches alternativos exclusivos de 7.54/7.59+;
- layouts, controles e páginas de UI inexistentes no 7.48;
- opcodes, structs e campos sem representação no protocolo 7.48;
- loaders e formatos modernos que só servem ao client mais novo;
- flags de compatibilidade cuja única função seja escolher entre 7.48 e uma
  versão posterior.

Não manter o caminho moderno "por compatibilidade futura": ele cria uma segunda
ABI, confunde a auditoria e aumenta a superfície de crash. Sources mais novas
continuam válidas como referência semântica read-only. Uma feature ou dado novo
só entra quando puder ser projetado explicitamente no formato suportado pelo
7.48; nesse caso a implementação final ainda deve possuir apenas o contrato
7.48.

A remoção deve ser baseada em evidência. Não apagar código apenas por nome,
idade ou suspeita: identificar primeiro o equivalente nativo, callers, recursos
e lifecycle no Ghidra para não remover uma rotina que o 7.48 também utiliza.

---

# Contratos arquiteturais

## World

`World` é o único dono do estado mutável de gameplay.

Nenhum outro package pode manter uma segunda fonte de verdade para estado de
jogo.

## Score

`Score` v2 é autoritativo.

`LegacyScore28` existe apenas como projeção do protocolo.

Nunca derive estado autoritativo de valores recebidos do client.

## Validação server-side

Validar no servidor, quando aplicável:

- packet;
- tamanho;
- opcode;
- fase;
- estado da sessão;
- alvo;
- identidade;
- distância;
- terreno;
- gameplay space;
- cooldown;
- item;
- UID;
- slot;
- preço;
- requisito;
- saldo;
- inventário;
- capacidade;
- membership;
- sequência;
- deadline.

Não confiar em campos do pacote apenas porque o client normalmente os envia
corretamente.

---

# Persistência e anti-dupe

PostgreSQL é a persistência autoritativa.

JSON existe somente como adaptador explícito de desenvolvimento:

```text
database_driver=json
```

Não existe fallback automático.

Se PostgreSQL estiver configurado e indisponível, o servidor deve falhar no
boot em vez de iniciar com outro estado.

Toda operação capaz de criar dupe segue:

```text
validar
→ criar snapshot/cópia
→ mutar estado
→ persistir
→ publicar
```

Se a persistência falhar:

```text
restaurar TODO o snapshot
```

Não fazer rollback parcial.

Especialmente revisar essa ordem em:

- craft;
- compra/venda;
- trade;
- ticket;
- instância;
- reward;
- ground drop;
- ascensão;
- counters;
- guild;
- inventário;
- equipamento.

Persistência e publicação nunca devem ficar em ordem inversa quando isso puder
confirmar ao client uma operação ainda não persistida.

---

# Identidades

Distinguir sempre:

```text
ClientID       = identidade efêmera da entidade live / wire
CharacterUID   = identidade estável do personagem
Item UID       = identidade estável server-side do item materializado
RuntimeID      = identidade server-side do gameplay space
```

`ClientID` pode ser reutilizado após disconnect.

Portanto `ClientID` não deve ser usado como identidade persistente ou durável
para:

- ownership;
- affects persistidos;
- reconnect;
- rewards;
- membership resumível;
- delayed actions long-lived;
- estado salvo no banco.

Sempre verificar risco de ID reuse em qualquer referência que sobreviva ao
tick ou à sessão que a criou.

UID de item nunca vai ao wire.

---

# Gameplay space

`RuntimeID` é uma fronteira autoritativa de gameplay.

```text
RuntimeID == ""  → mundo público
RuntimeID != ""  → runtime privado/compartilhado
```

Para entidades dinâmicas, dois participantes pertencem ao mesmo espaço somente
quando os RuntimeIDs são exatamente iguais.

A regra deve valer, conforme aplicável, para:

- Player ↔ Player;
- Player ↔ Mob;
- Mob ↔ Player;
- Mob ↔ Mob;
- summons;
- boss;
- skills;
- AoE;
- affects;
- AI;
- aggro;
- party EXP;
- drops;
- ground items;
- rewards;
- collision;
- movement;
- spawn;
- teleport;
- reconnect.

Nunca inferir gameplay space por coordenadas.

Entidades em runtimes diferentes podem ocupar exatamente as mesmas coordenadas.

NPCs globais, merchants e objetos permanentes devem possuir exceções explícitas
quando necessário. Não implementar exceção implícita apenas porque
`RuntimeID == ""`.

`Merchant != 0` nunca é hostil nem atacável.

---

# Visibilidade e atualização

Não usar `CreateMob` para atualizar jogador que já está visível.

Para entidade já materializada, usar packets incrementais apropriados:

- score;
- affects;
- equipamento;
- movimento;
- HP;
- estado.

`CreateMob` é para materialização inicial.

---

# Índice espacial

Usar o índice espacial para consultas locais.

Não varrer todos os mobs ou todos os players em ticks frequentes quando a
consulta é espacial.

Ao adicionar uma nova mecânica de tick, verificar explicitamente sua
complexidade.

---

# Inventário

Inventário:

```text
64 slots estruturais
63 slots visíveis
```

Cargo:

```text
128 slots estruturais
120 slots visíveis
```

Não confundir índice estrutural com célula visível do client.

---

# Dados autoritativos

Arquivos sob `data/` são a fonte autoritativa de conteúdo.

## Itens

`itemlist.csv` define:

- itens;
- efeitos estáticos;
- valores estáticos do item.

## Nomes

`Itemname.csv` substitui somente nomes.

Não deve alterar semântica.

## Efeitos

`ItemEffect.h` define a relação:

```text
ID ↔ EF_*
```

dos efeitos persistidos.

## Skills

`SkillData.csv` define todos os parâmetros das skills.

Não duplicar valores desses arquivos em:

- maps hardcoded;
- tabelas Go;
- switches de correção;
- overrides pós-load;
- constantes duplicadas.

Se um valor existe em arquivo autoritativo, o código deve carregá-lo.

---

# Organização

## model

Domínio puro.

Sem protocolo, store ou sessão.

## wire

Somente:

- protocolo;
- criptografia;
- serialização;
- builders.

Não colocar regra de gameplay.

## net

Somente:

- conexão;
- sessão;
- transporte.

Não colocar regra de jogo.

## store

Persistência.

## data

Loaders e validação dos arquivos autoritativos.

## game

Coordenação e regras de gameplay.

Fragmentar por feature.

Handlers devem:

```text
decodificar
→ validar envelope básico
→ rotear
```

Não colocar lógica extensa em:

```text
world.go
handlers.go
```

Criar arquivo específico da feature quando a regra crescer.

---

# Texto

Texto enviado ao client deve ser em inglês.

Logs e comentários internos podem ser em português.

---

# Protocolo WYD 7.48

O protocolo é byte-exato.

Nunca considerar um packet correto apenas porque os campos aparentam estar
certos.

Ao criar ou alterar packet:

1. confirmar opcode;
2. confirmar tamanho;
3. confirmar offsets;
4. confirmar tipos;
5. confirmar signed/unsigned;
6. confirmar padding;
7. confirmar ID do header;
8. confirmar comportamento do client;
9. comparar com packet nativo quando possível.

Um byte extra ou ausente pode fazer o client ou servidor rejeitar o packet.

Não portar layouts diretamente de 7.54/7.59 para 7.48.

---

# Patches do client 7.48

Patches no executável exigem nível de evidência maior que código Go.

Nunca assumir que um endereço ou ABI está correto porque:

- existe no W2PP;
- existe em outro client;
- o endereço parece estar dentro da função correta;
- o patch compila;
- o SHA final bate;
- um teste estático encontra o CALL/JMP.

Para cada hook/trampoline:

1. confirmar bytes originais;
2. desassemblar o contexto;
3. identificar a função;
4. identificar calling convention;
5. identificar stack frame;
6. identificar registradores live;
7. identificar stack cleanup;
8. identificar caminho de retorno;
9. preservar registradores exigidos;
10. confirmar endereço de retorno;
11. verificar instruções deslocadas;
12. reproduzir a chamada nativa quando houver equivalente.

Não transplantar:

```text
[EBP-offset]
ECX=this
stack offsets
```

de outra função sem comprovação.

Um teste estático de bytes não substitui teste runtime.

Patches devem:

- ter guarda SHA;
- verificar bytes originais;
- ser reproduzíveis;
- possuir rollback;
- documentar entrada e saída;
- não fazer edição manual invisível.

---

# Auditoria

Quando o usuário pedir:

- revisar;
- auditar;
- validar;
- verificar se foi corrigido;
- fazer auditoria profunda;

não fazer revisão superficial.

A auditoria deve seguir o comportamento de ponta a ponta.

## Procedimento obrigatório

### 1. Identificar a invariável

Exemplo:

```text
um player de Runtime A nunca pode afetar Runtime B
```

### 2. Encontrar todas as representações envolvidas

Exemplo:

```text
Player
Mob
Affect
GroundItem
ItemInstance
Party
Summon
```

### 3. Encontrar todos os caminhos de entrada

Exemplo:

```text
attack
skill
AoE
tick
AI
boss
summon
reward
reconnect
```

### 4. Verificar mutação

Checar:

- estado antes;
- validação;
- estado depois;
- rollback.

### 5. Verificar publicação

Checar o que o client recebe.

### 6. Verificar persistência

Checar restart/relogin quando relevante.

### 7. Procurar bypasses

Pesquisar:

- helpers antigos;
- lógica duplicada;
- checks especializados;
- callers que não usam o novo helper;
- fallbacks;
- IDs efêmeros;
- scans globais;
- branch legado.

### 8. Revisar testes

Testes devem reproduzir o bug real.

Não considerar suficiente um teste que apenas chama o helper corrigido.

### 9. Dar parecer

Classificar findings:

```text
CRITICAL
HIGH
MEDIUM-HIGH
MEDIUM
LOW
INFO
```

Para cada finding fornecer:

- comportamento;
- causa;
- cenário reproduzível;
- impacto;
- evidência;
- correção recomendada.

Findings primeiro. Resumo depois.

---

# Não declarar conclusão prematuramente

Não escrever:

```text
corrigido
resolvido
concluído
pronto para merge
implementação completa
```

sem evidência suficiente.

Usar estados distintos:

```text
IMPLEMENTED
STATICALLY VERIFIED
AUTOMATED TESTED
INTEGRATION TESTED
CLIENT-TESTED
PRODUCTION-VALIDATED
```

Exemplo:

```text
Runtime isolation:
IMPLEMENTED + AUTOMATED TESTED
client multiplayer acceptance pending
```

Isso evita transformar teste unitário em validação in-game.

---

# Relato do usuário versus validação própria

Quando o usuário disser:

```text
os testes passaram
foi corrigido
testei no client
```

trate como:

```text
user-reported validation
```

Não apresente como verificação independente.

Se puder inspecionar diretamente, inspecione.

Na resposta, diferencie:

```text
Confirmado por inspeção
Confirmado por CI
Relatado pelo usuário
Ainda não verificado
```

---

# Testes

Toda correção de bug deve adicionar teste que reproduza a falha real.

Preferir:

```text
arrange estado real
→ executar caminho de produção
→ verificar comportamento final
```

a testar diretamente um helper isolado.

Quando existir risco cross-runtime, usar no mínimo:

```text
PUBLIC
Runtime A
Runtime B
```

Quando existir risco de identidade:

```text
A recebe ID X
A cria estado
A desconecta
B recebe ID X
verificar que B não herda autoridade de A
```

Quando existir persistência:

```text
estado inicial
→ salvar
→ reconstruir World
→ restaurar
→ validar invariantes
```

---

# Revisão após implementação

Depois de aplicar um plano ou patch, não verificar somente as linhas alteradas.

Fazer uma segunda passagem procurando:

- código antigo ainda alcançável;
- helpers duplicados;
- callers esquecidos;
- fallback incompatível;
- tests que cobrem apenas happy path;
- inconsistência de nomenclatura;
- documentação afirmando mais que os testes provam.

Pesquisar por símbolos antigos relacionados ao bug.

Exemplo:

```text
grep/search por:
oldHelper
OwnerID
ClientID
positionOccupied
InstanceID
MaxPlayers
```

e justificar cada ocorrência restante.

---

# Mudanças

Preservar mudanças não relacionadas no worktree.

Não refatorar áreas fora do escopo apenas por preferência estética.

Correções devem ser pequenas e auditáveis sempre que possível.

Separar:

```text
bug fix
refactor
performance
documentation
```

em commits distintos quando forem independentes.

---

# Validação obrigatória

Na raiz `wyd-go`:

```powershell
go test -count=1 ./...
go vet ./...
go build -o tmp/tm-check.exe ./cmd/server
git diff --check
```

Quando o ambiente suportar:

```powershell
go test -race ./...
```

Não afirmar que qualquer comando passou sem realmente executá-lo ou possuir
resultado verificável de CI.

Remover somente artefatos criados pela própria validação.

Preservar o restante do worktree.

---

# Git e PR

Antes de considerar PR pronto:

1. revisar diff completo relevante;
2. verificar arquivos alterados;
3. verificar testes adicionados;
4. verificar documentação;
5. verificar status CI;
6. distinguir testes locais relatados de CI observado;
7. procurar regressões fora do trecho alterado.

PR em draft permanece draft enquanto existir finding blocker conhecido.

---

# Documentação

Estado entregue:

```text
DOCS/IMPLEMENTED.md
```

Único roadmap:

```text
DOCS/ROADMAP.md
```

Manuais locais que permanecem junto do artefato descrito:

```text
client748/PATCHES.md
data/boss/README.md
```

Não recriar planos paralelos. Uma feature concluída sai de `ROADMAP.md` e entra em
`IMPLEMENTED.md` no mesmo commit.

Documentação descreve intenção e estado conhecido; código continua sendo a
autoridade para auditoria da implementação.

---

# Comportamento esperado do agente

Para tarefas simples, responda diretamente.

Para tarefas técnicas complexas:

1. estabeleça o problema;
2. investigue;
3. mantenha o usuário informado durante trabalhos longos;
4. apresente findings assim que forem confirmados;
5. continue procurando problemas relacionados;
6. entregue conclusão somente depois da revisão completa.

Não faça perguntas desnecessárias quando o código ou as fontes disponíveis
podem resolver a dúvida.

Quando houver incerteza material, investigue em vez de escolher a hipótese mais
conveniente.

Nunca preencha lacunas técnicas com suposição silenciosa.

Se algo não puder ser confirmado, diga explicitamente:

```text
não confirmado
```

e informe o que falta para confirmar.

---


# Regra final

Neste projeto, uma resposta tecnicamente conservadora é melhor que uma
conclusão confiante baseada em evidência incompleta.

Quando houver conflito entre:

```text
comentário
documentação
relatório de implementação
teste superficial
código executado
```

a prioridade é descobrir o comportamento real do sistema.

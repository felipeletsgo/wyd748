# Continuidade eficiente entre sessões

Este protocolo reduz contexto repetido sem transformar anotações antigas em
fonte de verdade. O código, os testes, os dados, o binário 7.48 e o Ghidra
continuam autoritativos conforme o escopo.

## Estrutura

```text
AGENTS.md                         invariantes globais e roteamento curto
<subtree>/AGENTS.md               regras específicas do diretório
.agents/skills/<skill>/SKILL.md   workflow e índice de referências
.agents/skills/*/references/      contratos estáveis carregados por assunto
.agents/handoffs/<escopo>.md      estado operacional e retomada
```

Não duplicar o mesmo contrato em todas as camadas:

- regra universal fica no `AGENTS.md` mais alto aplicável;
- workflow fica na skill;
- evidência técnica extensa fica em referência;
- estado transitório fica no handoff;
- decisão executável fica no código/teste/dado, não na documentação.

## Início de uma nova sessão

1. Ler o `AGENTS.md` da raiz e os scoped dos arquivos que provavelmente serão
   tocados.
2. Inspecionar `.agents/skills` e abrir integralmente as skills aplicáveis.
3. Se a solicitação continuar um escopo existente, abrir apenas o handoff
   correspondente.
4. Rodar `git status --short` antes de qualquer edição.
5. Verificar fatos baratos e sujeitos a drift: existência de arquivos, hashes,
   branch, último build, processos e linhas atuais do código.
6. Carregar referências técnicas somente quando roteadas pela skill.
7. Corrigir no handoff qualquer informação stale detectada.

Não reler toda a referência histórica, todo o chat ou todo o repositório para
“recuperar contexto”. Começar pelo ponto de retomada e ampliar somente quando a
evidência exigir.

A revisão inicial termina assim que `status + diff scoped + fingerprints dos
inputs` confirmarem o ponto de retomada. Depois disso, executar o próximo passo;
não iniciar uma segunda auditoria preventiva da mesma evidência.

## Guarda contra looping

Compactação ou interrupção não invalida automaticamente o handoff. É proibido
reiniciar a recuperação por reflexo: não listar tarefas, reler o chat ou repetir
triagem global quando o handoff, a worktree e o próximo símbolo continuam
válidos.

Uma retomada deve seguir este limite:

1. uma única checagem curta de `git status` e diff scoped;
2. o próximo comando é um patch, teste focado ou diagnóstico específico;
3. se duas chamadas não trouxerem alteração, teste ou evidência nova, parar e
   informar o bloqueio verificável.

Não contar mensagens de progresso como avanço. Após uma interrupção, o ciclo
termina; somente uma nova solicitação explícita autoriza uma retomada. Essa
retomada não deve repetir o plano, o handoff ou as inspeções já concluídas.

## Cache e invalidação

Registre no handoff o input que torna um resultado reutilizável:

| Resultado | Repetir quando |
| --- | --- |
| SHA-256 do nativo | caminho, tamanho ou mtime mudarem, ou faltar fingerprint confiável |
| triagem/censo | catálogo, corpus, script ou objetivo da raiz mudar |
| export/fingerprint Ghidra | binário/projeto/script ou funções solicitadas mudarem |
| leitura de referência | arquivo mudar ou surgir uma decisão não coberta |
| `validate_research.py` | ficha, template, schema ou validador mudar |
| build client | source, asset ou input de build mudar |
| testes Go | código/dado/contrato consumido pelo teste mudar; suíte ampla no gate de integração |

Resultado com inputs idênticos é evidência reutilizável, não trabalho pendente.
Não duplicar comandos só para gerar um timestamp mais novo.

## Conteúdo obrigatório de um handoff

- objetivo exato e limites do escopo;
- artefatos canônicos e hashes relevantes;
- evidência confirmada com data e origem;
- arquivos alterados e observação sobre ownership da worktree;
- último comando executado e resultado verificável;
- estado por item usando a taxonomia de validação;
- falhas conhecidas, hipóteses explicitamente não confirmadas e riscos;
- próximo passo executável, com comando ou arquivo de entrada quando útil;
- critérios de aceite ainda pendentes.

Handoff deve ser curto, factual e regravável. Não incluir raciocínio privado,
transcrição de conversa, screenshots embutidos, dumps completos, logs extensos
ou cópias de código. Referenciar o caminho e a linha/âncora em vez de duplicar.

## Quando atualizar

Atualizar o handoff quando ocorrer qualquer um destes eventos:

- nova função Ghidra, hash ou contrato confirmado;
- mudança material de código/dado/asset;
- build, teste ou execução real altera o estado de validação;
- crash produz nova causa ou minidump;
- abordagem é abandonada ou uma hipótese é refutada;
- sessão termina com trabalho incompleto e ponto claro de retomada.

Não atualizar apenas para registrar atividade sem mudança de conhecimento.

## Verificação contra drift

Uma sessão nova deve tratar como volátil:

- hash de `project.exe` quando houver novo build;
- conteúdo de logs e dumps;
- PID/processo em execução;
- resultado de build/teste anterior quando input, toolchain, ambiente ou gate
  de integração tiver mudado;
- lista de mudanças da worktree;
- line numbers de arquivos em edição.

Reverificar somente o item cujo gatilho mudou. Para artefato histórico
imutável, conferir primeiro caminho/tamanho/mtime e o hash registrado;
recalcular somente pelos gatilhos da tabela acima.

## Concorrência e múltiplas sessões

- Cada handoff deve ter um único escopo claro.
- Registrar arquivos ativos para reduzir edição concorrente acidental.
- Não assumir que mudança não reconhecida pertence a esta sessão.
- Antes de editar arquivo listado por outro escopo, inspecionar o diff e
  preservar ambos os trabalhos; pedir direção somente se houver conflito real.
- Não usar handoff como lock. O `git diff` atual sempre prevalece.

## Fechamento da sessão

Antes de parar trabalho incompleto:

1. rodar validações proporcionais ao que mudou;
2. registrar somente resultados realmente observados;
3. atualizar estado e próximo passo no handoff;
4. deixar hipóteses e itens não testados explicitamente marcados;
5. garantir que nenhum segredo, caminho pessoal desnecessário ou transcript foi
   gravado.

Use `.agents/handoffs/TEMPLATE.md` para um novo escopo. Remover um handoff só
quando o trabalho estiver concluído e o conhecimento estável já estiver no
código, teste ou referência apropriada.

# Continuidade eficiente entre sessões

Use na retomada de trabalho incompleto. Entrada única, limites de repetição e
gates de validação estão no [AGENTS.md](../../../../AGENTS.md); esta referência
detalha apenas como manter evidência e próximo passo reutilizáveis.

## Retomar o ponto atual

Abrir somente o handoff do escopo, se existir, e confrontá-lo com a checagem
inicial da árvore. Se `HEAD`/diff mostrarem trabalho incorporado, revertido ou
substituído, descartar a premissa antiga. Não reconstruir o diff histórico.
Sem handoff, usar o arquivo/contrato da tarefa; não criar um apenas para começar.

O próximo passo vem do estado atual: patch, teste focado ou consulta que resolve
uma lacuna identificada. Não recuperar toda a conversa nem listar tarefas quando
o escopo já estiver claro. Não repetir a entrada ao trocar de skill.

## Reutilizar resultados

Registrar o vínculo entre evidência e inputs quando ele for necessário para
decidir se o resultado continua válido. Não calcular todos os hashes por padrão.

| Evidência | Gatilho para reverificar |
| --- | --- |
| Identidade do nativo | Arquivo substituído, metadados divergentes ou identidade registrada insuficiente; nesse caso confirmar hash antes de reutilizar endereços. |
| Censo/triagem | Corpus, catálogo ou triador alterado; uma nova raiz pode ser localizada por seed sem refazer o censo. |
| Export Ghidra | Novo binário/projeto/script ou pergunta não coberta pelo export válido. |
| Leitura de referência | Conteúdo alterado, indisponível no contexto ou decisão ainda não coberta. |
| Ficha validada | Ficha, template, schema ou validador alterado. |
| Teste/build | Código, dados, dependências, toolchain ou ambiente relevante alterado; incluir consumidores e gate integrado do lote. |
| Fluxo no client | Executável, assets, contrato server ou cenário relevante alterado; identificar o candidato efetivamente executado. |

Logs, PIDs e números de linha são voláteis: consultar somente se necessários à
próxima ação. Evidência ausente ou teste ignorado continua pendente. Resultado
anterior reusado deve ser identificado como anterior, não como nova execução.

## Handoff mínimo

Usar o [template](../../../handoffs/TEMPLATE.md) quando uma entrega incompleta
precisar de continuidade. Atualizar o registro do escopo, sem criar um por patch:

- objetivo e limites;
- `HEAD` observado e arquivos ativos, distinguindo mudanças alheias;
- decisão/evidência confirmada e seu caminho canônico;
- última validação: comando, resultado, inputs/ambiente relevantes;
- lacunas e critérios de aceite pendentes;
- próximo passo executável e condição de bloqueio, se existir.

Atualizar no fechamento do lote ou antes de parar com trabalho incompleto,
quando houver mudança material nesses itens. Não registrar atividade sem
mudança de conhecimento. Links substituem dumps, código copiado e transcrições.

Handoff não é lock nem prova superior à implementação. Preservar trabalho
concorrente; pedir direção somente diante de conflito real. Remover o registro
apenas quando concluído e seu conhecimento durável já estiver no local canônico.

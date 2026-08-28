# Método de pesquisa do client 7.48

## Fontes e limites

Use as fontes nesta ordem para responder perguntas sobre o client:

1. executável histórico `WYD.exe` com hash confirmado e projeto Ghidra;
2. assets 7.48 realmente consumidos;
3. protocolo e comportamento atuais do WYD-Go;
4. source recompilável atual, tratada como implementação candidata;
5. TMProject/W2PP/Secrets/Micronics como referências semânticas secundárias.

O corpus `.c` exportado permite busca em massa, mas perde informação importante:
xrefs de dados, chamadas indiretas, tipos, stack, estruturas e parte do call
graph. Uma ocorrência textual ausente não prova ausência de caller. Resolver no
projeto Ghidra antes de promover a ficha a `TRACED`.

## Delimitação do fluxo

Comece por uma transição com entrada e saída observáveis. Exemplos úteis:

- bytes recebidos -> packet validado -> handler -> HUD atualizado;
- clique em slot -> seleção/drag -> intenção enviada -> rollback recebido;
- packet de login -> cena criada -> recursos vinculados -> mundo ativo;
- tecla -> root de UI alternado -> focus alterado -> janela destruída;
- affect recebido -> estado local -> efeito renderizado -> expiração/remoção.

Evite títulos baseados em arquivo (`TMFieldScene.cpp`) ou feature ampla
(`inventário`). O mesmo arquivo contém muitos fluxos e um fluxo cruza arquivos.

## Coleta nativa

Para cada entrada:

1. registrar endereço, nome atual, assinatura inferida e calling convention;
2. registrar callers diretos, xrefs indiretos relevantes e motivo de entrada;
3. registrar callees que mutam estado, emitem rede, carregam recurso ou alteram
   lifecycle;
4. identificar globals lidos/escritos e ownership dos ponteiros;
5. descrever precondições, branches de sucesso, rejeição, teardown e retry;
6. registrar constantes, strings, IDs, opcodes e layouts somente com contexto;
7. reexportar função incompleta com timeout maior antes de inferir o bloco
   ausente.

Ao usar `ExportWydFlow.java`, passe somente as funções, bases de vtable e slots
que respondem à pergunta da ficha. A janela de dados mostra contexto estrutural,
mas os xrefs de um slot só entram como prova quando o endereço exato também foi
solicitado. Não versionar uma varredura ampla apenas para manter possibilidades
de busca futura.

Para procurar dispatch virtual computado, usar `virtualslot:<offset>`, com o
offset hexadecimal com ou sem prefixo `0x`. O relatório registra cada
`CALL [reg+offset]` compatível e termina com `virtual_slot_search`, incluindo
hits, candidatos e instruções varridas. O mesmo offset aparece em vtables de
classes diferentes: owner da instrução não é owner do receptor. Antes de nomear
a classe, provar a origem do registrador receptor, o vptr observado, a vtable e
a transição de lifecycle que torna aquela instância alcançável.

O Ghidra headless pode capturar uma exceção de script e ainda encerrar o processo
com código `0`. Toda execução deve validar em conjunto: ausência de
`SCRIPT ERROR` no log, presença do SHA-256 esperado na linha `program` e presença
do resumo do modo solicitado no TSV. Argumentos inválidos devem produzir
`SCRIPT ERROR` e nenhum resumo aceito; não tratar apenas o exit code como teste.

### Correlação diferencial em lote

`ExportWydFingerprints.java` extrai, para cada função, hashes de bytes,
mnemônicos, operandos normalizados, p-code e CFG, além de strings, imports,
constantes e vizinhos diretos. Executá-lo em projetos Ghidra separados e
read-only para o binário 7.48 e para o `project.exe` exato que será comparado.
O export precisa conter o SHA-256 do programa; o correlator deve também validar
os binários quando eles estiverem disponíveis.

`correlate_fingerprints.py` usa unicidade, melhor candidato recíproco, margem e
suporte limitado do callgraph. Interpretar as classes assim:

- `EXACT_MATCH`: fingerprint estrutural único ou bytes únicos, ainda sujeito a
  revisão do fluxo;
- `STRONG_MATCH`: candidato único com sinais estáveis e margem suficiente;
- `CANDIDATE`: útil para ordenar análise manual, sem decisão comportamental;
- `NO_MATCH`: nenhuma correlação útil; não prova ausência na source.

A correlação reduz a busca, mas não prova semântica, ABI, tipo do receptor,
ownership, alcançabilidade ou lifecycle. Revisar no projeto Ghidra a raiz e os
vizinhos relevantes antes de registrar um nome ou promover a ficha. Os exports
completos são caches regeneráveis e ficam fora do Git; versionar apenas recortes
focados que sustentem claims de uma ficha.

Quando uma função não possuir xref tipado, usar `pointers:<entry>` para procurar
o VA bruto de 32 bits em toda a memória carregada. Registrar o número de hits e
classificar cada slot no projeto. Zero hits elimina apenas essa representação
literal; não elimina chamada calculada, thunk, slot construído em runtime ou
referência ainda não materializada pela análise.

Para distinguir a ausência de ponteiro literal da ausência de chamada direta,
usar também `relatives:<entry>`. Esse modo percorre a listing decodificada e
resolve somente `CALL E8` e `JMP E9` de cinco bytes pelo deslocamento `rel32`
assinado. Validar o scanner com pelo menos um alvo conhecido no mesmo binário.
Zero hits elimina apenas esses branches decodificados; chamadas indiretas,
regiões não analisadas e destinos construídos em runtime continuam pendentes.

Quando o entry point não possuir xref, usar `bodyrefs:<entry>` para procurar
xrefs externos destinados a qualquer instrução do corpo da função, não somente
à entrada. O relatório separa `FLOW`, `DATA`, outros tipos e thunks reconhecidos,
além de registrar a instrução imediatamente anterior e seu fallthrough. Um xref
`FLOW` externo ou thunk pode revelar outra entrada estática; xrefs `DATA` que
formam jump tables internas descrevem o CFG do switch e não são callers. A
ausência de `FLOW` e thunk ainda não elimina chamada indireta ou destino
calculado em runtime.

Usar `rawrelatives:<entry>` para completar o scan da listing: ele percorre cada
byte dos blocos executáveis inicializados e resolve candidatos `E8/E9 rel32`,
inclusive em regiões ainda não decodificadas. Cada hit é classificado como
`instruction`, `data` ou `undefined` e precisa de revisão no projeto Ghidra.
Zero hits elimina apenas essa codificação nos blocos e bytes registrados; não
elimina chamada indireta, destino derivado, tabela construída em runtime ou
memória que não estava materializada no programa analisado.

Executar `bodyrefs` e `rawrelatives` também contra funções com callers diretos
conhecidos no mesmo binário. Registrar hits, candidatos, bytes, blocos e erros;
se os controles positivos não recuperarem as entradas esperadas, o zero do alvo
é inválido e não pode sustentar uma conclusão.

Um endereço sem módulo/hash não é estável. Em crash, registrar módulo, base
carregada, RVA, VA preferida e ponteiro inválido.

## Wire e ABI

Uma ficha de wire deve conter:

- direção (`C->S`, `S->C` ou local);
- opcode e tamanho total;
- header, offsets, larguras, signedness e packing;
- construtor/emissor e consumidor nativos;
- packet final maior que embute a struct, quando aplicável;
- equivalente no `internal/wire` e validação de entrada no servidor;
- corpus byte-a-byte ou teste que torne a conclusão falsificável.

Uma tabela de `static_assert` na source candidata protege a implementação atual,
mas não prova sozinha que o layout veio do 7.48.

## UI, input e render

Registrar em conjunto:

- arquivo de recurso, root e IDs filhos materializados;
- função de construção e estado inicial;
- binding na source candidata;
- entrada de mouse/tecla e hit-test;
- callback que abre/fecha e condição de visibilidade;
- ownership, foco, teardown e acesso quando fechado;
- resolução/escala usada na comparação;
- assets, textura, mesh, blend/depth e ordem de desenho quando visual.

Controle moderno ausente pode ser `nullptr`; provar todos os callers e preservar
a transição principal. Não fabricar um ID para satisfazer a topologia 7.59.

## Matriz de delta

Compare por claim, não por arquivo inteiro:

| Claim | Nativo 7.48 | Source atual | TMProject | WYD-Go | Decisão |
| --- | --- | --- | --- | --- | --- |
| comportamento/layout | evidência | implementação | origem/risco | contrato | portar/remover/manter/não confirmar |

Use estas classes:

- `CONFIRMED`: sustentado por evidência primária reproduzível;
- `PROBABLE`: múltiplas pistas coerentes, ainda falta uma ligação;
- `HYPOTHESIS`: candidato de busca, nunca contrato;
- `REJECTED`: contradito por evidência superior.

## Revisão e promoção

Antes de promover a ficha:

1. executar `validate_research.py`;
2. reabrir pelo menos a entrada, um caller e um callee relevante;
3. conferir que o hash ainda corresponde;
4. garantir que gaps e claims não confirmados não foram usados na decisão;
5. registrar testes reproduzíveis e estado real da validação.

`CLIENT_TESTED` exige execução do fluxo no `client748/project.exe`; build ou
startup isolado não bastam.

Para fichas de lifecycle em `TRACED`, `CONTRACT` ou `CLIENT_TESTED`, o validador
exige conteúdo verificável para entrada observável, matriz de transições,
vtables/vptrs/receptores, ownership, falha parcial, cleanup/teardown, shutdown e
logout/relogin. Quando um item realmente não se aplicar, registrar `N/A:` seguido
da justificativa; marcador vazio não fecha o gate.

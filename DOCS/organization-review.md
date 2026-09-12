# Revisão da organização — 12/09/2026

## Resultado

O repositório tem duas sources ativas: `tmproject/` (client C++ derivado do
TMProject 7.69, adaptado para o produto 7.48) e `wydgo748/` (servidor Go
autoritativo). A reorganização não declara o jogo completo nem comprova
compatibilidade integral entre todas as features.

- [AGENTS.md](../AGENTS.md) é o único arquivo de regras.
- [Índice](README.md) e [inventário](documentation-map.md) centralizam a documentação.
- Cinco skills cobrem organização, servidor/integração, pesquisa nativa,
  catálogo e assets; referências técnicas não duplicam mais as regras globais.
- Documentação saiu das sources. Pesquisa e handoffs permanecem em `.agents/`;
  binários e patchers históricos ficam separados em `references/client748/`.
- Comandos de build, CI, testes que leem assets e ferramentas de pesquisa usam
  os caminhos atuais. Builds novos ficam em `tmproject/build/` e `wydgo748/bin/`.
- O conversor histórico de NPCs exige `-in` e `-out` explícitos. Não há busca
  automática em checkouts externos nem saída padrão dentro de dados ativos.
- O validador em `tools/repository/` verifica inventário, links locais, regras
  duplicadas, documentação nas sources e referências documentais a raízes obsoletas.

## Limpeza e preservação

Foram removidos arquivos duplicados de regras/README, regras aninhadas e
skills/workflows obsoletos. As regras globais antes repetidas nas referências
foram consolidadas. Entradas antigas de projetos, caminhos externos e a regra
LFS sem alvo existente foram retirados dos pontos operacionais revisados.
Arquivos anteriormente versionados continuam recuperáveis pelo histórico Git.

Registros técnicos antigos foram mantidos e identificados como históricos;
menções de procedência a sources excluídas não são recomendações de uso. O
score ativo foi corrigido na documentação para 35 campos uint32/140 bytes,
sem promover layouts ou sidecars antigos a contratos atuais.

Assets, executável instalado do client e 26 arquivos do acervo nativo foram
preservados. O build de verificação não substituiu `client748/project.exe`.
Não houve mudança de regra de gameplay: além da organização, o código mudou
somente em caminhos de fixtures, argumentos do conversor e comentários de
procedência.

Três executáveis locais preexistentes (`wydgo748/tm.exe`,
`wydgo748/account-create.exe`, `wydgo748/cmd/server/tm.exe`) permanecem no disco,
ignorados pelo Git. A remoção foi recusada pelo ambiente e não foi contornada.
Eles não são saídas dos comandos de build atuais.

## Validação da reorganização anterior à revisão de eficiência

| Verificação | Resultado |
| --- | --- |
| Layout e links Markdown locais | Passou; inventário reproduzível |
| Cinco skills — quick_validate | Passaram |
| Ferramentas Python de pesquisa | 12 testes passaram |
| Fichas de pesquisa | 75 válidas: 46 CONTRACT, 19 TRACED, 8 LOCATED, 2 UNMAPPED |
| Servidor — `go test -count=1 ./...` | Passou; URL de testes PostgreSQL configurada |
| Servidor — `go vet ./...` e build | Passaram |
| Conversor sem argumentos | Recusou a execução antes de ler/gravar dados |
| Client — ArchitectureTests Release/Win32 | 26.968 verificações e asserts estáticos passaram |
| Client — solução Release/x86, MSBuild v145 | Build passou; warnings existentes do compilador permanecem |
| `git diff --check` | Passou |

Artefato C++: `tmproject/build/TMProject748/Release/WYD.exe`.
SHA-256: `CAA7026AA6A4D41950730B8E762653F5C7563047E7F98503386E32C2A9A1B464`.

O workflow de CI foi atualizado, mas não houve execução remota nesta rodada.
Não foram executados conexão real, login, mundo, UI, multiplayer ou relogin.
Nenhuma ficha foi promovida a `CLIENT_TESTED` por causa desses builds.

## Escopo de evidência

Classificação: `MODERNIZACAO_COMPATIVEL` estrutural. Código atual, testes,
configuração e documentação foram utilizados. Assets e acervo nativo foram
preservados; não foram usados para inferir novos contratos. Ghidra e guias são
`NÃO APLICÁVEIS` a esta alteração de organização, que não adapta comportamento,
ABI, render ou recursos. A revisão semântica de cada feature continua sujeita
às fichas e gates de pesquisa, independentemente da limpeza dos documentos.

## Revisão de eficiência das skills — 12/09/2026

As cinco skills e as referências de execução foram ajustadas com
`repo-architecture` e `skill-creator`. O `AGENTS.md` concentra entrada única,
invalidação de evidência, interrupção de repetições sem progresso e matriz de
validação por risco. As skills selecionam o fluxo específico, sem repetir
status/HEAD, inventário ou auditoria ao trocar de etapa.

Foram retiradas exigências de suíte Go a cada feature, rebuild por asset sem
dependência de compilação, catálogo antes de toda pesquisa, reabertura de
evidência já suficiente e análise global de código morto em edição comum.
A referência de continuidade deixou de sugerir regras em subpastas. Prompts
das skills e caminhos documentais auxiliares foram alinhados ao fluxo atual.

Revisão manual de decisões (não simulação de agentes nem teste in-game):

| Cenário | Caminho previsto pelas instruções revisadas |
| --- | --- |
| Ajuste documental/skill | Validação documental e da skill; sem Ghidra ou builds do jogo. |
| Correção interna do servidor | Fluxo e teste afetados; sem pesquisa nativa se não mudar fronteira legada. |
| Adaptação com ficha CONTRACT suficiente | Reutilizar a ficha e implementar; não exportar novamente o corpus. |
| Packet legado sem layout comprovado | Resolver a lacuna wire/ABI antes de editar; testar emissor e consumidor. |
| Extensão coordenada | Especificar contrato novo e validar ambos os lados; estudar apenas integração legada interceptada. |
| Textura com ID/formato preservados | Conferir recurso/loader e aparência no client; não rebuildar sem dependência afetada. |
| Retomada ou falha repetida | Confrontar o estado uma vez; executar o próximo passo ou mudar diagnóstico com evidência, sem repetir consulta esgotada. |

Esta rodada altera apenas instruções, metadata de skills e documentação.
Os resultados de Go/C++ acima pertencem à reorganização anterior e não são
novas execuções. Não houve alteração de gameplay, assets, executável instalado
ou promoção das fichas de pesquisa. A eficácia em sessões futuras ainda depende
de seguir esses critérios; validação estrutural não garante ausência de loops.

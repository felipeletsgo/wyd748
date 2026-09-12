# Plano de implementação arquitetural

## Princípios de execução

- Uma unidade pequena por vez; nunca migrar um subsistema inteiro sem build intermediário.
- Classificação padrão: `MODERNIZACAO_COMPATIVEL`.
- Preservar ABI, packing, opcodes, nomes públicos e ordem de teardown.
- Cada etapa termina com evidência: diff focado, build Debug Win32 e teste específico.
- Fachadas temporárias são permitidas, mas devem ter proprietário e critério de remoção.

## Fase 0 — Baseline e contratos

**Entregáveis:** inventário arquitetural, mapa de includes, testes de `sizeof`/`offsetof` das mensagens críticas e registro do executável produzido.

**Gate:** build limpo; nenhum layout alterado; catálogo de contratos revisado.

## Fase 1 — Portas mínimas

Criar interfaces pequenas para `ITransport`, `IClock`, `IResourceProvider`, `IRenderSink` e `IAudioSink`. Implementar adaptadores que chamem o código atual sem mudar o comportamento.

**Gate:** cenas compilam usando os adaptadores; smoke test de bootstrap, login e encerramento.

## Fase 2 — Wire e aplicação

Mover validação de tamanho/opcode e tradução de mensagens para `wire`. Criar casos de uso para login, mudança de cena, movimento e logout. `TMFieldScene` continua como facade e delega gradualmente.

**Gate:** cada opcode migrado tem teste de entrada válida, tamanho inválido e fallback; nenhuma mensagem é enviada duas vezes.

## Fase 3 — TMFieldScene

Separar por responsabilidades, nesta ordem: dispatch de pacotes, estado de interação (trade/party/quest), mix/inventário e posicionamento de painéis. Manter a classe atual como coordenadora de lifecycle.

**Gate:** abrir/fechar cena, reconectar, trade e mix preservam estado; build e teste manual de dois clientes quando aplicável.

## Fase 4 — Domínio de entidades

Extrair primeiro funções puras de `TMHuman` e entidades: cálculo, validação, seleção e transformação de estado. Depois introduzir comandos/resultados de domínio. O adaptador visual permanece fora do domínio.

**Gate:** testes determinísticos das regras; nenhum include de Win32/DirectX em `domain`.

## Fase 5 — UI e render

Dividir `SGrid` em modelo de dados e controle visual. Criar portas de render para efeitos e mundo; mover detalhes DirectX para adapters/render. Preservar IDs, callbacks e fechamento.

**Gate:** inventário, equipamento, trade e painéis existentes funcionam; validação visual manual somente após build estável.

## Fase 6 — Redução de `core`

Mover tipos para seus donos (`domain`, `wire`, `platform`) e reduzir `Basedef` a facade/adaptadores. Remover includes transitivos somente quando todos os consumidores incluírem o contrato correto.

**Gate:** busca de dependências proibidas, build completo e comparação de layouts/opcodes.

## Fase 7 — Endurecimento e limpeza

Adicionar documentação de ownership/lifecycle, eliminar fachadas sem consumidores, ativar warnings relevantes e atualizar o mapa arquitetural. Não remover código legado apenas por ausência no nativo 7.48.

**Definição de concluído:** todos os domínios têm dono claro, dependências seguem as regras, contratos críticos têm testes e o fluxo bootstrap→login→cena→mundo→logout foi validado.

## Ordem operacional por ciclo

### Entrada de build e estado da entrega

Usar `./client-source/tmproject/Build-Client.ps1 -Configuration Release`
(ou `Debug`; `-Rebuild` para recompilacao completa). O script local descobre
MSBuild via Visual Studio/PATH, compila a solucao x86 e instala
`client748/project.exe`. Divergencia SHA-256, falha de MSBuild ou copia
interrompem a entrega. Nao encerra o jogo automaticamente.

Baseline anterior de 2026-09-05: Release e copia verificados com SHA-256
`2CEA4BCD8EE94A8C2E0E4D9640D941F3F8AF53B0D1A65D02A3F5A82ABFFFED49`.
Isso nao comprova teste in-game das migracoes recentes de PacketView.
Classificacao: `MODERNIZACAO_COMPATIVEL`; procedencia do script: local.
As fases 1 a 7 continuam sujeitas aos seus gates: contar chamadas migradas
nao comprova inversao de dependencias nem separacao de responsabilidades.

### Lote de validacao de transporte — 2026-09-05

- Implementado: predicado puro de intervalo em `PacketView`, usado antes dos
  casts pelas fachadas `TMUtil`, `CPSock` e `ITransport`.
- Implementado: gate automatico `ArchitectureTests.vcxproj`, executado por
  `Build-Client.ps1` antes do client e da instalacao; 12 verificacoes de limites
  e nulabilidade, ativas inclusive em Release e sem dependencias Win32/DirectX.
- Evidencia: `AddMessage` modifica o cabecalho do buffer original. Documentado
  o requisito de memoria gravavel; uma view const ainda nao resolve ownership.
- Debug: build e instalacao passaram; SHA-256
  `5A66C659A447FDDE74B314DBCDA8A02AD4ECC5068EDA87EB918BAADC8D372C61`.
- Release: build, 12 testes e instalacao passaram; candidato atual SHA-256
  `C06D3B39A77337D5A14FA1CA924DF5A23B193CFFF651808039663D483B59D458`.
  Permanecem warnings legados C4018/C4305/C4309; nao houve teste in-game.
- Fase 2 permanece parcial. Estes testes verificam o predicado de comprimento,
  nao criptografia, dispatch, casos de uso, relogin ou funcionamento no jogo.
- Lote seguinte implementado: `MutablePacketView` separado da recepcao;
  `PacketSendBoundary.h` permite testar o emissor sem socket. Os 140 pontos de
  envio passaram a fornecer `char*` e nao dependem de remover const na fachada.
  Nenhum buffer e copiado: os efeitos de preenchimento continuam no original.
- Validacao deste lote: 22 checks com emissor falso e 2 asserts de tipagem
  passaram em Debug e Release. Debug compilado/instalado com SHA-256
  `273017A66D93CEC7A60A119695CB5EFCC3F38F29850DE073C8726BC7BA0466C4`.
  Release compilado/instalado com SHA-256
  `DE8BF7F3FC2F2AA3453E573D9F551526CB8DFBFD3D6B9FF263D8526FC5B6DA2B`.
- Lote seguinte implementado: ITransport independente de Basedef; adaptador
  SocketTransport com consumidor vivo na selecao de personagem. Detalhes abaixo.

### Lote de fila de envio — 2026-09-05

- `SendBuffer.h` centraliza limite estrito, compactacao segura e enqueue/flush.
- `CPSock` preserva cifragem, cabecalho, consumo de chave e flushes; propaga
  rejeicao de fila como melhoria deliberada, nao como paridade nativa.
- 98 checks passaram, incluindo WORD, overflow, fila cheia e envio parcial.
- Debug SHA-256 `2CE0810D03C07F636CAD8BCB3223FFB67518DC88B6C8F30DA3687EEB0A0EE826`.
- Release SHA-256 `861973D2A67BA63B99FBB018E43425804C4AF52ED30F03389D1047A0075A07F2`.
- `AddMessage2`, recepcao e teste in-game continuam pendentes.

### Endurecimento do caminho raw — 2026-09-05

- `CanAppendRaw` remove a soma signed do `AddMessage2` e rejeita ponteiro nulo,
  tamanhos negativos e overflow sem alterar a fila.
- `RefreshRecvBuffer` usa `memmove`, preservando bytes quando a origem e o
  destino se sobrepoem.
- Debug e Release passaram com 102 checks; Release instalado com SHA-256
  `D406C08758D5234C4497FFFB6D1C9B8676E418BD6D349D7E0BADFE4ADC9258C4`.
- Ainda faltam handlers de recepcao, smoke test in-game e as fases restantes
  da arquitetura; este lote nao promove maturidade nativa.

### Guard de recebimento — 2026-09-05

- `Receive` agora valida socket, buffer e faixa de `nRecvPosition` antes de
  chamar WinSock, evitando escrita fora da fila quando ela esta cheia ou em
  estado inconsistente.
- Debug e Release passaram com 102 checks; Release instalado com SHA-256
  `B00B050BA0105B3833940F5026DE7DF1E278BDE470DE3F0A68D335EDB5DF020A`.

### Desconexao ordenada — 2026-09-05

- `CPSock::Receive` trata `recv == 0` como fechamento do peer e retorna falha
  para o dispatcher executar o teardown existente; erros negativos continuam
  no mesmo caminho.
- Debug e Release passaram com 105 checks; Release instalado com SHA-256
  `453279ACEFC293BA3B91E6C3C4C2B3DF4ABD03158A61E50AFE220903ACFDFBCD`.
- TODOs de renderizacao/gameplay permanecem documentados como fora deste lote:
  sem contrato, teste ou evidência nativa suficiente, não foram alterados.

### Fronteira de despacho — 2026-09-05

- Extraido `application/ports/PacketDispatch.h`; `NewApp` consulta a politica
  antes de converter a view para os callbacks legados.
- A politica valida armazenamento e envelope minimo, sem decidir opcode ou
  alterar o payload. Foram adicionados testes de view completa e curta.
- Debug/Release passaram com 111 checks; Release instalado com SHA-256
  `A6E7B2171368323B03571D64527191B0C5DF4217109AC43C7AA56BFA43A05ADF`.
- Callbacks ainda recebem `char*`; a migracao para payload somente-leitura
  continua pendente e requer cobertura por fluxo.

### Lote de inversao do login de personagem — 2026-09-05

- `RequestCharacterLogin` depende apenas de `ICharacterLoginSender`; o encoder
  `CharacterLoginSender` implementa esta porta usando `ITransport`. A cena
  compoe o encoder e `SocketTransport<CPSock>` com lifetime local emprestado.
- `PacketView`/`MutablePacketView` pertencem a application/ports. Os quatro
  consumidores migraram para esse header, removendo a fachada wire redundante.
  Nao ha include wire/platform na application.
- Extraidos `MessageHeader.h` e `CharacterLoginPacket.h`, reexportados pelos
  headers anteriores, preservando tipos, sizeof, offsets e os 36 bytes de 0x213.
- Caso de uso nao altera debounce, existencia de personagem, timer, botoes,
  recepcao ou lifecycle. Evidencia reutilizada: `scene-transition.md`, evento
  0x1204 e callsite 0x004A3422; nao ha novo claim de paridade.
- 70 checks e assertions passaram em Debug/Release, incluindo a porta semantica
  isolada sem wire e comparacao byte a byte dos quatro slots antes do transporte.
- Debug compilado e inicialmente instalado com SHA-256
  `BCDAA50A27A1B2340AA6C793A28A5D9CB5971766415B839E263A4766CEEDFB33`.
  A repeticao apos adicionar testes passou compilacao/70 checks, mas a copia
  encontrou `user-mapped section open`. Nenhum processo foi encerrado.
- XML de projeto/filtros sem entradas duplicadas; `git diff --check` passou.
- Fechamento da limpeza: Debug/Release, 70 checks e instalacao passaram.
  Debug SHA-256 `BFC85AA5A10E90ABE8AAFA92A39424D779C0BD492275B06038D013EF79E0C36E`.
  Release atual SHA-256 `2B03DE7585E48B9096D22A73D5F2A2F27F515343934A9B9F44EDDF616DAAC440`.
  Corrigida a entrada antiga de SharedStructs no projeto/filtros para
  `CommonFiles/SharedStructs.h` local. Warnings legados permanecem; nao CLIENT_TESTED.
- Proximo gate de fluxo: login aceito/recusado, selecao, retorno, mundo e
  logout/relogin no jogo. A migracao de handlers de recepcao depende deste
  gate; callbacks ainda usam `char*` e descartam o tamanho enquadrado.
- Proxima unidade tecnica independente: revisar limites de fila de AddMessage.
  Evidencia de source: soma signed `Size + nSendPosition`; `SendOneMessage`
  ignora rejeicao de AddMessage; `RefreshSendBuffer` copia de pRecvBuffer.
  Nao assumir que true do backend comprova enfileiramento; testar fila cheia,
  envio parcial e consumo de chave antes de corrigir estes caminhos.
  Fases 1/2 continuam parciais; demais portas e casos de uso nao estao completos.

1. Selecionar um único contrato/fluxo.
2. Mapear callers/callees e dependências reais.
3. Adicionar teste ou assertion de preservação.
4. Extrair/adaptar sem alterar semântica.
5. Atualizar `.vcxproj`, filtros e documentação.
6. Compilar e executar a validação focada.
7. Registrar resultado e só então iniciar o próximo ciclo.

### Continuidade: entrada de recepcao no ObjectManager (2026-09-05)

- Modo `MODERNIZACAO_COMPATIVEL`, implementacao local. Sem novo claim nativo,
  opcode, layout wire ou mudanca no percurso de cenas/objetos.
- `NewApp` entrega a view a `ObjectManager::OnPacketView`; leitura do relogio
  e dump permanecem anteriores ao callback e agora usam cabecalho const.
- A entrada nao virtual preserva a vtable e delega ao `OnPacketEvent` virtual
  existente. O armazenamento original de CPSock continua emprestado e gravavel;
  o const_cast necessario ao legado fica no ObjectManager, nao no dispatcher.
- `packet_dispatch::Dispatch` testa o envelope e entrega uma unica vez, sem
  copiar ou reter. Rejeicao nao executa callback. Nao valida payload por opcode.
- Eventos locais/desconexao continuam usando a API anterior. Handlers de cena
  ainda recebem char*; nao alegar migracao completa nem validacao in-game.
- Testes puros: 118 checks PASS, incluindo identidade do ponteiro, opcode,
  tamanho, chamada unica e rejeicao de frame curto/nulo/vazio.
- Debug compilado e instalado: SHA-256
  `A395C7FD2777C343BAB4AFC8D4B5930421B8C03BB8F017089A708DD65FA0BC34`.
  Release compilado e instalado: SHA-256
  `08A10A1C02DA956A047C4B2274FB3E14A2D220BD6629BA7EB636410F7031280A`.
  118 checks e asserts PASS nas duas configuracoes; XML do projeto/filtros
  valido, headers novos registrados uma vez; diff --check PASS.
  Warnings legados C4018/C4305/C4309 permanecem. Nao CLIENT_TESTED.

### Continuidade: consumo especial da selecao (2026-09-05)

`ObjectManager::HandleSelectCharacterItem` agora concentra o tratamento de
`MSG_SendItem` que precisa ocorrer antes do percurso da arvore. A ordem, a
condicao `ESCENE_SELCHAR`, a atualizacao de `m_stItemCargo` e o retorno imediato
foram preservados; nenhuma validacao nova de payload foi inventada. Debug e
Release recompilados, com 118 checks PASS e instalacao do candidato. O handler
geral e os handlers de cena ainda sao legados `char*` e permanecem na fila de
migracao gradual.

### Continuidade: efeito de julgamento na selecao (2026-09-05)

Em `TMSelectCharScene`, o caso `0x3B4` foi extraido para
`HandleJudgementEffect`. A geometria, limites de indice, criacao opcional do
efeito e retorno de consumo foram mantidos; o dispatcher agora fica menor e
continua apenas roteando o packet. Debug/Release: 118 checks PASS e candidatos
instalados. Teste in-game permanece pendente.

### Continuidade: confirmacao de login na selecao (2026-09-05)

O caso `MSG_CNFCharacterLogin_Opcode` foi extraido para
`HandleCharacterLogin`. A atualizacao do timer, copia do MOB, exp, hometown,
skills, clima e transicao para `TM_FIELD_STATE` permanecem na mesma ordem.
Debug/Release recompilados com 118 checks PASS; Release instalado. A execucao
in-game ainda nao foi realizada.

- 2026-09-05: extraídos HandleCharacterCreated/HandleCharacterDeleted de TMSelectCharScene::OnPacketEvent (MODERNIZACAO_COMPATIVEL); ordem, cópias e ReloadCharList preservados. Debug validado (118 checks); Release compilado, instalação bloqueada porque client748/project.exe está aberto.

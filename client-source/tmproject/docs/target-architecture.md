# Arquitetura-alvo do TMProject 7.48

## Decisão

O projeto deve evoluir para uma arquitetura em camadas, com dependências apontando para dentro. A compatibilidade legada fica nas bordas; regras de jogo e contratos não devem depender de DirectX, Win32 ou controles visuais.

```text
platform -> adapters -> application -> domain
presentation (ui/scenes) -> application
wire <-> adapters        -> application
render <- presentation/adapters
```

`core` não deve continuar como depósito geral. Ele será reduzido em etapas: tipos de domínio vão para `domain`, contratos de mensagem para `wire`, e serviços de sistema para `platform` ou `adapters`.

## Camadas

### `domain`

Estado e regras puras: entidades, inventário, combate, fórmulas, posições e value objects. Não inclui `HWND`, DirectX, singletons globais, logging de UI ou envio direto de pacote. Funções devem receber dados explicitamente e devolver resultado/erro.

### `application`

Casos de uso e orquestração: login, mudança de cena, movimento, trade, combate, quests e ciclo do campo. Define portas para transporte, relógio, recursos, áudio e renderização. `TMFieldScene` será migrado para coordenador de casos de uso, não para dono das regras.

### `wire`

Mensagens, serialização, validação de tamanho/opcode e tradução para comandos da aplicação. Layouts legados permanecem estáveis e recebem `static_assert`/testes de tamanho antes de qualquer alteração.

### `presentation`

`ui` e cenas exibem estado e traduzem input em comandos. Controles não devem alterar entidades diretamente; callbacks chamam casos de uso e atualizam a visão. A ordem de lifecycle permanece explícita.

### `adapters`

Implementações concretas das portas: transporte atual, tabelas/arquivos, recursos, DirectX e APIs antigas. Esta é a área apropriada para encapsular globals e funções de `Basedef` durante a transição.

### `platform`

Win32, janela, entrada, mídia, temporização e bootstrap. Não contém regra de jogo nem decisão de protocolo.

## Regras de dependência

1. `domain` não inclui headers de `platform`, `render`, `ui` ou `wire`.
2. `application` depende somente de `domain` e interfaces próprias.
3. `wire`, `render` e `platform` implementam/adaptam interfaces; não são importados pelo domínio.
4. Globals existentes são acessados por um adaptador único, inicialmente compatível com `BasedefGlobals.h`.
5. Cada extração deve preservar ABI, packing, opcode, ownership e teardown.

## Sequência de migração

1. Congelar o inventário e adicionar testes de layout/opcode.
2. Extrair portas de transporte, relógio, recursos e renderização.
3. Separar mensagens de `TMFieldScene` em handlers de aplicação, mantendo um facade temporário.
4. Migrar entidades de `TMHuman` para `domain`, começando por funções puras.
5. Dividir `SGrid` em modelo de dados e controle visual.
6. Encapsular globals de `Basedef` em adaptadores e reduzir a fachada.
7. Remover dependências proibidas por camada, uma compilação por vez.

Cada etapa deve ser pequena, compilável e reversível. Build valida compilação; não permite declarar migração ou fluxo concluído sem testes correspondentes.

## Transporte incremental — fronteira em migracao

Recepcao: `NewApp` entrega `PacketView` a `ObjectManager::OnPacketView`, entrada
nao virtual que valida o envelope e adapta para o callback virtual legado.
O buffer de CPSock permanece emprestado, gravavel e valido durante a chamada;
esta entrada nao aceita armazenamento originalmente const. O percurso das cenas
e seus callbacks `char*` permanecem inalterados. A politica pura `Dispatch`
preserva ponteiro/opcode/tamanho e entrega uma vez; nao valida payload por opcode.
Desconexao e eventos locais sem frame continuam usando `OnPacketEvent`.

`internal/application/ports/PacketView.h` define o DTO não proprietário para mensagens
enquadradas. `CPSock::ReadPacketView` preserva o tamanho validado pelo socket
e `CPSock::SendPacket` valida o intervalo antes de delegar ao método legado.
`NewApp` já usa a leitura size-aware; os logins de conta e personagem usam o
envio size-aware. Os demais emissores permanecem compatíveis e serão migrados
por grupos de contrato, evitando alterações mecânicas que possam esconder
diferenças de tamanho ou ownership.

O caminho utilitário legado também expõe `SendPacket(const MutablePacketView&)`.
Ele mantém a limitação de frequência existente em `SendOneMessage`, mas permite
que macros e outros módulos antigos expressem explicitamente opcode, buffer e
tamanho sem depender da API de socket concreta.

### Limites e ownership verificados na source (2026-09-05)

Procedencia local; `MODERNIZACAO_COMPATIVEL`, sem claim novo de paridade.
`PacketView::HasSizeBetween` centraliza a verificacao pura de ponteiro e limites
inclusivos. As fachadas de envio exigem `sizeof(MSG_STANDARD)` antes de ler ou
escrever o cabecalho e rejeitam overflow antes de estreitar `size_t`.
O opcode da view continua metadado: o envio legado usa `MSG_STANDARD::Type`.
Nao ha mudanca de layout, opcode, criptografia ou ordem dos envios validos.

Atencao: `CPSock::AddMessage` escreve `Size`, `KeyWord`, `CheckSum` e `Tick` no
buffer do emissor, copiando o resultado criptografado para sua fila interna.
As fachadas de envio agora exigem `MutablePacketView`, cujo `char*` torna esse
requisito explicito. Seus 140 emissores nao removem mais const para enviar;
recepcao continua usando `PacketView`. `AsReadOnly` permite somente a conversao
segura de gravavel para leitura. A view nao possui nem prolonga a vida da
memoria e nao comprova a capacidade real alocada.
O limite `INT_MAX` evita overflow de conversao, mas nao substitui as restricoes
de capacidade da fila nem a validacao semantica de cada opcode.

`TMProject748/tests/ArchitectureTests.vcxproj` compila testes puros sem
Win32/DirectX; `Build-Client.ps1` executa-os antes de compilar/instalar o client.
`PacketSendBoundary.h` compartilha a validacao e a chamada sincrona entre
CPSock e TMUtil, com emissor injetado e sem globals. Os checks e asserts
estaticos cobrem limites, nulabilidade, overflow, tipagem mutavel, buffer
emprestado, chamada unica, mutacao visivel, rejeicao sem envio e propagacao de
falha sem retry. Nao cobrem sockets reais, criptografia ou fluxo in-game.
O dispatch legado em NewApp ainda adapta a view recebida para callbacks
`char*`; a assinatura desses callbacks nao foi migrada neste lote.
`ITransport` nao depende mais de `Basedef` nem dos tipos MSG. O adaptador
`SocketTransport<CPSock>` tem consumidor vivo no envio de selecao de personagem
via `RequestCharacterLogin`; possui lifetime local e nao fecha nem possui o
socket. O caso de uso valida a intencao; `CharacterLoginSender` monta a
solicitacao wire; UI e lifecycle ficam na cena.
`MessageHeader.h` e `CharacterLoginPacket.h` possuem as definicoes unicas dos
structs extraidos, reexportadas pelas fachadas anteriores.

Os 98 checks atuais incluem teste byte a byte dos quatro slots de login,
rejeicao de indices invalidos, falha sem retry e emprestimo do adaptador. Os
testes continuam sem headers Win32/DirectX, mas os structs wire exigem o modelo
de inteiros Windows, conferido por asserts.
Uma unidade de compilacao separada exercita o caso de uso com uma porta
semantica falsa, sem encoder: valida os quatro slots, INT_MIN/-1/4/INT_MAX,
propagacao de falha e ausencia de retry. Assim, a defesa adicional do encoder
nao pode mascarar uma regressao na validacao da aplicacao.

Neste fluxo, application depende somente de suas portas: o caso de uso chama
`ICharacterLoginSender`, implementado pelo encoder wire `CharacterLoginSender`.
O encoder depende de `ITransport`, implementado por `SocketTransport` na
plataforma. A cena compoe estes adaptadores locais, sem alterar ownership.
Os quatro consumidores da antiga fachada `wire/PacketView.h` agora incluem
diretamente a porta de application. A fachada redundante foi removida; as
definicoes e o comportamento permanecem no header proprietario.
Essa inversao vale para o fluxo extraido, nao comprova as fases 1/2 completas.

### Divida do resultado de envio legado

Na source atual, `CPSock::SendOneMessage` ignora o retorno de `AddMessage` e
devolve `SendMessageA`. Portanto o objetivo da porta (true = aceite local)
ainda nao e garantido pelo backend real quando a fila rejeita o pacote.
Os testes com backend falso demonstram propagacao, nao corrigem esse defeito.
A cena continua ignorando o resultado como antes; nao houve mudanca de fluxo.
Proximo lote deve testar rejeicao de fila, overflow, consumo de chave e envio
parcial antes de corrigir a propagacao. `RefreshSendBuffer` tambem copia da
fila de recepcao, exigindo revisao focada; nao foi alterado nesta extracao.

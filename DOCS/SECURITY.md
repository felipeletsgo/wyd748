# Segurança do servidor

Atualizado em 26/07/2026.

O client 7.48 é considerado uma interface visual não confiável. Alterar memória
com Cheat Engine, capturar/repetir pacotes com WPE ou construir um client próprio
não pode conceder autoridade sobre movimento, combate, inventário, economia ou
progressão. Toda decisão de jogo precisa ser recalculada e validada no servidor.

## Fronteira de rede

- O socket exige o `InitCode` correto antes de aceitar pacotes.
- O tamanho máximo reproduz `MAX_MESSAGE_SIZE` do `CPSock` nativo: 8192 bytes.
- Pacotes menores que o header, maiores que o limite, incompletos ou com checksum
  inválido encerram a conexão antes de chegar ao `World`.
- Cada conexão aceita no máximo 256 pacotes e 512 KiB por segundo. O limite é
  propositalmente folgado para o tráfego normal e interrompe floods grosseiros.
- O `World` confere se `Header.Size` coincide com os bytes recebidos e exige o
  tamanho exato dos opcodes 7.48 cujo layout já foi confirmado.
- Cada opcode é aceito somente na fase correta da sessão: conexão, autenticação,
  seleção de personagem ou mundo. Repetir login/entrada no mundo não materializa
  outra cópia do personagem.
- Violações de protocolo são contabilizadas por sessão. Doze recusas em um
  minuto encerram a conexão; o log é limitado para não virar outra forma de DoS.

## Movimento e wallhack

- Coordenadas escritas pelo client não são aceitas como teleporte.
- O servidor valida origem, destino, direções da rota, quantidade de passos,
  limites do mapa, terreno caminhável e diferença de altura permitida.
- Pacotes sem rota só confirmam segmentos curtos e inteiramente transitáveis.
- Um orçamento de deslocamento é recarregado pelo tempo e pelo `RunSpeed`
  server-side. Reenviar rotas válidas rapidamente não produz speedhack.
- `ActionStop` e `MoveStop` não podem reposicionar o personagem para longe.
- Ataques físicos e skills ofensivas exigem linha transitável até o alvo. Essa é
  uma proteção adicional do emulador contra ataque através de paredes; não é uma
  regra universal confirmada de todas as sources nativas.

## Combate e skills

- O pacote informa intenção e alvo. Dano, defesa, acerto, evasão, parry, crítico,
  alcance e morte são calculados com o estado autoritativo do servidor.
- O dano e o crítico enviados ou alterados no client não participam da fórmula.
- O servidor rejeita repetição do mesmo `ClientTick` e também aplica o intervalo
  de ataque derivado do personagem. Reenviar `0x39D` não acelera golpes.
- Skills exigem classe compatível, aprendizado, mastery, MP, alvo, alcance e
  cooldown. O cooldown usa `SkillData.Delay` em segundos, como na source.
- Skills físicas, mágicas, em área e PvP usam apenas alvos encontrados e
  validados pelo servidor; uma lista de alvos forjada não é confiável.

## Inventário e economia

- Todo acesso passa por tipo de contêiner e limites server-side. Slots invisíveis,
  face, equipamento e campos estruturais não podem ser vendidos por pacote
  forjado; venda comum aceita somente o inventário.
- Compra e venda revalidam o NPC, a função de merchant, a distância e a loja
  aberta no momento da operação. Repetir um pacote antigo longe do NPC falha.
- Compra, venda, drop e coleta usam persistência antes da confirmação visual.
  Se o save falhar, gold e item voltam ao estado anterior.
- Um drop manual somente é publicado aos clientes depois de a remoção do
  inventário estar persistida. Isso fecha a janela de dupe entre duas contas.
- Trocas entre jogadores requerem persistência atômica das duas contas. Stores
  sem `SaveAccounts` atômico não podem confirmar uma troca multi-conta.
- O estado de cargo, inventário, equipamento e gold pertence ao servidor. O
  catálogo e os efeitos de itens também são carregados dos dados do servidor.
- Cada instância persistida possui UID de 128 bits exclusivo do servidor. O
  PostgreSQL exige UID globalmente único e uma única instância por localização;
  uma tentativa de dupe desfaz a transação completa.
- O UID não vai ao wire. Depois de validar `Index+Eff` enviados pelo client, os
  fluxos mantêm a cópia autoritativa com UID; comparar a struct interna inteira
  com o snapshot de oito bytes do client seria incorreto.

## Testes adversariais

Há testes de framing, handshake, checksum, fila cheia, fase de sessão, replay de
login/ataque, payload anexado, movimento impossível, orçamento de movimento,
wallhack, loja remota, venda de equipamento, rollback de drop/coleta, cooldown e
persistência de trade. Parsers expostos a bytes hostis também têm fuzz tests.

Validação local:

```text
go test ./...
go vet ./...
go build -o tm ./cmd/server
```

## Limites e próximos reforços

Essas regras impedem que o client seja a autoridade, mas não substituem defesa
operacional. Antes de produção pública ainda são recomendados:

- limite por IP/sub-rede no proxy ou firewall, além do limite por conexão;
- proteção contra SYN/connection flood e limites do sistema operacional;
- trilha de auditoria persistente para economia, trade, craft e punições;
- corpus de capturas reais do client para testes de regressão do protocolo;
- alertas por conta/IP e quarentena antes de banimento automático;
- teste in-game prolongado para calibrar o orçamento de movimento sem afetar
  jogadores com latência alta.

Nenhuma proteção depende de esconder offsets ou detectar WPE/Cheat Engine no
computador do jogador. Um anticheat client-side pode complementar telemetria, mas
a segurança principal permanece server-side.

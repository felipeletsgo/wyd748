# Handlers do protocolo 7.48

Auditoria atualizada em 02/08/2026 contra o dispatcher do WYD-Go, W2PP 7.59, Secrets
7.54 e os pacotes observados do client 7.48.

## Pacote periódico

`0x3A0`, 12 bytes, é `_MSG_Ping`. Ele não consulta data/hora. Tanto W2PP
quanto Secrets atualizam o tempo da última recepção na camada de conexão e
retornam sem resposta. No WYD-Go o recebimento pelo `Session.Serve` já comprova
atividade; `onPing` é intencionalmente vazio.

O sincronismo periódico de calendário é `0x334`, 128 bytes, com
`MobName="day"` e texto vazio. A resposta nativa é um `0x101` de 108 bytes com
`"!#11  2"`. O prefixo `!#` faz o client atualizar `m_nYear/m_nDays` sem exibir
mensagem. O comando manual `time` continua mostrando a data/hora do host.

## Implementados nesta auditoria

| Opcode | Tamanho | Função |
|---|---:|---|
| `0x2E4` | 20 | Excluir item do Carry, conferindo slot e índice e salvando antes de confirmar. |
| `0x2E5` | 24 | Separar pilha com `EF_AMOUNT`, novo UID e rollback das duas pilhas. |
| `0x334 day` | 128 | Sincronizar o calendário oculto do client com a resposta nativa `!#11  2`. |
| `0x336` | variável | Reconhecer e descartar score enviado pelo client; o score permanece server-side. |
| `0x368` | 52 | Executar Illusion como `Action2`, preservando efeito, animação e câmera nativos. |
| `0x374` | 20 | Abrir portão/objeto permanente; revalida ID, estado, alcance, visibilidade e `EF_KEYID`. |
| `0x39F` | 20 | Consultar jogador próximo; usa painel porque a janela `0x324` do 7.59 não existe no 7.48. |
| `0x3A0` | 12 | Keepalive explícito, sem resposta, como nas sources. |

### Rotas reconhecidas sem mecânica completa

Os opcodes `0x28E`, `0x28F` e `0xE0E` permanecem registrados para o client não
gerar `sem handler`, mas não simulam uma guerra inexistente. Eles agora passam
pelo mesmo framing estrito dos handlers funcionais (respectivamente 16, 20 e
20 bytes), impedindo caudas arbitrárias. `0x28C` e `0xE12`, que já executam
expulsão e aliança, também tiveram seus layouts 16/20 bytes travados. O
`0x324` foi removido do código: era somente uma constante de um pacote S→C
posterior e não possui layout utilizável no 7.48.

## Protocolos sociais e identidade visual entregues

`0x3D5 InviteGuild` e `0x3E8 Rebuy` já estão implementados com validação de
sessão, limites nativos e persistência transacional. O `Guild`/`GuildLevel`
também é sincronizado nos pacotes de entrada, atualização e materialização do
personagem; `Guilds.txt` é exportado após o commit.

## Pendentes com função nativa confirmada

| Opcode | Complexidade | Estado necessário |
|---|---|---|
| `0x3D5 InviteGuild` | média | Implementado: valida ClientID, alvo online, reino/rank, custo, cooldown e persiste guild + contas atomicamente. |
| `0x3E8 Rebuy` | alta | Implementado: lista de dez entradas no layout 7.48, UID autoritativo, preço recalculado e compra transacional. |
| `0x2CD CapsuleInfo` | alta | Implementado: valida o ID do Spirit's Seal e responde `0xDC3` no layout compacto de 52 bytes confirmado no client 7.48. |
| `0x3CC PutoutSeal` | alta | Implementado: valida UID/origem/nome/slot e materializa a ficha na mesma transação da conta. |
| `0x28E/0x28F Challenge` | alta | O pacote é reconhecido, mas falta disputa de cidades, agenda, taxa, território e vencedor. |
| `0xE0E GuildWar` | alta | Reconhecido sem mutação; depende de guerra de guild, placar, torre, zonas e persistência. |
| `0x39F modos de duelo` | alta | A consulta foi implementada; os modos 1x1/5x5/10x10 comentados na W2PP dependem do sistema de ranking/arena. |

## Crafting que exige confirmação no client 7.48

Esses handlers existem na W2PP, mas não devem ser portados apenas pelo opcode:
parte pertence a clientes posteriores e alguns NPCs/itens não existem no
catálogo 7.48.

| Opcode | Handler W2PP |
|---|---|
| `0x2C4` | Shany |
| `0x2D4` | Extração |
| `0x2E1` | Alquimia |
| `0x2E2` | Odin2 |
| `0x2E6` / `0x2E9` | Dedekinto / Dedekinto2 |

## Não são handlers C→S pendentes

São pacotes de apresentação enviados pelo servidor. Mesmo que algumas headers
7.59 tragam os dois flags, aceitar seu conteúdo do client quebraria a
autoridade do servidor:

- `0x18B UpdateWeather`;
- `0x196 CloseShop`;
- `0x324 RequestInfoPlay` (janela posterior, incompatível com o 7.48 e por isso
  deliberadamente não declarado no protocolo do emulador);
- `0x336 UpdateScore` é reconhecido apenas para descarte; nunca altera o estado;
- `0x3A1 StartTime`;
- `0x3A2 EnvEffect`;
- `0x3A3 SoundEffect`;
- `0x3A4 GuildDisable`;
- `0x3A5 GuildBoard`;
- `0x3A8 WarInfo`;
- `0x3B0 MobCount`;
- `0x3B9 UpdateAffect`.

Também ficam fora do alvo 7.48 a senha numérica `AccountSecure`, transferência
entre servidores e os opcodes customizados de donate/Pix/roupas do fork W2PP.

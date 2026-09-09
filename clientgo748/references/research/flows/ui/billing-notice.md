---
id: billing-notice
title: Notificação de billing 0x194
subsystem: ui
status: CONTRACT
native_sha256: 8AA2F918844BCE3AFE21F1204F69757A443E32EB2F2F616936B1D9BFE215F593
updated: 2026-09-05
---

# Notificação de billing 0x194

## Pergunta

Qual frame o client 7.48 aceita para a notificação de billing e que efeitos
locais devem permanecer quando o aviso chega à cena?

## Fronteira de evidência

- `UTILIZADA` — executável histórico `client748/wyd.exe nativo+patches/WYD.exe`,
  identificado pelo hash nativo registrado nesta ficha.
- `UTILIZADA` — Ghidra `FUN_0055890A @ 0x0055890A` (tamanho) e
  `FUN_0049889A @ 0x0049889A` (branch do opcode e efeitos).
- `UTILIZADA` — source atual `TMScene::OnPacketEvent`, `Basedef.h`,
  `BillingNoticePacket.h` e `ReceivedPacketDispatch.h`.
- `NAO APLICAVEL` — WYD-Go não possui emissor vivo desta notificação; não foi
  criado builder ou regra de billing no servidor.
- `NAO APLICAVEL` — assets: o aviso reutiliza o MessageBox e a linha 132 já
  carregados; nenhum recurso foi alterado.
- `NAO APLICAVEL` — W2PP, Secrets e Micronics são fontes excluídas.

## Fluxo nativo 7.48

### Entrada observável

`FUN_0055890A` aceita `Type=0x194` somente com `Size=0x10` (16 bytes). O frame
é S->C e possui um header de 12 bytes seguido por quatro bytes que o branch do
receptor não interpreta.

### Callers

O receptor da cena é `FUN_0049889A`, chamado pelos dispatchers de cena já
registrados nas fichas de transporte/lifecycle. Este contrato não cria caller
nem emissor histórico novo.

### Função principal

Quando o header chega com `Type=0x194`, `FUN_0049889A` marca o estado global de
billing e chama o container de controles para exibir a mensagem localizada de
índice 132. Não lê, copia ou valida semanticamente os quatro bytes do corpo.

### Callees

O branch escreve o estado `m_bBilling` no ObjectManager e usa o controle de
mensagem da cena para apresentar o aviso. Não há regra de gameplay, persistência
ou resposta de rede criada pelo branch.

### Saídas e erros

- frame canônico: estado billing marcado e MessageBox exibido;
- tamanho diferente: recusado pelo gate antes do callback;
- payload diferente: ainda é opaco e não altera a decisão;
- não existe ACK ou mutação autoritativa associada.

## Estado e lifecycle

### Matriz de transições

| Evento | Precondição | Função | Estado resultante | Side effects | Erro/saída |
| --- | --- | --- | --- | --- | --- |
| receber billing | frame 16B, cena ativa | `FUN_0049889A` | `m_bBilling=1` | MessageBox 132 visível | frame inválido não despacha |
| controle ausente | frame válido | receptor base | estado ainda marcado | nenhum controle criado | sem crash por ausência de UI |
| troca de cena | aviso já consumido | lifecycle da cena | estado segue o ObjectManager | painel antigo destruído | sem buffer retido |

### Vtables, vptrs e receptores

O receptor é a cena base e seu container existente. O gate novo não escolhe
vtable, não cria controle e não altera a ordem de dispatch.

### Ownership

O transporte empresta o buffer somente durante o callback. O gate copia apenas
o header para validar o envelope; o receptor não retém o payload opaco.

### Falha parcial, cleanup e shutdown

O frame não cria estado próprio além do indicador billing já pertencente ao
ObjectManager. Teardown e shutdown seguem os owners normais da cena; não há
rollback ou memória adicional.

### Logout e relogin

Nenhum byte é persistido no frame. Logout/relogin rematerializa a cena e seus
controles pelo lifecycle existente.

## Wire, ABI e recursos

Contrato nativo S->C, little-endian:

| Offset | Tamanho | Campo |
| --- | ---: | --- |
| `0x00` | 12 | `MSG_STANDARD`, `Type=0x194` |
| `0x0C` | 4 | payload opaco, não interpretado pelo branch |
| total | 16 | tamanho exigido por `FUN_0055890A` |

`BillingNoticePacket.h` fixa o corpo como `unsigned char OpaquePayload[4]` para
evitar inventar signedness ou semântica. A source continua usando a mensagem
localizada 132 do recurso já existente.

## Mapeamento atual

`Basedef.h` reexporta o contrato. `ReceivedPacketDispatch::ExpectedSize` agora
valida 16 bytes, `Size` e igualdade entre o opcode externo e `Header.Type`.
`TMScene::OnPacketEvent` rejeita o tamanho divergente antes de marcar billing;
para o frame válido preserva a ação existente e não toca no payload.

O WYD-Go não emite esse aviso. Conhecer o consumidor nativo não autoriza criar
tráfego de billing sem uma feature coordenada.

## Matriz de delta

| Claim | Nativo 7.48 | Source antes | Source atual | Decisão |
| --- | --- | --- | --- | --- |
| opcode | `0x194` | literal nomeado | header de wire | `PARIDADE_NATIVA` |
| tamanho | 16 bytes | apenas limite mínimo de 12 | gate exato de 16 | `PARIDADE_NATIVA` |
| payload | quatro bytes não lidos | não tipado | opaco | preservar |
| efeito | billing + MessageBox 132 | já existente | preservado | manter |
| servidor | nenhum emissor observado | sem builder | sem builder | não implementar |

## Decisões

- Extrair o wire para `internal/wire` e validar o frame antes da cena.
- Manter o payload opaco e a mensagem localizada 132, sem criar regra de
  cobrança ou confirmação.
- Reutilizar o lifecycle existente; esta ficha mede contrato, não teste visual.

## Lacunas

- Executar um frame válido e um frame recusado no `client748/project.exe` com
  MessageBox materializado.
- Confirmar em runtime o indicador billing após troca de cena e relogin.

## Validação

- Pesquisa: `FUN_0055890A` confirma 16 bytes e `FUN_0049889A` confirma estado,
  mensagem e payload não lido.
- Automação: fixture C++ cobre truncamento, excesso, nulo, Type/Size,
  entrega única, payload opaco e preservação do buffer.
- Build: Debug e Release passaram pelo `Build-Client.ps1` com 1761
  checks/asserts em cada configuração; XML, caminhos e `git diff --check`
  também passaram. O candidato Release instalado possui SHA-256
  `D7A487FD78CB838FA1867584A2C2C67476DAA3FF5927700D3D9CE1DBA28C2841`.
- Client real: ainda não executado; nenhuma alegação `CLIENT_TESTED` é feita.

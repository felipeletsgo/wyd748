# Contrato das fichas de evidência

As fichas vivem em `.agents/research/client748/flows/<subsistema>/<fluxo>.md` e
usam o template versionado naquele diretório.

## Front matter obrigatório

- `id`: identificador estável em kebab-case;
- `title`: transição concreta investigada;
- `subsystem`: domínio do mapa de subsistemas;
- `status`: `UNMAPPED`, `LOCATED`, `TRACED`, `CONTRACT` ou `CLIENT_TESTED`;
- `native_sha256`: hash do `WYD.exe` analisado ou `UNRESOLVED`;
- `updated`: data ISO `YYYY-MM-DD`.

## Seções obrigatórias

Toda ficha não-template contém `Pergunta`, `Fronteira de evidência`, `Fluxo
nativo 7.48`, `Estado e lifecycle`, `Wire, ABI e recursos`, `Mapeamento atual`,
`Matriz de delta`, `Decisões`, `Lacunas` e `Validação`.

Em `TRACED`, `Callers` e `Callees` devem estar resolvidos no projeto Ghidra. Em
`CONTRACT`, o ABI/recurso e a decisão precisam ser testáveis. Em
`CLIENT_TESTED`, a seção de validação registra cenário, artefato/hash e resultado
do fluxo real.

## Citações

Cite evidência com endereço/símbolo e arquivo atual, por exemplo:

```text
Native: WYD.exe sha256=... FUN_0055890A @ 0x0055890A
Source: client-source/.../CPSock.cpp :: CPSock::ReadMessage
Server: internal/game/security.go :: inboundPacketSizeAllowed
Asset: client748/UI/FieldScene2.bin :: root 257
```

Não cole pseudocódigo extenso. Registre a interpretação, os branches relevantes
e como reproduzir a consulta.

# Mapa de subsistemas do client 7.48

Este é um inventário de pesquisa, não uma declaração de paridade. `UNMAPPED`
significa que ainda não existe fluxo nativo completo versionado. Atualize a
linha somente quando a ficha correspondente passar pelo validador.

| Subsistema | Transições a mapear | Estado atual | Fichas |
| --- | --- | --- | --- |
| Bootstrap e configuração | WinMain, config, device, managers, primeira cena, falha de boot | `UNMAPPED` | — |
| Transporte e criptografia | connect, seed/queues, framing, decrypt, checksum, disconnect | `CONTRACT` estreito | `transport/socket-disconnect-return-selectserver.md` cobre connect, callback, ownership e disconnect; framing, decrypt e checksum permanecem abertos |
| Gate de tamanho por opcode | header recebido, opcode/tamanho, log/rejeição, caller real | `LOCATED` | `transport/packet-size-gate.md` |
| Seleção de servidor e login | server list, credenciais, resposta, troca de chaves, falhas | `UNMAPPED` | — |
| Seleção/criação de personagem | lista, create/delete, slot, enter world, teardown | `CONTRACT` estreito | `lifecycle/character-logout-selectchar-relogin.md` cobre logout, retorno à seleção e novo enter world; `ui/select-character-exp-threshold.md` rastreia a apresentação de EXP na seleção; criação/delete permanecem abertos |
| Dispatcher de cena/campo | socket -> cena atual -> controles -> handler -> side effects | `UNMAPPED` | — |
| Tick e input do mundo | message loop, FrameMove, mouse/tecla, focus, auto-run | `UNMAPPED` | — |
| Movimento | intenção, rota, predição local, stop, correção server-side | `UNMAPPED` | — |
| Combate e skills | seleção, alcance, intenção, confirmação, efeitos, rejeição | `UNMAPPED` | — |
| Character/Score/equipamento | create/update, score, equip, owner/observer, HUD | `UNMAPPED` | — |
| Affects e efeitos visuais | update, criação visual, tick, remoção, relogin | `UNMAPPED` | — |
| Inventário e cargo | abertura, grid, drag, use, move, rollback, fechamento | `UNMAPPED` | — |
| NPC, shop e auto-trade | criação, interação, buy/sell, merchant, teardown | `UNMAPPED` | — |
| Trade, party e guild | request, confirmação, janelas, sync, cancelamento | `UNMAPPED` | — |
| Mix, quest e sistemas especiais | recipes, slots, intents, resposta, falha | `UNMAPPED` | — |
| UI e lifecycle | construção, binding, root, input, focus, destroy, relogin | `LOCATED` | `ui/control-focus-ime-lifecycle.md` |
| Render e assets | loaders, meshes, texturas, escala, pipeline, device reset | `UNMAPPED` | — |
| Shutdown e reconexão | socket close, cena teardown, globals, retry, exit | `CONTRACT` por fluxos estreitos | `lifecycle/application-close-global-shutdown.md` cobre teardown terminal; `transport/socket-disconnect-return-selectserver.md` cobre disconnect e novo login; `field-scene-rebuild-after-server-move.md` cobre migração e `character-logout-selectchar-relogin.md` cobre logout na mesma sessão; troca explícita de conta permanece aberta |

Os documentos `WYD748_PORT.md`, `WYD748_UI_WINDOWS.md` e comentários atuais da
source são índices legados. Migre conhecimento estável deles para fichas por
transição conforme cada subsistema for rastreado; não marque a linha como
`TRACED` apenas porque há funções citadas nesses documentos.

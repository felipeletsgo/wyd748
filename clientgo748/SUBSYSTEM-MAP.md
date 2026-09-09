# Mapa operacional de subsistemas

O mapa canônico das transições nativas está em
[`references/skills/subsystem-map.md`](references/skills/subsystem-map.md). Este
arquivo é a entrada curta para quem trabalha no client Go.

| Frente | Estado do conhecimento | Próximo gate |
| --- | --- | --- |
| Bootstrap, janela e device vazio | `CLIENT_TESTED` no executável Go | dados, managers e primeira cena ficam em fichas próprias |
| Transporte, framing e criptografia | `CONTRACT` nativo; `AUTOMATED TESTED` Go | validar conexão real e iniciar o contrato de login |
| Login e seleção de personagem | `CONTRACT` nativo; dispatcher `AUTOMATED TESTED` Go | implementar controller/cenas e validar login/logout/relogin real |
| Cenas e dispatcher | manager e dispatcher de login testados | ligar estado → transição de cena sem permitir mutação pela goroutine do socket |
| Tick, input e movimento | `CONTRACT` estreito | completar rota, predição, correção e auto-run |
| Entidades, morte e renascimento | `CONTRACT` estreito | completar visibilidade, death state e rematerialização |
| Portal, NPC, cidade e requests | `CONTRACT` estreito | manter contratos de intenção; regras ficam no servidor |
| Combate e skills | `UNMAPPED` | rastrear seleção, alcance, confirmação, efeitos e rejeição |
| Score, equipamento e HUD | `UNMAPPED` | fechar structs, binding, owner/observer e relogin |
| Affects e efeitos visuais | `UNMAPPED` | fechar criação, tick, remoção e recursos |
| Inventário, cargo, shop e trade | `UNMAPPED` | rastrear grid, drag, rollback, callbacks e fechamento |
| Party, guild e sistemas especiais | `UNMAPPED` | completar sincronização e cancelamento |
| UI, input e lifecycle | `LOCATED` | provar materialização, foco, destroy e relogin |
| Render e assets | contexto WGL implementado; loaders `UNMAPPED` | fechar primeira textura, escala, device reset e fallback |
| Shutdown e reconexão | `CONTRACT` estreito | cobrir troca explícita de conta e falhas parciais |

`UNMAPPED` é uma lacuna de pesquisa, não uma autorização para copiar o
TMProject. Uma nova implementação deve ser classificada como
`PARIDADE_NATIVA`, `MODERNIZACAO_COMPATIVEL` ou `EXTENSAO_COORDENADA` antes do
patch.

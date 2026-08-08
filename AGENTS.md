# WYD-Go

## Política de modelo

Todas as tarefas deste projeto, incluindo skills e subagentes, devem usar o
modelo `gpt-5.6-luna` com esforço de raciocínio `max`. Skills não devem
substituir essa política por outro modelo ou reduzir o esforço.

Emulador Go server-authoritative para o client WYD 7.48. O client envia
intenções e apresenta o estado; nunca é fonte de verdade.

## Contratos

- `World` é o único dono do estado mutável de gameplay.
- `ExtendedScore` v2 é autoritativo. `WireScore` é somente projeção do protocolo.
- Validar server-side pacote, fase, alvo, distância, terreno, cooldown, item,
  slot, preço, requisito e saldo.
- Operações anti-dupe seguem: validar → mutar cópia → persistir → publicar.
  Restaurar todo o snapshot se a persistência falhar.
- PostgreSQL é a persistência autoritativa; JSON é adaptador explícito de
  desenvolvimento.
- Arquivos sob `data/` são a fonte autoritativa de conteúdo. `itemlist.csv`
  define itens e efeitos estáticos; `Itemname.csv` substitui somente os nomes;
  `ItemEffect.h` define a relação ID↔EF_* dos efeitos persistidos; e
  `SkillData.csv` define todos os parâmetros das skills. Não duplicar valores
  desses arquivos em mapas, tabelas ou correções pós-load no código.
- Item materializado possui UID server-side e esse UID nunca vai ao wire.
- Usar o índice espacial para consultas locais; não varrer todos os mobs em
  ticks frequentes.
- `Merchant != 0` nunca é hostil ou atacável.
- Não usar `CreateMob` para atualizar jogador já visível; usar pacotes
  incrementais para score, affects, equipamento e movimento.
- Inventário tem 64 slots estruturais/63 visíveis; cargo 128/120.
- Texto enviado ao client é inglês; logs e comentários podem ser português.

## Organização

- `model`: domínio puro.
- `wire`: protocolo, criptografia e builders.
- `net`: conexão e sessão, sem regra de jogo.
- `store`: persistência.
- `data`: loaders e validação de arquivos.
- `game`: coordenação, fragmentada por feature.

Handlers devem decodificar e rotear. Coloque regras extensas no arquivo da
feature, não em `world.go` ou `handlers.go`.

## Pesquisa

Para mecânica nativa, use a skill `wyd-go-feature`. Consulte apenas as
referências relacionadas à tarefa; não leia toda a documentação por padrão.
Prioridade: implementação atual → W2PP → Secrets 7.54 → Micronics/client 7.48.
Porte algoritmos, nunca structs, offsets ou endereços de outra versão.

## Validação

Execute na raiz `wyd-go`:

```powershell
go test ./...
go vet ./...
go build -o tmp/tm-check.exe ./cmd/server
git diff --check
```

Adicione teste que reproduza a falha real. Remova apenas os artefatos criados
pela validação. Preserve mudanças não relacionadas do worktree.

Documentação detalhada é consultada sob demanda em `DOCS/`; estado entregue em
`DOCS/IMPLEMENTATION_STATUS.md` e roadmap em `DOCS/plan.md`.

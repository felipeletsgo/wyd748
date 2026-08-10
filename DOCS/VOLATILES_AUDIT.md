# Auditoria de Volatiles — 7.48/7.54

Data da auditoria: 2026-08-02.

Este documento registra o cruzamento entre o catálogo autoritativo do
servidor, `data/volatiles.json`, o `_MSG_UseItem.cpp` do W2PP, os trechos
equivalentes do Micronics e a documentação pública de desenvolvimento. A
regra do projeto é: `itemlist.csv` define o código `EF_VOLATILE`; o JSON define
o contrato e o código Go executa a mutação server-side.

## Resultado do cruzamento

- `data/itemlist.csv`: **355 itens** com `EF_VOLATILE`, distribuídos em
  **124 códigos distintos**.
- Todos os 355 itens resolvem para uma regra explícita; nenhum item real cai em
  `generic`.
- O item 3443 (`Spirit's Seal`) usa `celestial_capsule`: encapsula a forma
  Celestial/SubCelestial e grava o snapshot/UID no agregado da conta. O item
  3455 permanece `no_direct_use`, pois o `_MSG_PutoutSeal.cpp` nativo só aceita
  o índice 3443. O fluxo e os limites estão em `DOCS/CELESTIAL_CAPSULE.md`.
- O código `9` agora pertence à Adamantita 578. Os códigos antes livres
  `180–183` identificam Diamond, Emerald, Coral e Garnet.
- Os testes de dados agora protegem a contagem 355/124 e a ausência de regra
  genérica.

O `ItemList.csv` do W2PP contém 577 itens com volatile, enquanto o catálogo
7.48 deste servidor contém 355. Portanto, copiar o W2PP por ID produziria
funções para itens inexistentes e aceitaria itens de releases posteriores. Há
também remapeamentos deliberados no catálogo local: por exemplo Love/Sefirah/
Courage usam `67` no cliente 7.48, e as moedas Wyden usam `184` no contrato
local. O cruzamento foi feito primeiro por item existente e depois por código;
não por uma lista numérica cega.

O tutorial público confirma a semântica básica que usamos: volatile é a função
executada quando o item é utilizado e o número do `EF_VOLATILE` precisa apontar
para o handler correspondente. A discussão pública do Warrior Seal também
confirma que o defeito clássico é a divergência entre struct/handler e o
número no `itemlist.csv`.

## Famílias confirmadas e estado atual

| Família | Códigos/itens | Estado no WYD-Go |
| --- | --- | --- |
| Poções, moedas e recall | `1–3`, `11–13`, `18`, `185` | Completo; valores vêm do itemlist/Price e consumo é persist-before-confirm. |
| Ori/Lac e montarias | `4`, `5`, `15`, `16`, `90–94`, `196`, `3453–3454`, caixas `4003–4007` | Completo para o catálogo 7.48; Vol. 90 restaura +1 LP, 91/92 catalisam nível 100/120, 93 restaura 1–3 de longevidade e 94 avança estágio. Ovo usa `EF_SANC` + `EF_INCUDELAY`, exige slot 14 e conta 6–8h somente online/equipado. O acelerador (`196`) transforma diretamente em cria. Todas as mutações têm rollback. |
| Progressão | `6`, `7`, `8` | Completo conforme o contrato do projeto: Magical Pill única (+9), Fairy Dust em um marco e Eye Wax com os valores customizados já solicitados. |
| Adamantita e Gemas | `9`, `180–183`; itens `578`, `3386–3389` | Adamantita transforma equipamento compatível pelo `Extra`; as quatro Gemas aplicam variantes +10..+15 e aceitam armas Ancient abaixo de +10, preservando UID, adicionais e refinação. Bônus de drop, perfuração, EXP e absorção são derivados server-side. |
| Buffs e alimentos | `10`, `30`, `52–67`, `198`, `3313`, `3361–3366`, `3378`, `1739`, `4145` | Completo e configurável. A resolução de Love/Sefirah usa os `SkillData` autoritativos; Courage aplica o bônus de hit somente em PvE; EXP acumula até 24h; Bigger/Health usam affect 35 e não acumulam cópias paralelas. |
| Fogos e transformação | `19`, `70–77`, `89` | Completo; fogos comuns usam Motion 100, Premium preserva o desenho 10x10 do client e o estado é persistido antes de publicar o visual. |
| Livros Sephira | `32–36` | Completo; bits 25–29 de `LearnedSkill`, sem inventar `SecondaryLearnedSkill`. |
| Contratos | `41–43`, `46–48` | Completo para o modo temporário atual: um summon ativo, troca atômica do contrato, segue/combate pelo dono, não entra na party e é removido no logout/morte. |
| Cube/Big Cube/Water | `21–28`, `30`, `51`, `54`, `131–140`, `161–172`, `171–172`, overrides `777–785`, `3171–3190` | Completo no modelo configurável. Big Cube usa a pergunta O/X somente para membros da instância e envia a todos eles a decisão; a saída padrão é Armia `(2100,2100)`. |
| Nightmare/Hell Gate/Hidden Ruins | `173–177`, `197` e overrides `3324–3329`, `3390–3392`, `3439–3440`, `3909–3910`, `3974` | Completo como instâncias/acessos server-side, com limite, expiração e validação de área. |
| Tintura e Repliction | `186`, `190`, `3407–3417`, `4016–4025` | Completo; preserva o slot de refino/tintura, substitui os dois adicionais e rejeita Cythera/alvo inválido. `4025` mantém compatibilidade do catálogo local, embora o W2PP não tenha um ramo Premium-E distinto. |
| Game Room e contadores | `178`, `184`, `188–189`, `199`, `3393–3396`, `4111–4116` | Completo com `SpecialCoins`/charstate transacional. Warrior Seal concede fama, sem misturá-la com CP/EXP. |
| Quest boxes | `191`, `4117–4121` | Completo; snapshot integral de EXP/level/score/HP/MP/inventário e rollback em falha de persistência. |
| Molar e caça | `194`, `195` | Completo; Molar exige Mortal interno `200..255`, uma vez, e os Hunting Scrolls validam `WarpID` 1..10, terreno e posição livre. |
| Fama, CP e Celestial | `187`, `203`, `206`, `211–214` | `187`, `203`, `211–214` e o 3443 do código `206` estão implementados conforme o modelo atual. O 3455 é no-direct-use por compatibilidade com o W2PP. |

## Divergências intencionais, não regressões

1. **Eye Wax (`8`)**: o valor customizado do projeto foi preservado por pedido
   explícito; não deve ser substituído pelo valor da source sem nova decisão.
2. **Buffs**: duração, teto de 24 horas e percentuais ficam no JSON. A source
   W2PP fornece a família/affect; o balanceamento é uma decisão do servidor,
   não uma leitura confiável do cliente.
3. **Código 67**: no cliente 7.48 Love/Sefirah/Courage foram compactados para um
   código aceito pela interface. A regra resolve os efeitos reais pelo
   `SkillData`, em vez de tratar `41/43/44/45` como tipos de affect.
4. **Cristais (`187`)**: o W2PP mais novo verifica `ClassMaster == CELESTIAL`,
   mas também grava o progresso no bloco Arch e exige o piso 355. Neste branch
   a progressão Celestial tem teto interno 199; por isso a porta é Arch e o
   progresso é salvo em `ArchCrystals`. É uma decisão de compatibilidade da
   versão, documentada no código.
5. **Teleporte de caça**: além das dez coordenadas nativas, o servidor valida
   mapa/terreno/ocupação para não aceitar coordenadas adulteradas pelo pacote.
6. **Código `0` e itens indexados**: `no_direct_use` é correto para itens que a
   source usa em NPC/comando. Não transformar todo item `EF_VOLATILE=0` em um
   consumível abre uma superfície de exploit.

## Itens tratados por índice, não por EF_VOLATILE

O W2PP/Micronics também possui dispatches por `sIndex` para objetos que não
trazem `EF_VOLATILE` no `itemlist.csv` 7.48 local. Exemplos confirmados:

- `3386–3389` (gems), `3445–3448` (separação/agrupamento), `3200–3210` e
  `3221` (loteria), `3318` (Roast Chicken Package), `4104` (Lucky Pocket);
- barras/objetos de releases diferentes (`3027–3030`, `4905`, `5359–5360`)
  que não existem no catálogo autoritativo deste branch.

Eles não foram falsamente adicionados a `volatiles.json`: o loader rejeita uma
regra por item que não tenha `EF_VOLATILE`, mantendo `itemlist.csv` como fonte
de verdade. Caso esses itens precisem ser clicáveis no 7.48, a próxima etapa é
criar uma tabela separada de `indexed_use_items`, com o mesmo fluxo
validate → mutate → persist → publish; alterar o `EF_VOLATILE` sem confirmar a
versão mudaria a semântica do catálogo e reabriria o risco de exploits.

## Fontes e método

- Source principal: `SERVER W2PP/SOURCE SERVER/Code/TMSrv/_MSG_UseItem.cpp`.
- Comparação 7.54: `SOURCE(secrets) para referência/Source/TMSrv/TMSrv_PacketProtocolV754/UseItem.cpp`.
- Comparação Micronics: `Sourcer Star Micronics/Volatile.cpp`, `pVolatile.cpp`,
  `Quests_Especial.cpp` e `pVariados.cpp`.
- Catálogo: `data/itemlist.csv`, `data/SkillData.csv` e
  `data/volatiles.json`.
- Referência pública de `EF_VOLATILE` e ligação item→handler:
  <https://www.webcheats.com.br/topic/1878624-tutorialvolatile-completo-explicado/>.
- Relato público do Warrior Seal e da necessidade de alinhar itemlist/handler:
  <https://www.webcheats.com.br/topic/2264166-bugs/>.
- Implementação aberta de referência do W2PP:
  <https://github.com/Jean1dev/w2pp-OpenWYD/blob/main/Source/Code/TMSrv/_MSG_UseItem.cpp>.

## Pendência real

Não há pendência de servidor na família `206`: Spirit Seal/Extraction possui
cápsula persistente, logout ordenado e reentrada transacional. Permanece apenas
a validação visual no client 7.48. O item 3455 não é consumido diretamente,
porque o handler nativo `_MSG_PutoutSeal.cpp` só aceita o índice 3443. Nenhum
outro item real do `itemlist.csv` está no handler genérico.

Validação executada após a auditoria:

```text
go test ./internal/game ./internal/data
```

Os testes cobrem o ciclo de valor crítico 3 até a poeira final, limites de 6 e
8 horas, pausa em inventário/offline, rejeição durante a espera e falhas de
persistência. No acelerador, ovo e item retornam exatamente ao snapshot anterior
e nenhum consumo é confirmado antes do save.

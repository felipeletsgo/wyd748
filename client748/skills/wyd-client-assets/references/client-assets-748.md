# Assets visuais do client WYD 7.48

## Índice

- Escopo e evidência
- Trajes e partes do corpo
- Formato das meshes
- Tabelas de textura
- Montarias
- Importação segura
- Diagnóstico por sintoma
- Validação

## Escopo e evidência

Aplicar estes fatos ao client 7.48 deste repositório. Revalidar quando o
executável ou os clients-fonte forem substituídos.

Fontes locais usadas nesta investigação:

```text
client748/WYD.exe
CLIENTS/WYD
CLIENTS/wyd-test
client748/Costumes-KR.json
client748/Mounts-KR.json
```

Na auditoria de 13/08/2026, `CLIENTS/wyd-test` era subconjunto exato dos assets
visuais de `CLIENTS/WYD`; 137 arquivos existiam somente no client KR atual. O
Test não fornecia nenhuma dependência ausente do atual. Essa é uma conclusão do
snapshot, não uma regra universal: repetir o inventário se as pastas mudarem.

## Trajes e partes do corpo

O 7.48 equipa traje em `Equip[13]` (`Pos=8192`). O slot moderno de outra versão
não deve ser transplantado.

O renderer trabalha com seis partes. No fluxo nativo
`TMSkinMesh::RestoreDeviceObjects`, a iteração começa em `i=0` e o primeiro
arquivo de traje formado é `ch0101<tipo>.msh`; as seguintes usam
`ch0102<tipo>.msh` até `ch0106<tipo>.msh`.

```text
parte 0 = primeira geometria real do skin (ch0101<tipo>.msh)
partes 1..5 = geometrias seguintes (ch0102..ch0106)
```

Não se deve tratar universalmente a parte 0 como rosto nem retorná-la sempre ao
renderer nativo. Isso omite geometria real e deixa o torso/equipamento comum
visível em trajes como 2023 Autumn, 2023 Halloween e White Christmas. O
fallback nativo é permitido somente quando a entrada correspondente estiver
explicitamente vazia no manifesto; partes vazias posteriores são omitidas sem
deslocar as demais.

O número de `m_nCosType` sozinho também não identifica uma instância importada:
personagens comuns e NPCs reutilizam valores presentes na tabela KR. Em
13/08/2026, habilitar a parte 0 apenas por esse número fez personagens sem
traje e faces de NPC desaparecerem. A solução reproduzível usa o bit `0x4000`
de `m_nCosType` como namespace exclusivo dos trajes importados, mascara o bit
antes do lookup e faz fallback integral para toda skin nativa sem a marca.
Entradas `part0` vazias preservam a face nativa; entradas reais carregam
`ch0101<tipo>.msh`.

As montarias KR também precisam integrar todos os gates de corrida. No 7.48,
`SetRunMode`, a atualização do botão e `TMHuman::SetSpeed` repetem uma allowlist
nativa de tipos 31/40/20/39. Para tipos importados, ampliar somente quando
`m_cMount == 1` e a assinatura completa `type + scale + 3*(mesh,skin)` existir
na tabela `.mountkr`; liberar apenas por tipo pode atingir outra entidade.

Classificação corporal confirmada no catálogo importado:

```text
EF_CLASS=5  -> TK/BM
EF_CLASS=10 -> FM/HT
EF_CLASS=15 -> renderer realmente dinâmico/unissex
```

Não converter automaticamente 5 ou 10 para 15. Um traje feminino equipado em
corpo masculino pode carregar a geometria e ainda deformar o modelo.

O selector do executável KR fornece a relação exata:

```text
item -> renderer type -> skin/body
```

Extrair essa relação; não inferir pelo nome do item.

O caminho nativo do 7.48 possui uma barreira anterior ao selector: somente
itens `4151..4200` no `Equip[13]` seguem para a preparação do traje. Portanto,
quando itens baixos funcionam e itens modernos ficam com o corpo original, a
primeira inspeção deve ser o gate de admissão, não o renderer. A correção segura
é consultar a lista exata do manifesto antes da barreira e preservar o fluxo
original para qualquer item desconhecido. Aumentar apenas o limite superior
admitiria itens arbitrários no slot e não é aceitável.

## Formato das meshes

As meshes importadas observadas usam formatos já presentes no client 7.48:

| formato | stride observado |
| --- | --- |
| `0x1116` | `0x24` |
| `0x1118` | `0x28` |
| `0x111A` | `0x2C` |
| `0x111C` | `0x30` |

Isso comprova compatibilidade estrutural desses formatos, mas não comprova:

- skeleton correto;
- body/skin correto;
- quantidade correta de partes;
- textura registrada;
- animação compatível.

Verificar cada uma dessas fronteiras separadamente.

## Tabelas de textura

O client 7.48 usa 2.048 registros legados de 264 bytes, equivalentes a:

```text
char filename[255]  @0
char alpha          @255
DWORD lastUsed      @256
DWORD showTime      @260
```

O client KR atual usa registros de 528 bytes. Nele existe um segundo pathname
iniciando em `@255`, e o modo alpha real está em `@510`.

Portanto:

```text
KR @510 -> 7.48 @255
```

Nunca copiar o registro inteiro nem o byte KR `@255`. Na falha descoberta, esse
byte era o caractere `m` (109), início de `mesh\...`, gravado como alpha. Texturas
que exigiam modo `C` ficavam invisíveis e deixavam apenas a face; montarias sem
registro apareciam brancas.

Modos confirmados no catálogo atual:

```text
A = 65
C = 67
N = 78
a = aceitar somente quando a fonte comprovar
```

Copiar o `.wys` não basta. Todo pathname realmente formado pelo renderer precisa
existir em `MeshTextureList.bin`. O nome runtime pode ser um segundo skin ou uma
variante compartilhada que não aparece como nome principal da entrada visual.

O significado completo do material `C` ainda exige validação visual no renderer
7.48. A tentativa de convertê-lo globalmente para `A` tornou trajes invisíveis
e foi revertida. Preservar o byte comprovado em `@510` até existir um adapter
de render específico, validado por modelo.

## Culling de meshes esqueletais modernas

Torso ausente, superfícies translúcidas e olhos/interior visíveis podem ser
descarte de faces, mesmo quando `.msh`, `.wys` e o modo de material estão
corretos. O renderer 7.48 escolhe `D3DCULL_CW` ou `D3DCULL_CCW` para
`TMSkinMesh.m_nMeshType == 1`. O caminho skinned do client KR/W2PP atual chama
`D3DCULL_NONE` antes do draw e restaura `CCW` depois.

Nunca portar isso globalmente: meshes antigas dependem do comportamento do
7.48. A adaptação segura precisa identificar a instância importada:

- traje: `m_nCosType` deve resolver uma entrada não vazia da tabela KR;
- montaria: comparar a assinatura materializada completa (type, scale e os
  três pares mesh/skin) com o manifesto antes de marcar o `TMSkinMesh`;
- somente a instância confirmada recebe `D3DCULL_NONE`;
- o caminho nativo e a restauração do estado permanecem intactos.

Não usar `TMHuman.m_nSkinMeshType` como booleano de sexo e não sobrescrevê-lo:
ele contém a variante real do corpo/skeleton. Não usar `m_cRotate[1]` como
marcador persistente de traje, pois `SetWeaponType` o recalcula; uma sentinela
nesse byte só é aceitável no `TMSkinMesh` separado da montaria.

Regressões obrigatórias incluem texturas de modos diferentes. Exemplos atuais:

```text
mesh\WhitePolice.wys  -> A
mesh\ch0201108.wys    -> C
mesh\ch0201109.wys    -> C
mesh\ch0202104.wys    -> C
mesh\KK010101.wys     -> A
mesh\dr010149.wys     -> C
```

## Montarias

O 7.48 equipa montaria em `Equip[14]`. A implementação atual materializa as
aparências KR sobre o contrato server-side da Shire; isso não importa o sistema
moderno de costume/timer de montaria.

Resolver a aparência pelo `SetMountCostume` do client KR e confirmar a formação
do nome no caminho equivalente a `TMSkinMesh::RestoreDeviceObjects`, incluindo
as exceções `God2Exception`. Prefixos comprovados no catálogo atual:

```text
20 dr01   29 be01   31 hs01   39 dr02   40 bd02
48 CP01   49 KK01   50 mc01   51 ct01   59 mo02
```

Não associe item moderno a um case por sequência ou semelhança visual. Extraia
o tuple completo do switch nativo nos dois executáveis de referência. Em
13/08/2026, as aproximações dos nove últimos itens produziram cavaleiro
invertido, meshes no chão, dragão no lugar de pogball e materiais incorretos.
O catálogo aceito usa somente os cases `11..56` e `62` confirmados em ambos.

Uma montaria pode depender de:

- `.bon` skeleton;
- `.ani` animações;
- `.msh` geometria;
- `.wys` textura;
- `BoneAni4.txt`;
- `AniSound4.txt`;
- bloco do tipo em `ValidIndex.bin`;
- registro correto no `MeshTextureList.bin`.

O 7.48 possui dois caminhos visuais distintos. A materialização completa lê o
item do `STRUCT_MOB`; o `UpdateEquip 0x36B` passa pelos 16 `sEquip` compactos e
reconstrói o objeto imediatamente. No caminho incremental nativo, `Equip[14]` é
mascarado com `0x0FFF`. Por isso um item importado `4190+` pode funcionar no
login/teleporte e desaparecer ou demorar ao equipar: o selector enxerga apenas
o valor baixo `94+`. Um patch de montaria só está completo quando resolve a
mesma tabela visual nos dois caminhos, antes de `InitObject()/UpdateMount()`.

O renderer de montaria possui duas transformações independentes:

```text
matriz da TMSkinMesh -> orientação do corpo da montaria
offset do TMHuman    -> posição/orientação aparente do cavaleiro
```

Montaria vertical exige comparar a matriz com o client fonte. Montaria correta
com o jogador vertical/deslocado exige comparar o ramo de assento. Não instalar
um hook global de matriz antes de validar cada família no 7.48: a tentativa
global feita em 13/08/2026 quebrou inclusive montarias antes funcionais e foi
revertida para o build `78B27091…2005`.

Mesh visível e branca aponta primeiro para WYS/tabela alpha. Item equipado sem
troca de aparência aponta primeiro para selector/tipo/skin ou dependência de
skeleton/índice.

## Importação segura

1. Extrair selector e renderer do executável fonte.
2. Inventariar todas as dependências nos dois clients.
3. Criar/atualizar o manifesto com `available=false` para qualquer conjunto
   incompleto.
4. Copiar somente assets comprovados, sem sobrescrever arquivos 7.48 existentes
   silenciosamente.
5. Traduzir a tabela de textura campo a campo.
6. Materializar item e nome nos dados autoritativos.
7. Distribuir lojas em blocos de no máximo 27.
8. Patchar o executável somente com script guardado por SHA e byte assertions.
9. Reconstruir desde backups conhecidos e registrar hashes intermediários.

Não associar item sem case visual a uma mesh “parecida”. Não considerar um
arquivo ausente resolvido apenas porque outra versão possui nome semelhante.

## Diagnóstico por sintoma

| sintoma | verificar primeiro |
| --- | --- |
| item equipa, aparência não muda | item->type/skin, hook e slot 13/14 |
| somente face aparece | alpha `C` gravado incorretamente ou partes deslocadas |
| torso esticado | body class/skin incompatível ou part mapping errado |
| mãos/pés somem | parts 1..5 incompletas/deslocadas |
| mesh aparece branca | WYS ausente ou não registrado em MeshTextureList |
| IDs antigos funcionam e IDs modernos não mudam o corpo | gate nativo anterior ao selector ainda limita `4151..4200` |
| montaria fica invisível | skeleton/ValidIndex/mesh/type e tabela de textura |
| montaria aparece apenas segundos/minutos depois | hook ausente no `UpdateEquip 0x36B`; somente a materialização completa foi coberta |
| montaria inteira fica em pé | matriz de rotação do skeleton moderno tratada como legacy |
| montaria correta, rider em pé/deslocado | ramo de assento do tipo/mesh não portado |
| olhos/interior visíveis através da mesh | material `C` moderno usado literalmente ou estado de culling/alpha incompatível |
| funciona no owner, não observer | `UpdateEquip`, materialização e assets do observer |
| funciona até relogar | item/slot ou cadeia de patch não persistida/reproduzida |

## Validação

Validação estática mínima:

- manifesto sem IDs/tipos duplicados;
- todo asset disponível existe;
- todo WYS usado está registrado com alpha correto;
- renderer binário coincide com o manifesto;
- partes 0..5 preservam a anatomia esperada;
- ItemList e lojas cobrem exatamente os itens disponíveis;
- hashes de cada elo da cadeia coincidem;
- `-VerifyOnly` não escreve.

Validação in-game mínima:

```text
owner + observer
TK/BM + FM/HT + renderer dinâmico
equipar + desequipar
andar + atacar
transformação BeastMaster
relogin
troca de mapa/visibilidade
```

Usar estados distintos: `STATICALLY VERIFIED` não significa `CLIENT-TESTED`.

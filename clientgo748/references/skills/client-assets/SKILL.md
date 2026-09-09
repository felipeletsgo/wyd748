---
name: wyd-client-assets
description: Investigar, importar, corrigir e validar assets visuais no client WYD 7.48, incluindo meshes MSH, texturas WYS, MeshTextureList, skeletons BON, animações ANI, costumes, montarias e manifests. Toda lógica ativa deve ser adaptada em client-source e todo dado visual nos assets; binários e scripts históricos servem apenas para estudo. Usar quando uma aparência não carregar, ficar branca/invisível, usar corpo errado, perder mãos/pés/face, divergir entre owner e observer, ou ao portar conteúdo de clients KR.
---

# WYD client assets

## Autoridade e rastreabilidade

Resolver conflitos nesta ordem: consumidor/source atual → comportamento e
Ghidra 7.48 → manifests/testes atuais → bytes dos clients-fonte → W2PP/KR →
referências mais novas. Nome semelhante, índice próximo ou screenshot não prova
compatibilidade.

Para cada asset importado ou corrigido, registrar:

```text
feature/item | versão e arquivo de origem | SHA-256 origem | transformação
destino ativo | selector/loader consumidor | dependências | testes | estado
```

Uma cadeia completa inclui selector, índice, mesh, todas as texturas formadas
em runtime, skeleton/animação quando aplicável e owner/observer. Hash sem
consumer conhecido prova identidade do arquivo, não que o client o carregará.

## Fluxo obrigatório

1. Ler `AGENTS.md` da raiz e `client748/AGENTS.md`.
2. Classificar o defeito antes de editar:
   - item não selecionado pelo renderer;
   - tipo/skin corporal incorreto;
   - partes MSH deslocadas ou ausentes;
   - textura WYS ausente/não registrada/alpha incorreto;
   - skeleton, animação ou `ValidIndex` ausente;
   - selector/renderer da source ainda não adaptado ao contrato 7.48.
3. Inventariar os dois clients fornecidos (`CLIENTS/WYD` e
   `CLIENTS/wyd-test`) e provar qual contém cada dependência. Não presumir que o
   client Test completa o atual.
4. Confirmar o caminho vivo no executável alvo 7.48. Usar W2PP/KR somente para
   semântica de nomes e algoritmos; nunca transplantar endereço, stack frame ou
   layout.
5. Atualizar primeiro o manifesto correspondente. Marcar um item como
   disponível somente quando selector, mesh, textura e dependências auxiliares
   estiverem comprovados.
6. Escolher a fronteira ativa antes de implementar:
   - comportamento, selector ou renderer: código C++ 7.48 em `client-source/`,
     comprovado no Ghidra e sem offsets absolutos do binário histórico;
   - meshes, texturas, tabelas e catálogos: assets do `client748/`, com
     transformação reproduzível e formato comprovado pelo loader 7.48;
   - `client748/wyd.exe nativo+patches/`: evidência histórica read-only. Nunca
     executar, editar, restaurar ou usar seus scripts como etapa de validação.
7. Compilar a source com `client-source/tmproject/Build-Client.ps1`, que deve
   instalar e conferir automaticamente o resultado como `project.exe`.
   Alterações em assets devem ser consumidas por esse candidato sem reconstruir,
   aplicar ou verificar qualquer cadeia binária legada.
8. Executar os testes estáticos e registrar separadamente a validação in-game.

## Condições de parada

Não importar nem habilitar quando faltar qualquer elo da cadeia de dependências,
quando a versão/formato do registro estiver incerta, quando duas origens forem
plausíveis sem discriminador, ou quando o selector/loader 7.48 ainda não tiver
sido identificado. Marcar como `INCOMPLETO` e listar a prova ausente; não usar
fallback visual aproximado para esconder mesh, textura, BON ou ANI faltante.

Se a correção depender de comportamento C++, retornar ao gate Ghidra de
`wyd-go-feature` antes da edição. Se depender de layout/controle da UI, esta
skill deixa de ser suficiente e `client-ui-748.md` passa a governar o fluxo.

## Fonte de verdade

- Seleção/renderização: executável e comportamento do client 7.48.
- O `WYD.exe` histórico é somente referência Ghidra; `project.exe` é o único
  candidato ativo. Hashes e offsets históricos nunca viram mecanismo de build.
- Catálogos em `wyd.exe nativo+patches/` documentam a importação antiga e podem
  orientar estudo, mas dados ativos devem existir nos assets ou em `data/`.
- Itens/efeitos: `data/itemlist.csv`; nomes: `data/Itemname.csv`.
- Assets: arquivos existentes nos clients fornecidos; ausência real não pode
  ser preenchida por aproximação de nome.
- Contrato detalhado: ler
  `references/client-assets-748.md` antes de alterar renderer, tabela de
  textura, costume ou montaria.

## Regras de implementação

- Manter `Equip[13]` para traje e `Equip[14]` para montaria no 7.48.
- Para montarias importadas, cobrir tanto a materialização completa quanto o
  refresh incremental de `UpdateEquip 0x36B`; validar equipar e desequipar sem
  relog, teleporte ou espera.
- Preservar a face/cabeça quando o traje não fornece parte 0.
- Não tratar `.msh` compatível como prova de skeleton ou corpo compatível.
- Registrar todo pathname `.wys` formado em runtime em `MeshTextureList.bin`;
  ele pode ser diferente do nome principal extraído do manifesto.
- Preservar o material obtido no registro KR `@510`; não traduzir `C` para
  outro modo sem validação visual no renderer 7.48.
- Tratar rotação da mesh e offset/orientação do rider como contratos separados;
  corrigir um não implica corrigir o outro.
- Traduzir layouts de tabela por campo sem copiar registros inteiros entre
  versões.
- Não duplicar em code cave uma tabela que o manifesto ou o client já fornece.
- Não habilitar item incompleto apenas para aumentar a contagem da loja.
- Limitar cada loja visual a 27 itens.

## Papéis de agents

- **Pesquisa read-only:** comparar inventários, decompilar selector/renderer e
  entregar evidência `arquivo:offset/função`, sem editar.
- **Exploração:** localizar manifests, scripts, hashes, tabelas e testes vivos.
- **Implementação:** possuir somente source/manifests/assets atribuídos; não
  reverter mudanças concorrentes.
- **Integração:** revisar o diff completo, compilar a source, conferir hashes
  e separar `STATICALLY VERIFIED` de `CLIENT-TESTED`.

Não delegar ao worker a escolha especulativa de mesh, skin, alpha ou ABI.

## Validação

Executar `client-source/tmproject/Test-Client748Assets.ps1` para os formatos
ativos e compilar com `client-source/tmproject/Build-Client.ps1`; o build deve
instalar, conferir e informar o novo hash de `client748/project.exe`.
Nenhum script sob `wyd.exe nativo+patches/` constitui gate válido. Depois
executar os gates do repositório. No client, testar owner e observer,
corpos masculino/feminino, equipar/desequipar, movimento, ataque, transformação
BM e relogin. Somente essa última etapa permite declarar `CLIENT-TESTED`.

Classificar por item/família, não pelo pacote inteiro:

- `IMPORTED`: bytes/manifests alterados, ainda sem prova suficiente;
- `STATICALLY VERIFIED`: formatos, dependências, hashes e testes estáticos
  passaram;
- `AUTOMATED TESTED`: teste automatizado relevante exerceu o consumidor;
- `CLIENT-TESTED`: owner e observer passaram pelo fluxo real afetado.

Uma montaria validada não prova todas as montarias; um screenshot parado não
prova animação, refresh incremental, desequipar, transformação ou relogin.

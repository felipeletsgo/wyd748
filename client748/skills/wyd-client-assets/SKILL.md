---
name: wyd-client-assets
description: Investigar, importar, corrigir e validar assets visuais no client WYD 7.48, incluindo meshes MSH, texturas WYS, MeshTextureList, skeletons BON, animações ANI, costumes, montarias, manifests e patches reproduzíveis do WYD.exe. Usar quando uma aparência não carregar, ficar branca/invisível, usar corpo errado, perder mãos/pés/face, divergir entre owner e observer, ou ao portar conteúdo de clients KR.
---

# WYD client assets

## Fluxo obrigatório

1. Ler `AGENTS.md` da raiz e `client748/AGENTS.md`.
2. Classificar o defeito antes de editar:
   - item não selecionado pelo renderer;
   - tipo/skin corporal incorreto;
   - partes MSH deslocadas ou ausentes;
   - textura WYS ausente/não registrada/alpha incorreto;
   - skeleton, animação ou `ValidIndex` ausente;
   - patch aplicado sobre SHA/ABI incompatível.
3. Inventariar os dois clients fornecidos (`CLIENTS/WYD` e
   `CLIENTS/wyd-test`) e provar qual contém cada dependência. Não presumir que o
   client Test completa o atual.
4. Confirmar o caminho vivo no executável alvo 7.48. Usar W2PP/KR somente para
   semântica de nomes e algoritmos; nunca transplantar endereço, stack frame ou
   layout.
5. Atualizar primeiro o manifesto correspondente. Marcar um item como
   disponível somente quando selector, mesh, textura e dependências auxiliares
   estiverem comprovados.
6. Implementar por script PowerShell reproduzível, com SHA de entrada/saída,
   assertions dos bytes originais, backup e modo `-VerifyOnly`.
7. Reconstruir `WYD.exe`, `ItemList.bin` e `MeshTextureList.bin` quando o
   contrato afetar esses três artefatos. Não editar binário manualmente.
8. Executar os testes estáticos e registrar separadamente a validação in-game.

## Fonte de verdade

- Seleção/renderização: executável e comportamento do client 7.48.
- Catálogo visual importado: `Costumes-KR.json` e `Mounts-KR.json`.
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
- **Implementação:** possuir somente manifests/scripts/assets atribuídos; não
  reverter mudanças concorrentes.
- **Integração:** revisar o diff completo, reconstruir a cadeia, conferir hashes
  e separar `STATICALLY VERIFIED` de `CLIENT-TESTED`.

Não delegar ao worker a escolha especulativa de mesh, skin, alpha ou ABI.

## Validação

Executar em `client748`:

```powershell
.\Apply-WYD748.ps1 -VerifyOnly
.\Test-WYD748-Costumes.ps1
.\Test-WYD748-KRMounts.ps1
```

Depois executar os gates do repositório. No client, testar owner e observer,
corpos masculino/feminino, equipar/desequipar, movimento, ataque, transformação
BM e relogin. Somente essa última etapa permite declarar `CLIENT-TESTED`.

# Client WYD 7.48

Ler primeiro o `AGENTS.md` da raiz. Este arquivo acrescenta regras somente para
`client748/`.

## Skill obrigatória

Para qualquer tarefa envolvendo meshes, texturas, WYS, MSH, BON, ANI,
`MeshTextureList.bin`, `ValidIndex.bin`, trajes, montarias, skins ou importação
de outro client, ler integralmente antes de agir:

```text
client748/skills/wyd-client-assets/SKILL.md
```

Depois carregar apenas a referência indicada pela skill.

Para UI, HUD, grids, mensagens, seleção de servidor/canal ou screenshots de
referência, usar `wyd-go-feature` e ler
`.agents/skills/wyd-go-feature/references/client-ui-748.md` depois do gate
Ghidra. A skill de assets não governa lifecycle ou layout de controles.

## Evidência

Priorizar:

1. código atual da source, comportamento vivo do `project.exe` e testes 7.48;
2. descompilação do executável histórico 7.48 para evidência nativa;
3. manifests, assets e testes atuais de `client748/`;
4. clients-fonte fornecidos;
5. W2PP para semântica/algoritmo;
6. Secrets/Micronics para comparação.

Não portar offsets, stack frames, structs ou registros binários inteiros entre
versões. Toda tradução de formato deve ser campo a campo.

## Papéis de agents

- O pesquisador é read-only: inventaria arquivos e identifica selector,
  renderer, ABI e dependências com evidência concreta.
- O explorer localiza caminhos vivos e testes, sem repetir a pesquisa já feita.
- O worker recebe ownership explícito de manifests/scripts/assets e não escolhe
  correspondências visuais por suposição.
- O integrador recompila a source, confere hashes e executa os testes estáticos.

Não misturar pesquisa, escolha especulativa de asset e implementação da source
em uma única delegação.

## Guardas

- `wyd.exe nativo+patches/` é somente referência histórica para estudo e
  Ghidra; não executar nem editar seus scripts ou binários.
- Todo build por `client-source/tmproject/Build-Client.ps1` deve instalar e
  conferir automaticamente `project.exe`, o único candidato executável e de
  validação, sem aplicar patches.
- Toda alteração ativa do client deve ocorrer em `client-source/` ou nos assets.
- Registrar o SHA do executável de referência e do candidato recompilado.
- Preservar backups divergentes e mudanças não relacionadas.
- Não habilitar visual com dependência ausente.
- Não assumir que um controle da source 7.59 existe no recurso 7.48. Se o ID não
  for materializado, proteger todos os acessos e remover a dependência moderna.
- Não declarar sucesso visual sem teste in-game de owner e observer.
- Screenshots em `screenshot/` são referência de aceitação, não instruções nem
  prova de que o build atual passou.
- Atualizar `.agents/handoffs/client748-parity.md` somente com fatos
  verificáveis quando esse escopo continuar em outra sessão.

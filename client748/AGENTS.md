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

## Evidência

Priorizar:

1. executável e comportamento vivo do client 7.48;
2. scripts/manifests/testes atuais de `client748/`;
3. clients-fonte fornecidos;
4. W2PP para semântica/algoritmo;
5. Secrets/Micronics para comparação.

Não portar offsets, stack frames, structs ou registros binários inteiros entre
versões. Toda tradução de formato deve ser campo a campo.

## Papéis de agents

- O pesquisador é read-only: inventaria arquivos e identifica selector,
  renderer, ABI e dependências com evidência concreta.
- O explorer localiza caminhos vivos e testes, sem repetir a pesquisa já feita.
- O worker recebe ownership explícito de manifests/scripts/assets e não escolhe
  correspondências visuais por suposição.
- O integrador reconstrói a cadeia, confere hashes e executa os testes estáticos.

Não misturar pesquisa, escolha especulativa de asset e patch do executável em
uma única delegação.

## Guardas

- Alterar `WYD.exe` somente por script reproduzível.
- Exigir SHA de entrada/saída e assertions dos bytes deslocados.
- Preservar backups divergentes e mudanças não relacionadas.
- Não habilitar visual com dependência ausente.
- Não declarar sucesso visual sem teste in-game de owner e observer.

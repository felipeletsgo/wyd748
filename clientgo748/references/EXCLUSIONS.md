# Exclusões e limites

- W2PP, Secrets e Micronics não são copiados como fonte de contrato e não
  podem ser usados para validar o client.
- Patches e executáveis históricos são somente leitura; não executar patchers,
  não editar `WYD.exe` e não tratar um binário patchado como o candidato ativo.
- `client748/project.exe` é um artefato de validação da source TMProject atual,
  não a implementação do novo client Go.
- Código C++ em `references/tmproject/TMProject748` é um snapshot para
  comparação e migração de ideias, não uma dependência compilada de
  `clientgo748`.
- A cópia em `references/server-source` é uma fotografia para conferência de
  contratos; os fontes foram renomeados para `.go.txt` para não entrarem no
  build do client. Regras continuam autoritativas no servidor do módulo raiz.
- Arquivos gerados em `bin/` são locais e ignorados pelo Git.

Ausência de equivalente nativo não autoriza remover um recurso. Quando uma
feature nova for necessária, ela deverá ser registrada como
`EXTENSAO_COORDENADA`, com contrato client/server, IDs/opcodes sem colisão,
fallback, rollback e testes dos dois lados.

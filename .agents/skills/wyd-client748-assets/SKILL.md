---
name: wyd-client748-assets
description: Auditar e adaptar assets do client 7.48 mantendo identidade, caminhos, dependências e compatibilidade com o server.
---

# Assets do client 7.48

Use esta skill para assets, recursos visuais, UI, sons ou arquivos de runtime
sob `tmproject/client748/`.

- Primeiro classifique o delta e, para uma fronteira legada, use
  `wyd-client748-research` para provar recurso, ID, binding, input, ownership,
  fechamento e teardown.
- Preserve nomes, formatos e arquivos existentes. A ausência no nativo 7.48
  não autoriza remover um asset manualmente criado; demonstrar incompatibilidade
  no runtime, contrato ou build.
- Diferencie assets efetivamente usados pelo build dos assets históricos ou de
  referência. Registre origem, hash quando a identidade importar e destino
  materializado em `DOCS/` ou `.agents/research/`, nunca em um dump na source.
- `references/client748/` é somente evidência histórica;
  não substituir seus binários por um resultado de adaptação.
- Quando o asset participa de uma extensão coordenada, documente o ID,
  versionamento/capability, fallback e o efeito esperado no `wydgo748/`.
- Builds, conversões e inspeções devem usar diretórios temporários ignorados e
  removíveis; não deixar scripts, logs ou artefatos gerados espalhados.

Valide o build do client e, quando houver contrato novo, testes do servidor e
do client. Declare separadamente validação estática, automatizada e no client
real.

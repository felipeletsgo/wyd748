# Source-built WYD client

Ler primeiro o `AGENTS.md` da raiz e a skill `wyd-go-feature`.

Esta arvore e o client recompilavel em migracao. Ela nao substitui o client
distribuido em `client748/` antes de atingir paridade funcional e visual.

## Versao unica

Esta source deve compilar exclusivamente o WYD 7.48. Ela nao deve conservar uma
segunda implementacao para TMProject 7.59+ nem funcionar como client
multi-versao.

- confirmar primeiro o caminho nativo no Ghidra do `client748/WYD.exe`;
- portar o comportamento suportado para uma unica implementacao 7.48;
- remover o branch, recurso, opcode, struct ou loader exclusivo de versao mais
  nova assim que seus callers 7.48 estiverem cobertos;
- eliminar flags `compat` quando todos os seus usos no subsistema tiverem sido
  reduzidos ao caminho 7.48;
- manter sources posteriores somente como referencia externa de algoritmo;
- traduzir dados novos para a representacao 7.48, nunca manter o formato 7.59
  em paralelo dentro do executavel.

Nao fazer exclusao em massa por heuristica. Cada corte precisa registrar no
comentario da source qual contrato 7.48 substituiu o caminho removido e deve
passar pelo build e pelo fluxo real afetado.

## Contratos

- Toda alteracao nesta source deve possuir comentario explicativo junto ao
  codigo editado, registrando intencao, contrato e motivo da compatibilidade.
  Comentarios devem explicar o "por que", nao apenas repetir a operacao.
- `ExtendedScore` no servidor continua autoritativo.
- Estruturas C++ e packets devem possuir `static_assert`/testes de tamanho e
  offsets; nunca depender de `sizeof` implicito sem teste do wire.
- Campos recebidos do client sao intencoes e nunca restauram estado de score.
- Manter Win32 durante a fase de compatibilidade. Uma migracao x64 altera ABI,
  alinhamento e dependencias e deve ser um projeto separado.
- Alteracoes visuais e importacao de assets continuam seguindo
  `client748/skills/wyd-client-assets/SKILL.md` e suas evidencias.
- Nao copiar structs, offsets ou enderecos de W2PP/Secrets/Micronics. Portar
  semantica e confirmar o layout nesta source e no servidor Go.
- O build deve passar por `tmproject/Build-Client.ps1`; nao versionar a pasta
  `build/`.

Migrar um grupo pequeno de packets por vez e manter o executavel distribuido
7.48 como fallback ate validacao in-game de owner e observer. O fallback e o
binario de referencia, nao codigo 7.59 preservado na source recompilavel.

# WYD-Go

Política canônica do repositório: prefira `gpt-5.6-luna` com esforço `max`
quando essa seleção estiver disponível na sessão Codex. O ambiente continua
autoridade sobre o modelo efetivamente usado; caso Luna não esteja disponível,
use o maior nível exposto e não declare uso de um modelo indisponível.

`AGENTS.md` é o contrato canônico deste repositório. Leia e siga suas regras
antes de trabalhar e não as duplique nem substitua.

## Skills são obrigatórias

Antes de qualquer tarefa técnica não trivial:

1. inspecione `.agents/skills`;
2. identifique as skills aplicáveis;
3. abra e leia o `SKILL.md` atual de cada skill necessária;
4. siga o workflow e as validações definidos pelas skills;
5. somente então analise, planeje, edite ou implemente.

O diretório canônico de skills versionadas dentro do repositório é:

```text
.agents/skills
```

Resolva-o a partir da raiz do repositório; não codifique caminhos absolutos da
máquina do desenvolvedor. Considere também skills fornecidas pela sessão quando
a tarefa exigir uma integração específica.

Se existir uma skill pertinente, seu uso é **obrigatório**. Não trabalhe apenas
com conhecimento próprio do modelo e não considere uma skill usada sem abrir o
`SKILL.md` na tarefa atual.

Para qualquer tarefa relacionada ao WYD — incluindo mecânicas, protocolo,
client source 7.48, descompilação histórica, assets, itens, skills, mobs, bosses,
quests, instâncias, combate, affects, inventário, crafting, macros ou pesquisa
em W2PP/Secrets/Micronics — abra **antes de qualquer análise**:

```text
.agents/skills/wyd-go-feature/SKILL.md
```

No client, `client-source/tmproject` e os assets são as únicas fronteiras de
implementação. O conteúdo de `client748/wyd.exe nativo+patches/` é histórico e
read-only: nunca executar, editar, verificar ou reutilizar seus `.ps1`, nem
tratar um binário patchado como produto ou fallback. Compilar a source com
`client-source/tmproject/Build-Client.ps1`, que deve instalar e validar
automaticamente o candidato como `client748/project.exe`.

Para GitHub, PR, CI, review ou publicação, use obrigatoriamente a skill GitHub
mais específica disponível na sessão Codex ou no repositório.

Se uma skill obrigatória estiver ausente ou inacessível, declare a limitação;
não finja que ela foi consultada e não declare a tarefa concluída sem cumprir
os gates definidos em `AGENTS.md`.

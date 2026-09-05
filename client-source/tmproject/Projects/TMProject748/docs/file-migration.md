# Migracao arquitetural por arquivo

Este registro acompanha a reorganizacao do `TMProject748` do menor fonte para o
maior. O tamanho e o caminho original referem-se ao clone antes da movimentacao;
a copia permanente para consulta continua em `../TMProject`.

O modo do lote e `MODERNIZACAO_COMPATIVEL`: caminho e documentacao podem mudar,
mas comportamento, wire, ABI, recursos e lifecycle permanecem inalterados.

| Ordem | Bytes | Arquivo original | Destino | Decisao |
| ---: | ---: | --- | --- | --- |
| 1 | 142 | `Quest.h` | `internal/game/quest/Quest.h` | Placeholder do dominio `game`; nao representa sistema de quests implementado. |
| 2 | 143 | `MD5Checksum.h` | `internal/wire/crypto/MD5Checksum.h` | Placeholder de integridade/codec do dominio `wire`. |
| 3 | 146 | `pch.cpp` | `internal/platform/windows/pch.cpp` | Unidade exclusiva do toolchain Win32, sem runtime. |
| 4 | 164 | `desbase64.h` | `internal/wire/crypto/desbase64.h` | Placeholder nominal de codec legado. |
| 5 | 170 | `Base64.h` | `internal/wire/crypto/Base64.h` | Placeholder nominal de codec. |
| 6 | 171 | `StackWalker.h` | `internal/platform/diagnostics/StackWalker.h` | Placeholder de diagnostico dependente de plataforma. |
| 7 | 172 | `BlockKeyBoard.h` | `internal/game/input/BlockKeyBoard.h` | Placeholder de input; nenhum bloqueio ativo foi inferido. |
| 8 | 172 | `NewClientB.h` | `internal/app/compat/NewClientB.h` | Placeholder de compatibilidade, sem criar segunda versao ativa. |
| 9 | 173 | `md5.h` | `internal/wire/crypto/md5.h` | Placeholder separado ate a API historica ser conhecida. |
| 10 | 203 | `TMProject.h` | `cmd/client/TMProject.h` | Interface minima do executavel, paralela a `cmd/server` no Go. |
| 11 | 219 | `AES.h` | `internal/wire/crypto/AES.h` | Placeholder nominal de criptografia do dominio `wire`. |
| 12 | 269 | `targetver.h` | `internal/platform/windows/targetver.h` | Selecao do SDK Win32; nao possui estado de runtime. |
| 13 | 270 | `TMBike.h` | `internal/game/entities/TMBike.h` | Entidade movel da cena; preserva o contrato e o layout de `TMObject`. |
| 14 | 308 | `TMDust.h` | `internal/render/effects/environment/TMDust.h` | Efeito ambiental temporario, sem regra autoritativa de jogo. |
| 15 | 319 | `TMRain.h` | `internal/render/effects/weather/TMRain.h` | Efeito climatico com buffers internos de 50 gotas. |
| 16 | 320 | `TMSkillHaste.h` | `internal/render/effects/skills/TMSkillHaste.h` | Apresentacao visual de Haste; nao aplica a habilidade. |
| 17 | 328 | `TMSkillBash.h` | `internal/render/effects/skills/TMSkillBash.h` | Apresentacao visual de Bash; dano e acerto ficam fora da classe. |
| 18 | 338 | `TMSkillCure.h` | `internal/render/effects/skills/TMSkillCure.h` | Apresentacao visual de Cure; cura continua fora do renderer. |
| 19 | 338 | `TMSkillHeavenDust.h` | `internal/render/effects/skills/TMSkillHeavenDust.h` | Efeito visual; nome historico do arquivo e classe plural foram preservados. |

Arquivos placeholder continuam compilaveis e documentados. Eles nao recebem
APIs inventadas apenas para preencher nomes historicos.

Durante a migracao incremental, o projeto adiciona os novos diretorios ao
search path de includes. Isso mantem fontes maiores ainda nao processadas
compilaveis sem edita-las fora da ordem definida.

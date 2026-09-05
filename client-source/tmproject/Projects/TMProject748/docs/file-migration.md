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
| 20 | 341 | `TMSkillFlash.h` | `internal/render/effects/skills/TMSkillFlash.h` | Apresentacao visual de Flash; nao define regras de alvo. |
| 21 | 347 | `TMSkillSpeedUp.h` | `internal/render/effects/skills/TMSkillSpeedUp.h` | Apresentacao visual; velocidade continua fora do renderer. |
| 22 | 353 | `TMSkillHolyTouch.h` | `internal/render/effects/skills/TMSkillHolyTouch.h` | Efeito temporario configurado por posicao e variante. |
| 23 | 359 | `TMSkillThunderBolt.h` | `internal/render/effects/skills/TMSkillThunderBolt.h` | Apresentacao visual; nao decide acerto nem dano. |
| 24 | 363 | `TMDrop.h` | `internal/render/effects/weather/TMDrop.h` | Precipitacao localizada com respingos e audio ambiente. |
| 25 | 379 | `TMEffectLevelUp.h` | `internal/render/effects/status/TMEffectLevelUp.h` | Apresentacao de level up; nao altera nivel ou atributos. |
| 26 | 384 | `TMSkillHeal.h` | `internal/render/effects/skills/TMSkillHeal.h` | Efeito visual com referencia nao proprietaria ao objeto. |
| 27 | 385 | `TMLog.h` | `internal/platform/diagnostics/TMLog.h` | API de log dependente de Win32 e handle global. |
| 28 | 390 | `TMSkillPoison.h` | `internal/render/effects/skills/TMSkillPoison.h` | Particulas de poison; dano periodico fica fora do renderer. |
| 29 | 405 | `TMSnow.h` | `internal/render/effects/weather/TMSnow.h` | Neve e audio ambiente ao redor da camera/personagem. |
| 30 | 416 | `TMSkillLusterFurnish.h` | `internal/render/effects/skills/TMSkillLusterFurnish.h` | Trajetoria visual temporaria; o nome historico `TMSKill` foi preservado. |
| 31 | 422 | `TMProject.cpp` | `cmd/client/TMProject.cpp` | Unidade minima do executavel; nao define bootstrap ou estado proprio. |
| 32 | 426 | `TMSkillJudgement.h` | `internal/render/effects/skills/TMSkillJudgement.h` | Controlador de variantes visuais; regras de combate permanecem fora do renderer. |
| 33 | 436 | `framework.h` | `internal/platform/windows/framework.h` | Fachada minima do SDK Win32 e runtime C legado. |
| 34 | 457 | `TMEffectStart.h` | `internal/render/effects/skills/TMEffectStart.h` | Inicio visual com owner opcional; variante ciclica depende de remocao externa. |
| 35 | 491 | `TMLeaf.h` | `internal/render/world/vegetation/TMLeaf.h` | Objeto visual de vegetacao com lifecycle de skin mesh e dispositivo. |
| 36 | 491 | `TMSun.h` | `internal/render/world/sky/TMSun.h` | Sol e lens flares pertencentes ao renderer da cena celeste. |
| 37 | 498 | `CFrame.h` | `internal/render/animation/skeleton/CFrame.h` | No da hierarquia esqueletal; possui a malha, mas nao as ligacoes de filhos, irmaos ou skin. |
| 38 | 499 | `TMLight.h` | `internal/render/lighting/TMLight.h` | Luz pontual, culling e sincronizacao Direct3D; cena, camera e dispositivo permanecem externos. |
| 39 | 502 | `TMEffectDust.h` | `internal/render/effects/environment/TMEffectDust.h` | Poeira, pedras e respingos de impacto; malhas continuam pertencendo ao cache global. |
| 40 | 504 | `TMSkillSlowSlash.h` | `internal/render/effects/skills/TMSkillSlowSlash.h` | Trajetoria e particulas de habilidade com owner temporario nao proprietario. |
| 41 | 510 | `TMSkillSpChange.h` | `internal/render/effects/skills/TMSkillSpChange.h` | Composicao visual de mudanca de SP; objetos auxiliares usam o container global. |
| 42 | 520 | `TMEffect.h` | `internal/render/effects/core/TMEffect.h` | Base dos efeitos visuais, com transformacao, alpha e visibilidade comuns. |
| 43 | 529 | `TMFireEffect.h` | `internal/render/effects/skills/TMFireEffect.h` | Rastro visual de fogo; o alvo e apenas amostrado para calcular a trajetoria. |
| 44 | 535 | `TMSkillFire.h` | `internal/render/effects/skills/TMSkillFire.h` | Emissor visual de fogo parametrizado; owner e light map nao sao possuidos. |
| 45 | 549 | `TMCannon.h` | `internal/game/entities/TMCannon.h` | Entidade de canhao; apresenta recuo e disparo sem decidir projeteis ou dano. |
| 46 | 558 | `TMFlail.cpp` | `internal/game/entities/TMFlail.cpp` | Stub inerte da entidade visual; o cabecalho aguarda sua propria posicao na fila. |
| 47 | 565 | `TMSkillExplosion2.h` | `internal/render/effects/skills/TMSkillExplosion2.h` | Controlador radial visual; efeitos filhos pertencem ao container da cena. |
| 48 | 583 | `TMHouse.h` | `internal/render/world/structures/TMHouse.h` | Construcao visual do mapa; camera, terreno, meshes e container permanecem externos. |
| 49 | 592 | `TMSkillIceSpear.h` | `internal/render/effects/skills/TMSkillIceSpear.h` | Projetil visual; possui o light map e apenas observa o alvo temporariamente. |
| 50 | 592 | `TMEffectParticle.h` | `internal/render/effects/particles/TMEffectParticle.h` | Emissor generico; transfere os billboards criados ao container da cena. |
| 51 | 597 | `TMEffectGold.cpp` | `internal/render/effects/pending/TMEffectGold.cpp` | Stub visual inerte e sem consumidores; isolado ate existir contrato seguro para adapta-lo. |
| 52 | 601 | `TMTree.h` | `internal/render/world/vegetation/TMTree.h` | Vegetacao animada; possui a skin mesh herdada e transfere particulas ao container da cena. |
| 53 | 604 | `TMSkillFreezeBlade.h` | `internal/render/effects/skills/TMSkillFreezeBlade.h` | Segmento visual de gelo; transfere particulas e continuacoes ao container da cena. |
| 54 | 633 | `TMVideoWnd.cpp` | `internal/platform/windows/media/TMVideoWnd.cpp` | Adaptador DirectShow inativo; preserva a API sem simular reproducao ou lifecycle COM. |
| 55 | 637 | `TMFloat.h` | `internal/render/world/water/TMFloat.h` | Objeto visual aquatico; preserva a relacao legada entre billboards retidos e o container global. |
| 56 | 643 | `TMObjectContainer.h` | `internal/render/world/objects/TMObjectContainer.h` | Agregado visual da regiao; possui filhos e luzes, mas apenas referencia o terreno da cena. |
| 57 | 706 | `TMVideoWnd.h` | `internal/platform/windows/media/TMVideoWnd.h` | Declaracao do adaptador DirectShow; a implementacao permanece inativa e sem lifecycle COM simulado. |
| 58 | 819 | `TMFlail.h` | `internal/game/entities/TMFlail.h` | Declaracao da entidade visual; preserva o stub e o contrato historico sem inventar comportamento. |
| 59 | 988 | `TMEffectGold.h` | `internal/render/effects/pending/TMEffectGold.h` | Declaracao do efeito pendente; mantida isolada junto ao cpp sem consumidores confirmados. |

Arquivos placeholder continuam compilaveis e documentados. Eles nao recebem
APIs inventadas apenas para preencher nomes historicos.

Durante a migracao incremental, o projeto adiciona os novos diretorios ao
search path de includes. Isso mantem fontes maiores ainda nao processadas
compilaveis sem edita-las fora da ordem definida.
| 60 | 659 | `TMSea.h` | `internal/render/world/water/TMSea.h` | Interface da agua da cena; preserva mesh, picking e lifecycle visual. |
| 61 | 659 | `TMEffectCharge.h` | `internal/render/effects/core/TMEffectCharge.h` | Efeito visual base de carga; regras de combate e ownership externo permanecem inalterados. |
| 62 | 667 | `TMUtil.h` | `internal/platform/windows/TMUtil.h` | Funcoes utilitarias legadas dependentes de estruturas e audio Win32. |
| 63 | 642 | `TMSkillTownPortal.h` | `internal/render/effects/skills/TMSkillTownPortal.h` | Efeito de habilidade de teleporte; preserva a declaracao legada. |
| 64 | 727 | `TMSkillMagicShield.h` | `internal/render/effects/skills/TMSkillMagicShield.h` | Efeito de habilidade de escudo magico; preserva a declaracao legada. |
| 65 | 734 | `TMSkillMagicArrow.h` | `internal/render/effects/skills/TMSkillMagicArrow.h` | Efeito de habilidade de flecha magica; preserva a declaracao legada. |
| 66 | 991 | `TMSkillSnow.h` | `internal/render/effects/skills/TMSkillSnow.h` | Efeito de habilidade de neve; preserva a declaracao legada. |
| 67 | 1198 | `TMSkillDoubleSwing.h` | `internal/render/effects/skills/TMSkillDoubleSwing.h` | Declaracao de habilidade de combate; preserva contrato visual. |
| 68 | 1186 | `TMSkillMeteorStorm.h` | `internal/render/effects/skills/TMSkillMeteorStorm.h` | Declaracao de habilidade meteorologica; preserva contrato visual. |
| 69 | 9065 | `TMSkillDoubleSwing.cpp` | `internal/render/effects/skills/TMSkillDoubleSwing.cpp` | Implementacao movida sem alteracao semantica. |
| 70 | 23558 | `TMSkillMeteorStorm.cpp` | `internal/render/effects/skills/TMSkillMeteorStorm.cpp` | Implementacao movida sem alteracao semantica. |
| 71 | 1487 | `TMSkillBash.cpp` | `internal/render/effects/skills/TMSkillBash.cpp` | Implementacao de habilidade movida sem alteracao semantica. |
| 72 | 2672 | `TMSkillCure.cpp` | `internal/render/effects/skills/TMSkillCure.cpp` | Implementacao de habilidade movida sem alteracao semantica. |
| 73 | 3515 | `TMSkillExplosion2.cpp` | `internal/render/effects/skills/TMSkillExplosion2.cpp` | Implementacao de habilidade movida sem alteracao semantica. |
| 74 | 5410 | `TMSkillFire.cpp` | `internal/render/effects/skills/TMSkillFire.cpp` | Implementacao de habilidade movida sem alteracao semantica. |
| 75 | 2521 | `TMSkillFlash.cpp` | `internal/render/effects/skills/TMSkillFlash.cpp` | Implementacao de habilidade movida sem alteracao semantica. |
| 76 | 7808 | `TMSkillFreezeBlade.cpp` | `internal/render/effects/skills/TMSkillFreezeBlade.cpp` | Implementacao de habilidade movida sem alteracao semantica. |
| 77 | 2914 | `TMSkillHaste.cpp` | `internal/render/effects/skills/TMSkillHaste.cpp` | Implementacao de habilidade movida sem alteracao semantica. |
| 78 | 1633 | `TMSkillHeal.cpp` | `internal/render/effects/skills/TMSkillHeal.cpp` | Implementacao de habilidade movida sem alteracao semantica. |
| 79 | 1931 | `TMSkillHeavenDust.cpp` | `internal/render/effects/skills/TMSkillHeavenDust.cpp` | Implementacao de habilidade movida sem alteracao semantica. |
| 80 | 3208 | `TMSkillHolyTouch.cpp` | `internal/render/effects/skills/TMSkillHolyTouch.cpp` | Implementacao de habilidade movida sem alteracao semantica. |

| 81-220 | lote | arquivos restantes do raiz | internal/* | Reorganizacao por dominio sem alteracao semantica; caminhos do projeto atualizados automaticamente. |

| 221-250 | refinamento | misc/world | subdominios render/camera/resources/effects/terrain/sky/objects | Reclassificacao por responsabilidade, sem mudanca semantica. |

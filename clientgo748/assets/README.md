# Assets do novo client

`current/` é o snapshot dos assets ativos que o novo client Go deverá
consumir. Ele contém 7.370 arquivos e 347.172.280 bytes canônicos, incluindo `Effect`,
`Env`, `mesh`, `Shader`, `UI` e as tabelas binárias/JSON relevantes da raiz do
client atual. O tamanho textual usa LF canônico para não variar entre checkouts
Windows e Linux; formatos binários permanecem byte-exact.

Este snapshot é separado de `CLIENT OFICIAL 7.48/`:

- `CLIENT OFICIAL 7.48/` é a baseline histórica imutável;
- `assets/current/` é a entrada de desenvolvimento para conversão e build;
- o produto distribuído deverá consumir um pacote versionado, autenticado e
  criptografado, conforme `SECURITY.md`.

Executáveis, DLLs, drivers, logs, screenshots, patchers e backups não foram
classificados como assets ativos. Os arquivos originais permanecem preservados
e o futuro `assetc` deverá gerar saídas sem sobrescrevê-los.

Os formatos presentes incluem WYS, MSA, MSH, ANI, WYT, TRN, BON, DAT e BIN.
Nenhum loader é considerado implementado apenas porque o arquivo foi copiado:
selector, layout, dependências, teardown e comportamento owner/observer ainda
precisam ser fechados por fluxo.

# Evidência Ghidra autocontida

Este diretório materializa a análise nativa usada pelo programa de mapeamento.
Nenhum comando de pesquisa do `clientgo748` precisa buscar o projeto, o corpus
ou o executável fora deste pacote.

## Conteúdo

- `input/WYD.exe`: executável exato analisado, somente leitura;
- `project/WYD748Native_20260821.gpr`: descritor do projeto Ghidra;
- `project/WYD748Native_20260821.rep/`: banco completo do projeto;
- `corpus/`: 4.146 funções exportadas em C, mais o índice auxiliar do corpus.

## Fingerprint obrigatório

```text
input/WYD.exe
size:    2,060,288 bytes
SHA-256: 8AA2F918844BCE3AFE21F1204F69757A443E32EB2F2F616936B1D9BFE215F593
```

O arquivo `.gpr` vazio é normal: os dados persistentes ficam no diretório
`.rep`. O descritor tem SHA-256
`E3B0C44298FC1C149AFBF4C8996FB92427AE41E4649B934CA495991B7852B855`.

O corpus serve para busca e triagem. Claims sobre callback, vtable, chamada
indireta, ownership ou lifecycle devem ser confirmados no projeto Ghidra. Um
endereço citado que não seja entry point pode pertencer ao corpo de outra
função e não representa automaticamente uma função ausente.

O projeto foi copiado como evidência imutável. Novas análises devem usar uma
cópia de trabalho ou registrar claramente qualquer mudança e seu novo hash.

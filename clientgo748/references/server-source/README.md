# Snapshot de contratos do servidor

As pastas `wire`, `game` e `model` são cópias somente de leitura do WYD-Go
usadas para correlacionar o client com o servidor autoritativo. Os arquivos
que eram Go foram renomeados para `.go.txt`, portanto este snapshot não entra
em `go test ./clientgo748/...` nem se torna uma segunda implementação.

Ao alterar um contrato, editar primeiro a source viva no módulo raiz, atualizar
os testes do servidor e só depois renovar esta fotografia. A cópia não é fonte
de verdade nem deve ser importada pelo client Go.

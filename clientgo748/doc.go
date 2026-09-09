// Package clientgo748 abriga a implementação incremental do client WYD 7.48
// em Go.
//
// Este pacote ainda não implementa gameplay nem renderização. As subpastas
// separam as responsabilidades para que cada fronteira possa ser comparada
// com o client nativo, os assets 7.48 e o contrato do servidor antes de se
// tornar um caminho ativo. O TMProject fornece somente referência estrutural;
// sua implementação não é copiada por conter bugs e lifecycle implícito.
// O processo principal não deve alterar o estado autoritativo do jogo; ele
// apenas envia intenções e apresenta o estado recebido.
package clientgo748

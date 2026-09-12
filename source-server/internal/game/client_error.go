package game

// clientFacingError distingue mensagens prontas para a UI de erros internos.
// Erros Go comuns seguem o estilo minusculo e sem pontuacao; estes preservam a
// capitalizacao/traducao que o jogador deve receber em MessagePanel.
type clientFacingError string

func (e clientFacingError) Error() string { return string(e) }

func clientError(message string) error { return clientFacingError(message) }

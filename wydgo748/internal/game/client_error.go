package game

// clientFacingError distinguishes UI-ready messages from internal errors.
// Ordinary Go errors are lowercase and unpunctuated; these preserve the
// capitalization and wording that players should see in MessagePanel.
type clientFacingError string

func (e clientFacingError) Error() string { return string(e) }

func clientError(message string) error { return clientFacingError(message) }

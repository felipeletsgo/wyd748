package store

import "errors"

// rejectedWrite certifies that the adapter did not commit the requested write.
// Keep construction private: callers must not relabel an ambiguous store error.
type rejectedWrite struct{ cause error }

// unknownWrite takes precedence over a rejection in its error chain. For
// example, a rejected callback followed by a failed rollback is not a certified
// rejection of the transaction as a whole.
type unknownWrite struct{ cause error }

func (e *unknownWrite) Error() string { return e.cause.Error() }
func (e *unknownWrite) Unwrap() error { return e.cause }

func indeterminateWrite(err error) error { return &unknownWrite{cause: err} }

func (e *rejectedWrite) Error() string {
	return "store: write rejected before commit: " + e.cause.Error()
}
func (e *rejectedWrite) Unwrap() error { return e.cause }

func rejectWrite(err error) error {
	if err == nil || IsWriteRejected(err) {
		return err
	}
	return &rejectedWrite{cause: err}
}

// IsWriteRejected distinguishes adapter-certified rejection from an unknown
// commit outcome. It does not authorize replay, rollback or releasing resources;
// the operation owner must separately verify its rejection continuation.
func IsWriteRejected(err error) bool {
	var unknown *unknownWrite
	if errors.As(err, &unknown) {
		return false
	}
	var rejected *rejectedWrite
	return errors.As(err, &rejected)
}

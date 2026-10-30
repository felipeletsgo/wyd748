package wire

// InstanceQuestion is a display-only coordinated extension, not an answer request.
// Duration zero closes the panel. See DOCS/protocol/instance-question.md.
const (
	OpInstanceQuestion   = 0x7F12
	InstanceQuestionSize = 116
)

func InstanceQuestion(id uint16, question string, durationMS uint32) []byte {
	return InstanceQuestionText(id, 0, question, durationMS)
}

// InstanceQuestionText uses version 2 when textIndex names a UI/strdef.bin
// row. The client shows that localized row; question is the English fallback.
func InstanceQuestionText(id, textIndex uint16, question string, durationMS uint32) []byte {
	b := Build(OpInstanceQuestion, id, InstanceQuestionSize)
	putU16(b, 12, 1)
	if textIndex != 0 && question != "" && durationMS > 0 {
		putU16(b, 12, 2)
		putU16(b, 14, textIndex)
	}
	if durationMS > 10000 {
		durationMS = 10000
	}
	if question != "" && durationMS > 0 {
		putU32(b, 16, durationMS)
		copy(b[20:115], question)
	}
	return b
}

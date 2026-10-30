package game

import (
	"strconv"
	"strings"

	"wydgo/internal/wire"
)

// UI/strdef.bin rows with EN/PT Cube and OX notices (EXTENSAO_COORDENADA).
// The client picks the language; MessagePanel keeps English for other names.
const (
	msgCubeRoom                int16 = 484 // "Cube %s"
	msgCubeFinal               int16 = 485
	msgBigCube                 int16 = 486
	msgBigCubeWrong            int16 = 487
	msgQuizCorrect             int16 = 488 // "Correct! +%s EXP."
	msgOXEnrollmentOpen        int16 = 489
	msgOXEnterDuringEnrollment int16 = 490
	msgOXWrong                 int16 = 491
	msgOXEnrollmentCloses      int16 = 492 // "... in %s seconds."
	msgOXNextQuestion          int16 = 493 // "... in %s seconds."
	msgCubeEntranceBlocked     int16 = 494
	msgCubeWrong               int16 = 495
)

func quizCorrectPacket(exp uint32) []byte {
	return wire.MessageParameterized(msgQuizCorrect-1000, strconv.FormatUint(uint64(exp), 10))
}

// instanceStageNamePacket localizes the Cube stage names; any other stage
// name keeps the configured English text.
func instanceStageNamePacket(name string) []byte {
	switch name {
	case "Cube Final":
		return wire.MessageIndexed(msgCubeFinal - 1000)
	case "Big Cube":
		return wire.MessageIndexed(msgBigCube - 1000)
	}
	if room, ok := strings.CutPrefix(name, "Cube "); ok {
		if n, err := strconv.Atoi(room); err == nil && n > 0 {
			return wire.MessageParameterized(msgCubeRoom-1000, room)
		}
	}
	return wire.MessagePanel(name)
}

package game

import (
	"strconv"
	"strings"

	"wydgo/internal/model"
	"wydgo/internal/wire"
)

// WYD-Go rows 440..495 of the client UI/strdef.bin (EXTENSAO_COORDENADA).
// Their EN/PT text is also in the server catalog, so the server sends the
// channel language as a top notice instead of the client-language index.
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

// projectMessageKeys maps the WYD-Go strdef rows to server catalog keys.
var projectMessageKeys = map[int16]string{
	440: "_WG_Mystic_Land_Hunt",
	441: "_WG_Mystic_Land_Complete",
	442: "_WG_Mystic_Land_Start",
	443: "_WG_Mystic_Land_Claimed",
	444: "_WG_Mystic_Land_Reward",
	445: "_WG_Crystal_Weapon_Requirements",
	446: "_WG_Crystal_Weapon_Rejected",
	447: "_WG_Crystal_Weapon_Success",
	448: "_WG_Map_Fragments_Converted",
	449: "_WG_Service_Not_Available_Yet",
	450: "_WG_Zakum_Area_Empty",
	451: "_WG_Zakum_Area_Population",
	452: "_WG_BeastMaster_Summons_Dismissed",
	453: "_WG_Finish_Current_Action",
	454: "_WG_Equip_Apprentice_Emblem",
	455: "_WG_Apprentice_Level_Range",
	456: "_WG_Remove_Current_Cape",
	457: "_WG_Apprentice_Cape_Received",
	458: "_WG_Exchange_Not_Saved",
	484: "_WG_Cube_Room",
	485: "_WG_Cube_Final",
	486: "_WG_Big_Cube",
	487: "_WG_Big_Cube_Wrong",
	488: "_WG_Quiz_Correct_Exp",
	489: "_WG_OX_Enrollment_Open",
	490: "_WG_OX_Enter_During_Enrollment",
	491: "_WG_OX_Wrong_Answer",
	492: "_WG_OX_Enrollment_Closes",
	493: "_WG_OX_Next_Question",
	494: "_WG_Cube_Entrance_Blocked",
	495: "_NN_Incorrect_kicked",
}

// projectMessage sends a WYD-Go strdef row as catalog text in the channel
// language. Native rows keep the client index, whose text the client owns.
func (w *World) projectMessage(row int16, params ...string) []byte {
	if key, ok := projectMessageKeys[row]; ok {
		args := make([]any, len(params))
		for i, param := range params {
			args[i] = param
		}
		return wire.MessagePanel(w.msg(key, args...))
	}
	if len(params) == 0 {
		return wire.MessageIndexed(row - 1000)
	}
	return wire.MessageParameterized(row-1000, params...)
}

func (w *World) quizCorrectPacket(exp uint32) []byte {
	return w.projectMessage(msgQuizCorrect, strconv.FormatUint(uint64(exp), 10))
}

// instanceStageNamePacket shows a stage name in the channel language.
func (w *World) instanceStageNamePacket(name string) []byte {
	return wire.MessagePanel(w.tr(name))
}

// instanceQuestionPacket sends the question text in the channel language.
// Version 1 carries the text; the client-language strdef row is not used.
func (w *World) instanceQuestionPacket(id uint16, quiz *model.VolatileInstanceQuiz, durationMS uint32) []byte {
	return wire.InstanceQuestion(id, w.tr(strings.ReplaceAll(quiz.Question, "_", " ")), durationMS)
}

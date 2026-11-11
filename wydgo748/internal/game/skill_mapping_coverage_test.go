package game

import (
	"strconv"
	"testing"

	"wydgo/internal/model"
)

func TestServerSkillLearnedMapsTransKnightBits(t *testing.T) {
	for skill, serverSkill := range transKnightServerSkill {
		t.Run(strconv.Itoa(skill), func(t *testing.T) {
			ch := &model.Char{Class: 0, LearnedSkill: uint32(1) << skill}
			if !serverSkillLearned(ch, serverSkill) {
				t.Fatalf("learned client skill %d was not mapped to server skill %d", skill, serverSkill)
			}
			ch.LearnedSkill = 0
			if serverSkillLearned(ch, serverSkill) {
				t.Fatalf("unlearned client skill %d was reported for server skill %d", skill, serverSkill)
			}
		})
	}

	if serverSkillLearned(&model.Char{Class: 0, LearnedSkill: 1 << transKnightTwoHandMaster}, transKnightTwoHandMaster) {
		t.Fatal("unsupported TransKnight two-hand mastery was reported as learned")
	}

	ch := &model.Char{Class: 0, LearnedSkill: 1 << 5}
	if !serverSkillLearned(ch, 5) {
		t.Fatal("unmapped TransKnight skill did not use its local learned bit")
	}
	ch.LearnedSkill = 0
	if serverSkillLearned(ch, 5) {
		t.Fatal("unlearned unmapped TransKnight skill was reported as learned")
	}
}

func TestServerSkillLearnedMapsHuntressBits(t *testing.T) {
	for clientSkill, serverSkill := range huntressServerSkill {
		t.Run(strconv.Itoa(clientSkill), func(t *testing.T) {
			serverLocal := serverSkill - 72
			ch := &model.Char{Class: 3, LearnedSkill: uint32(1) << (clientSkill - 72)}
			if !serverSkillLearned(ch, serverLocal) {
				t.Fatalf("learned client skill %d was not mapped to server skill %d", clientSkill, serverSkill)
			}
			ch.LearnedSkill = 0
			if serverSkillLearned(ch, serverLocal) {
				t.Fatalf("unlearned client skill %d was reported for server skill %d", clientSkill, serverSkill)
			}
		})
	}

	if serverSkillLearned(&model.Char{Class: 3}, 11) {
		t.Fatal("server-only Huntress skill was reported as learned")
	}
	if serverSkillLearned(nil, 0) {
		t.Fatal("missing character was reported as having a learned skill")
	}
	if !serverSkillLearned(&model.Char{Class: 1, LearnedSkill: 1 << 5}, 5) ||
		serverSkillLearned(&model.Char{Class: 1}, 5) {
		t.Fatal("non-mapped class did not use its local learned bit")
	}
}

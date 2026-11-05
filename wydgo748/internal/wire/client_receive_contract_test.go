package wire

import (
	"bytes"
	"encoding/binary"
	"encoding/hex"
	"fmt"
	"os"
	"path/filepath"
	"testing"

	"wydgo/internal/model"
)

// These bytes come from production Go builders. The C++ architecture suite reads
// the same snapshot through its production receive gate. Neither side imports
// the other side's size constants. Regenerate only after reviewing both contracts.
func TestClientReceiveBuilderFixtures(t *testing.T) {
	score := &model.Score{Version: model.ScoreVersion, Level: 123, Str: 111, Int: 222,
		Dex: 333, Con: 444, CurHP: 123456, MaxHP: 654321, CurMP: 23456, MaxMP: 76543}
	item := model.Item{Index: 1234, Eff: [6]byte{1, 2, 3, 4, 5, 6}}
	ch := model.Char{Name: "go-sentinel", Class: 2, X: 123, Y: 456, Score: score, Gold: 1234567, Exp: 7654321}
	ch.Inv[0], ch.Equip[0] = item, item
	var tradeItems [15]model.Item
	var tradePos [15]int8
	var shopItems [12]model.Item
	var shopPos [12]int8
	var shopPrices [12]uint32
	tradeItems[0], shopItems[0], tradePos[0], shopPos[0], shopPrices[0] = item, item, 5, 6, 123456
	var prices [PremiumShopSlots]uint32
	var categories [PremiumShopSlots]uint8
	prices[0], prices[26], categories[0], categories[26] = 12345, 67890, 1, 3
	token := [16]byte{1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16}
	rows := []struct {
		name string
		data []byte
	}{
		{"message_panel", MessagePanel("go-sentinel")},
		{"message_chat", MessageChat(123, "go-sentinel")},
		{"message_whisper", MessageWhisper(0, "go-sentinel", "fixture-body", 0x1234)},
		{"message_indexed", MessageIndexed(123)},
		{"message_parameters", MessageParameterized(123, "alpha", "beta")},
		{"player_shout", PlayerShout("go-sentinel", "fixture-body")},
		{"day_sync", DaySync()},
		{"guild_challenge", GuildChallengePrompt()},
		{"motion", Motion(123, 100, 5)},
		{"firework", PremiumFirework(123, token)},
		{"gamble", ResultGamble(123, [5]byte{1, 2, 3, 4, 5}, [3]byte{6, 7, 8}, 123456, 987654)},
		{"capsule", CNFCapsuleInfo(123, 456, CapsuleInfoData{Class: 2, Level: 234, Str: 111, Int: 222, Dex: 333, Con: 444, Quest: 17})},
		{"party_request", PartyRequest(123, "go-sentinel", 2, 234, 1234, 5678, 456)},
		{"party_member", PartyMember(123, "go-sentinel", 2, 3, 234, 1234, 5678)},
		{"party_remove", PartyRemove(123)},
		{"trade", Trade(123, tradeItems, tradePos, 123456, true, 456)},
		{"trade_close", CloseTrade(123)},
		{"trade_check", CNFTradeCheck(123)},
		{"auto_trade", AutoTrade("go-sentinel", shopItems, shopPos, shopPrices, 7, 456)},
		{"item_sold", ItemSold(123, 11)},
		{"character_transfer", CharacterTransferUnavailable(2)},
		{"character_list", CharList("go-account", []model.Char{ch}, []model.Item{item}, 123456)},
		{"enter_world", EnterWorld(123, 2, ch)},
		{"character_created", CharacterSelectionUpdate(OpCNFNewCharacter, 123, []model.Char{ch})},
		{"character_deleted", CharacterSelectionUpdate(OpCNFDeleteCharacter, 123, []model.Char{ch})},
		{"character_failed", NewCharacterFail()},
		{"character_logout", CNFCharacterLogout(123)},
		{"already_playing", AlreadyPlaying()},
		{"weather", Weather(123, 2)},
		{"update_score", UpdateScore(123, ch)},
		{"hp_mp", SetHpMp(123, score)},
		{"affects", UpdateAffects(123, ch)},
		{"update_etc", UpdateEtc(123, ch)},
		{"short_skill", SetShortSkill(123, [20]byte{1, 2, 3, 4})},
		{"sys_quit", SysQuit(123)},
		{"carry", UpdateCarry(123, ch.Inv[:], 123456)},
		{"equip", SelfEquip(123, ch.Equip[:])},
		{"action_stop", ActionStop(123, 234, 345)},
		{"action_route", ActionRouteCorrection(123, 234, 345)},
		{"player_move", PlayerMove(123, 234, 345, 456, 567, 8, []byte{1, 2, 3})},
		{"attack_melee", AttackHit(123, 456, 10, 20, 30, 40, 12345, 20000, 76543, 1234)},
		{"attack_wide", AttackHitWide(123, 456, 10, 20, 30, 40, 123456, 654321, 76543, 1234)},
		{"skill_one", SkillHits(123, 10, 20, 30, 40, 76543, 1234, 29, 5, 1, 1, []SkillTarget{{ID: 456, Damage: 123456, MaxHP: 654321}})},
		{"skill_two", SkillHits(123, 10, 20, 30, 40, 76543, 1234, 29, 5, 1, 2, []SkillTarget{{ID: 456, Damage: 123456, MaxHP: 654321}, {ID: 457, Heal: 23456}})},
		{"skill_multi", SkillHits(123, 10, 20, 30, 40, 76543, 1234, 44, 5, 1, 13, []SkillTarget{{ID: 456, Damage: 123456, MaxHP: 654321}, {ID: 457, Miss: true}})},
		{"send_item", SendItem(123, 1, 62, item)},
		{"swap_item", SwapItem(123, 1, 5, 2, 6, 456)},
		{"buy_item", BuyItem(123, 456, 2, 3, 123456)},
		{"pickup", CNFGetItem(1, 62)},
		{"drop", CNFDropItem(1, 62, 3, 123, 456)},
		{"create_item", CreateItem(123, 456, 789, item, 1, 2, 3, 1, 123)},
		{"update_item", UpdateItem(789, 2)},
		{"remove_item", RemoveItem(789)},
		{"remove_mob", RemoveMob(123, 1)},
		{"mob_killed", CNFMobKill(123, 456, 1234, 5678)},
		{"war_info", WarInfo()},
		{"shop_list", ShopList([]model.Item{item}, 7, 0)},
		{"cargo_gold", UpdateCargoGold(123, 123456)},
		{"cargo_deposit", CargoGoldTransfer(OpDeposit, 123, 12345)},
		{"cargo_withdraw", CargoGoldTransfer(OpWithdraw, 123, 12345)},
		{"instance_time", StandardParm(OpInstanceTime, 123, 12345)},
		{"instance_mobs", StandardParm(OpInstanceMobs, 123, 27)},
		{"premium_result", PremiumBuyResult(123, PremiumResultSuccess, 123456, 654321)},
		{"premium_balance", PremiumBalance(123, 123456)},
		{"premium_v1", PremiumShopState(123, PremiumStateOpen, 17, 123456, prices)},
		{"premium_v4", PremiumCategorizedShopState(123, 17, 123456, prices, categories)},
		{"quiz", QuizChallenge(123, token, "go-sentinel", [4]int32{1, 2, 3, 4})},
		{"quiz_close", QuizClose(123, token)},
		{"instance_question", InstanceQuestion(123, "go-sentinel", 9000)},
		{"instance_localized", InstanceQuestionText(123, 123, "go-sentinel", 9000)},
		{"kefra_visible", KefraPortalState(true)},
		{"kefra_hidden", KefraPortalState(false)},
	}
	var output bytes.Buffer
	fmt.Fprintln(&output, "# name opcode plaintext_hex; production Go builders; Size finalized; Tick and cipher bytes zero")
	for _, row := range rows {
		if len(row.data) < HeaderSize {
			t.Fatalf("%s: incomplete builder header", row.name)
		}
		binary.LittleEndian.PutUint16(row.data, uint16(len(row.data)))
		fmt.Fprintf(&output, "%s %04X %s\n", row.name, binary.LittleEndian.Uint16(row.data[4:6]), hex.EncodeToString(row.data))
	}
	path := filepath.Join("..", "..", "..", "tmproject", "TMProject748", "tests", "fixtures", "go-receive-packets.txt")
	if os.Getenv("WYD_UPDATE_RECEIVE_FIXTURES") == "1" {
		if err := os.WriteFile(path, output.Bytes(), 0644); err != nil {
			t.Fatal(err)
		}
	}
	fixture, err := os.ReadFile(path)
	if err != nil {
		t.Fatal(err)
	}
	// Accept checkout line endings, but compare every generated payload byte.
	fixture = bytes.ReplaceAll(fixture, []byte("\r\n"), []byte("\n"))
	if !bytes.Equal(fixture, output.Bytes()) {
		t.Fatal("Go receive fixture differs from production builders; review both contracts before regeneration")
	}
	t.Logf("verified %d Go builder fixtures", len(rows))
}

#pragma once

#include "CharacterTransferPacket.h"
#include "SendItemContract.h"
#include "PickupConfirmationContract.h"
#include "DropConfirmationContract.h"
#include "GroundItemCreateContract.h"
#include "GroundItemStateContract.h"
#include "InstanceCounterContract.h"
#include "WorldStateParameterContract.h"
#include "HpMpContract.h"
#include "UpdateScoreContract.h"
#include "CarrySnapshotContract.h"
#include "UpdateEquipContract.h"
#include "UpdateAffectContract.h"
#include "CreateMobContract.h"
#include "ShopListContract.h"
#include "ItemSoldContract.h"
#include "PlayerChallengeContract.h"
#include "CombineCompleteContract.h"
#include "TradeSessionContract.h"
#include "TradeCheckConfirmationContract.h"
#include "WarInfoContract.h"
#include "ShortSkillSnapshotContract.h"
#include "ActionFrameContract.h"
#include "AttackFrameContract.h"
#include "InventoryTransactionContract.h"
#include "LegacySalePacket.h"
#include "CargoGoldTransferContract.h"
#include "AutoTradeContract.h"
#include "CapsuleInfoContract.h"
#include "PremiumFireworkPacket.h"
#include "GamblePacket.h"
#include "MessagePanelPacket.h"
#include "ShoutMessagePacket.h"
#include "ServerMigrationPacket.h"
#include "ChatMessagePacket.h"
#include "WhisperMessagePacket.h"
#include "CharacterLogoutConfirmPacket.h"
#include "CharacterLoginConfirmContract.h"
#include "LoginPacketContract.h"
#include "ClientIntegrityArrayContract.h"
#include "MobKillConfirmPacket.h"
#include "UpdateEtcPacket.h"
#include "DelayStartPacket.h"
#include "BillingNoticePacket.h"
#include "PartyAddPacket.h"
#include "PartyRemovePacket.h"
#include "PartyRequestPacket.h"
#include "MotionPacket.h"
#include "QuizEventPacket.h"
#include "InstanceQuestionPacket.h"
#include "PremiumShopPacket.h"
#include "IndexedMessageContract.h"
#include "LegacySceneMessagePacket.h"
#include "GuildChallengePromptPacket.h"
#include "WeatherPacket.h"
#include "KefraPortalPacket.h"
#include "../application/ports/PacketDispatch.h"
#include <cstring>

// Incremental boundary between transport frames and legacy callbacks.
// ExpectedSize lists migrated contracts; other opcodes remain subject to
// consumer-specific guards. Local events do not pass through this entry.
namespace received_packet
{
    // Zero identifies an unmigrated contract, never an empty frame.
    inline std::size_t ExpectedSize(unsigned int opcode)
    {
		const auto loginSize = LoginPacketExpectedSize(ClassifyLoginPacket(opcode));
		if (loginSize != 0)
			return loginSize;

        switch (opcode)
        {
        case quiz_event::ChallengeOpcode: return sizeof(quiz_event::Challenge);
        case instance_question::Opcode: return sizeof(instance_question::Packet);
        case premium_shop::StateOpcode: return sizeof(premium_shop::State);
        case kefra_portal::StateOpcode: return sizeof(kefra_portal::State);
        case MSG_ReqTransper_Opcode: return sizeof(MSG_ReqTransper);
        case MSG_SendItem_Opcode: return kSendItemPacketSize;
        case MSG_CNFGetItem_Opcode: return kPickupConfirmationPacketSize;
        case MSG_CNFDropItem_Opcode: return kDropConfirmationPacketSize;
        case MSG_CreateItem_Opcode: return kGroundItemCreatePacketSize;
        case MSG_UpdateItem_Opcode: return kGroundItemUpdatePacketSize;
        case MSG_RemoveItem_Opcode: return kGroundItemRemovePacketSize;
        case MSG_InstanceTime_Opcode:
        case MSG_InstanceMobs_Opcode: return kInstanceCounterPacketSize;
        case MSG_RemoveMob_Opcode:
        case MSG_UpdateCargoGold_Opcode: return kWorldStateParameterPacketSize;
        case MSG_SetHpMp_Opcode: return kHpMpPacketSize;
        case MSG_UpdateScore_Opcode: return kUpdateScorePacketSize;
        case MSG_UpdateCarry_Opcode: return kCarrySnapshotPacketSize;
        case MSG_UpdateEquip_Opcode: return kUpdateEquipPacketSize;
        case MSG_UpdateAffect_Opcode: return kUpdateAffectPacketSize;
        case MSG_CreateMob_Opcode: return kCreateMobPacketSize;
        case MSG_CreateMobTrade_Opcode: return kCreateMobTradePacketSize;
        case MSG_ShopList_Opcode: return kShopListPacketSize;
        case MSG_ItemSold_Opcode: return kItemSoldPacketSize;
        case MSG_PlayerChallenge_Opcode: return kPlayerChallengePacketSize;
        case MSG_CombineComplete_Opcode: return kCombineCompletePacketSize;
        case MSG_Trade_Opcode: return kTradePacketSize;
        case MSG_CloseTrade_Opcode: return kCloseTradePacketSize;
        case MSG_CNFTradeCheck_Opcode: return kTradeCheckConfirmationPacketSize;
        case MSG_WarInfo_Opcode: return kWarInfoPacketSize;
        case MSG_SetShortSkill_Opcode: return kShortSkillSnapshotPacketSize;
        case MSG_Action_Opcode:
        case MSG_Action_Stop_Opcode:
        case MSG_Action2_Opcode: return kActionPacketSize;
        case MSG_SwapItem_Opcode: return kSwapItemPacketSize;
        case MSG_Buy_Opcode: return kBuyPacketSize;
        case MSG_Sell_Opcode: return sizeof(MSG_Sell);
        case MSG_Withdraw_Opcode:
        case MSG_Deposit_Opcode: return kCargoGoldTransferPacketSize;
        case MSG_AutoTrade_Opcode: return kAutoTradePacketSize;
        case MSG_CapsuleInfo_Opcode: return kCapsuleInfoPacketSize;
        case MSG_PremiumFirework_Opcode: return sizeof(MSG_PremiumFirework);
        case MSG_ResultGamble_Opcode: return sizeof(MSG_ResultGamble);
        case MSG_MessagePanel_Opcode: return sizeof(MSG_MessagePanel);
        case MSG_MessageShout_Opcode: return sizeof(MSG_MessageShout);
        case MSG_LegacySceneMessage102_Opcode: return sizeof(MSG_LegacySceneMessage102);
        case MSG_LegacySceneMessage104_Opcode: return sizeof(MSG_LegacySceneMessage104);
        case MSG_MessageChat_Opcode: return sizeof(MSG_MessageChat);
        case MSG_MessageIndexed_Opcode:
        case MSG_MessageParameterized_Opcode: return kIndexedMessagePacketSize;
        case MSG_CNFRemoveServer_Opcode: return sizeof(MSG_CNFRemoveServer);
        case MSG_MessageWhisper_Opcode: return sizeof(MSG_MessageWhisper);
        case MSG_CNFCharacterLogout_Opcode: return sizeof(MSG_CNFCharacterLogout);
        case MSG_REQArray_Opcode: return sizeof(MSG_REQArray);
        case MSG_CNFMobKill_Opcode: return sizeof(MSG_CNFMobKill);
        case MSG_UpdateEtc_Opcode: return sizeof(MSG_UpdateEtc);
        case MSG_DelayStart_Opcode: return sizeof(MSG_DelayStart);
        case MSG_BillingNotice_Opcode: return sizeof(MSG_BillingNotice);
        case MSG_AddParty_Opcode: return sizeof(MSG_AddParty);
        case MSG_RemoveParty_Opcode: return sizeof(MSG_RemoveParty);
        case MSG_REQParty_Opcode: return sizeof(MSG_REQParty);
        case MSG_Motion_Opcode: return sizeof(MSG_Motion);
        case MSG_ReqChallenge_Opcode: return sizeof(MSG_STANDARD);
        case MSG_Weather_Opcode: return kWeatherPacketSize;
        default: return 0;
        }
    }

    // Unmigrated legacy consumers without a proven exact 7.48 size. The value
    // is the end of the last field each handler reads or rewrites in place, so a
    // shorter frame can no longer make it touch the next frame or the end of the
    // receive block. Zero means the consumer reads only the header or is
    // unlisted. Longer frames remain accepted: these are minimums, not contracts.
    inline std::size_t ConsumerMinimumSize(unsigned int opcode)
    {
        constexpr std::size_t header = sizeof(MSG_STANDARD);
        constexpr std::size_t oneParameter = header + sizeof(int);
        constexpr std::size_t twoParameters = header + 2 * sizeof(int);
        switch (opcode)
        {
        case 0x7B1: return header + 2 * sizeof(short) + 4 * 128;     // long message panel: Parm1, Parm2, Line[4][128]
        case 0x3B2: return header + sizeof(int) + 16;                // summon request: Result, Name[16]
        case 0x1D0: return kShopListPacketSize;                      // alternate shop list uses MSG_ShopList
        case 0x292: return header + sizeof(int) + sizeof(short);     // HP mode: Hp, Mode
        case 0x3A2: return header + 6 * sizeof(short);               // environment effect rectangle and effect
        case 0x2132: return header + 5 * sizeof(int);                // play information bonuses and cash
        case 0x457: return header + 5 * 16 + 5 * sizeof(int);        // tower war names and points
        case 0x755: return header + 3 * sizeof(int);                 // water scroll macro: Parm, PosX, PosY
        case 0x5000: return header + 128 + sizeof(int);              // experience panel: Msg[128], Color32
        case 0x18A: return header + sizeof(int) + sizeof(short);     // HP damage: Hp, Dam
        case 0x1D6: return header + sizeof(int) + 12;                // guild name: Parm, GuildName[12]
        case 0x3A3: case 0x3AC: case 0x3A4: case 0x3BB: case 0x3CF:
        case 0x3AF: case 0x1CF: case 0x193: case 0x3AD: case 0xADA:
            return oneParameter;
        case 0x1C2: case 0x3D0: case 0x3B4:
            return twoParameters;
        default: return 0;
        }
    }

    inline bool CanDispatch(const PacketView& packet)
    {
        if (!packet_dispatch::CanDispatch(packet, sizeof(MSG_STANDARD)))
            return false;

        // A view may start at an unaligned address. Copy only the header for
        // inspection; never write or copy the handlers' payload.
        MSG_STANDARD header{};
        std::memcpy(&header, packet.data, sizeof(header));

        if (header.Type == kefra_portal::StateOpcode || packet.opcode == kefra_portal::StateOpcode)
        {
            kefra_portal::State state{};
            return packet.opcode == header.Type &&
                kefra_portal::Parse(packet.data, packet.size, state);
        }

        // Attacks have native prefixes and variable-size coordinated extensions.
        // Validate both discriminants and the actual length before any cast in
        // OnPacketAttack.
        if (IsAttackOpcode(header.Type) || IsAttackOpcode(packet.opcode))
        {
            return packet.opcode == header.Type &&
                header.Size == packet.size &&
                IsAttackPacketSize(header.Type, packet.size);
        }

        const auto expected = ExpectedSize(header.Type);
        if (expected != 0 || ExpectedSize(packet.opcode) != 0)
        {
            // Check both discriminants to prevent mismatched metadata from
            // bypassing the guard: the legacy scene uses the buffer's Type.
            return packet.opcode == header.Type &&
                header.Size == expected && packet.size == expected;
        }

        const auto minimum = ConsumerMinimumSize(header.Type);
        if (minimum != 0 || ConsumerMinimumSize(packet.opcode) != 0)
        {
            return packet.opcode == header.Type &&
                header.Size == packet.size && packet.size >= minimum;
        }
        return true;
    }

    // True means one delivery, not success of the character operation.
    // The receiver borrows the same buffer and length only during the call.
    template <typename Receiver>
    bool Dispatch(const PacketView& packet, Receiver&& receive)
    {
        if (!CanDispatch(packet))
            return false;
        receive(packet);
        return true;
    }
}

#pragma once

struct MSG_Trade;
class SMessageBox;
class SGridControlItem;

// Field-scene trade ownership operations shared by dispatch and trade members.
// Pickup does not clear UI aliases; callers must retain the established order.
void WYD748_ResetTradeOffer(MSG_Trade& trade, unsigned short opponentID);
void WYD748_LogTradeSend(const char* origin, const MSG_Trade& trade);
void WYD748_CancelAutoTradePurchase(SMessageBox* dialog, int invalidatedSlot = -1);
void WYD748_ReleaseAutoTradeItem(SGridControlItem*& pItem);

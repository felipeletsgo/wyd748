// Exercises the same boundary that ObjectManager calls, without UI/socket/DirectX.
// Known bytes are independent of the C++ constructor so the wire contract is checked.
// The cases are grouped by packet domain in ReceivedPacketDispatch*Tests.cpp.
int RunReceivedPacketDispatchSelectionTests(int& checks);
int RunReceivedPacketDispatchGroundItemsTests(int& checks);
int RunReceivedPacketDispatchWorldStateTests(int& checks);
int RunReceivedPacketDispatchEntitiesAndShopTests(int& checks);
int RunReceivedPacketDispatchTradeTests(int& checks);
int RunReceivedPacketDispatchActionsAndInventoryTests(int& checks);
int RunReceivedPacketDispatchFeaturesTests(int& checks);
int RunReceivedPacketDispatchMessagesAndServerListTests(int& checks);
int RunReceivedPacketDispatchSessionAndPartyTests(int& checks);
int RunReceivedPacketDispatchRequestsAndEventsTests(int& checks);

int RunReceivedPacketDispatchTests(int& checks)
{
    int failures = 0;
    failures += RunReceivedPacketDispatchSelectionTests(checks);
    failures += RunReceivedPacketDispatchGroundItemsTests(checks);
    failures += RunReceivedPacketDispatchWorldStateTests(checks);
    failures += RunReceivedPacketDispatchEntitiesAndShopTests(checks);
    failures += RunReceivedPacketDispatchTradeTests(checks);
    failures += RunReceivedPacketDispatchActionsAndInventoryTests(checks);
    failures += RunReceivedPacketDispatchFeaturesTests(checks);
    failures += RunReceivedPacketDispatchMessagesAndServerListTests(checks);
    failures += RunReceivedPacketDispatchSessionAndPartyTests(checks);
    failures += RunReceivedPacketDispatchRequestsAndEventsTests(checks);
    return failures;
}

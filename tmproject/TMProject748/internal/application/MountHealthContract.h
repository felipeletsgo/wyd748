#pragma once
namespace mount_health {
// Snapshot of wydgo748/data/content/mounts.json, checked by the Go contract test.
inline constexpr int MaximumByType[30] = {10000,12000,14000,16000,18000,20000,22000,22000,24000,24000,26000,22000,22000,24000,24000,26000,28000,28000,28000,30000,30000,28000,28000,28000,30000,30000,30000,30000,30000,30000};
inline int Maximum(int item) {
 return item >= 2360 && item < 2390 ? MaximumByType[item - 2360] : 0;
}

// An HP-only snapshot of the same living adult cannot change its appearance.
// Death, revival, level, food, longevity and item changes use the full path.
template<class Item>
bool IsLiveHPUpdate(const Item& previous, const Item& next) {
 return Maximum(previous.sIndex) > 0 && previous.sIndex == next.sIndex &&
  static_cast<unsigned short>(previous.stEffect[0].sValue) > 0 &&
  static_cast<unsigned short>(next.stEffect[0].sValue) > 0 &&
  previous.stEffect[1].sValue == next.stEffect[1].sValue &&
  previous.stEffect[2].sValue == next.stEffect[2].sValue;
}
}

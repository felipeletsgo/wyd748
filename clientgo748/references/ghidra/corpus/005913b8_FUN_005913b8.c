// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 005913b8 | Name: FUN_005913b8


void FUN_005913b8(undefined4 *param_1)

{
  DWORD DVar1;
  undefined4 uVar2;
  _TIME_ZONE_INFORMATION local_d0;
  _SYSTEMTIME local_24;
  _SYSTEMTIME local_14;
  
  GetLocalTime(&local_14);
  GetSystemTime(&local_24);
  if (local_24.wMinute == DAT_013b7fd8._2_2_) {
    if (local_24.wHour == (WORD)DAT_013b7fd8) {
      if (local_24.wDay == DAT_013b7fd4._2_2_) {
        if (local_24.wMonth == DAT_013b7fd0._2_2_) {
          if (local_24.wYear == (WORD)DAT_013b7fd0) goto LAB_00591462;
        }
      }
    }
  }
  DVar1 = GetTimeZoneInformation(&local_d0);
  if (DVar1 == 0xffffffff) {
    DAT_013b7fc8 = 0xffffffff;
  }
  else if (((DVar1 == 2) && (local_d0.DaylightDate.wMonth != 0)) && (local_d0.DaylightBias != 0)) {
    DAT_013b7fc8 = 1;
  }
  else {
    DAT_013b7fc8 = 0;
  }
  DAT_013b7fd0._0_2_ = local_24.wYear;
  DAT_013b7fd0._2_2_ = local_24.wMonth;
  DAT_013b7fd4._0_2_ = local_24.wDayOfWeek;
  DAT_013b7fd4._2_2_ = local_24.wDay;
  DAT_013b7fd8._0_2_ = local_24.wHour;
  DAT_013b7fd8._2_2_ = local_24.wMinute;
  DAT_013b7fdc._0_2_ = local_24.wSecond;
  DAT_013b7fdc._2_2_ = local_24.wMilliseconds;
LAB_00591462:
  uVar2 = FUN_00596e0b(local_14.wYear,local_14.wMonth,local_14.wDay,local_14.wHour,local_14.wMinute,
                       local_14.wSecond,DAT_013b7fc8);
  if (param_1 != (undefined4 *)0x0) {
    *param_1 = uVar2;
  }
  return;
}


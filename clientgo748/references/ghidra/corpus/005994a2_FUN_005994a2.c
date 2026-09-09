// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 005994a2 | Name: FUN_005994a2


SIZE_T FUN_005994a2(LPCVOID param_1)

{
  byte *pbVar1;
  SIZE_T SVar2;
  undefined1 local_30 [4];
  byte *local_2c;
  undefined1 local_28 [4];
  SIZE_T local_24;
  byte *local_20;
  void *local_14;
  undefined1 *puStack_10;
  undefined *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &DAT_005aa350;
  puStack_10 = &LAB_00596b2c;
  local_14 = ExceptionList;
  if (DAT_013b8608 == 3) {
    ExceptionList = &local_14;
    FUN_00596aad(9);
    local_8 = 0;
    local_20 = (byte *)FUN_005958b8(param_1);
    if (local_20 != (byte *)0x0) {
      local_24 = *(int *)((int)param_1 + -4) - 9;
    }
    SVar2 = local_24;
    local_8 = 0xffffffff;
    FUN_0059950c();
    pbVar1 = local_20;
  }
  else {
    ExceptionList = &local_14;
    if (DAT_013b8608 != 2) goto LAB_00599560;
    ExceptionList = &local_14;
    FUN_00596aad(9);
    local_8 = 1;
    local_2c = (byte *)FUN_00596613(param_1,local_30,local_28);
    if (local_2c != (byte *)0x0) {
      local_24 = (uint)*local_2c << 4;
    }
    SVar2 = local_24;
    local_8 = 0xffffffff;
    FUN_00599587();
    pbVar1 = local_2c;
  }
  if (pbVar1 != (byte *)0x0) {
    ExceptionList = local_14;
    return SVar2;
  }
LAB_00599560:
  SVar2 = HeapSize(DAT_013b8604,0,param_1);
  ExceptionList = local_14;
  return SVar2;
}


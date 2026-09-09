// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0044fc4b | Name: FUN_0044fc4b


void __fastcall FUN_0044fc4b(int param_1)

{
  short sVar1;
  ushort uVar2;
  short sVar3;
  ushort uVar4;
  int iVar5;
  undefined4 uVar6;
  undefined4 uVar7;
  short local_50;
  undefined1 local_44 [4];
  undefined2 local_40;
  undefined2 local_3e;
  undefined4 local_38;
  int local_34;
  undefined2 local_28;
  undefined2 local_26;
  undefined2 local_24;
  int local_20;
  undefined4 local_1c;
  int local_18;
  int local_14;
  int local_10;
  int local_c;
  int *local_8;
  
  if (*(short *)(DAT_013b71e8 + 0x7a8) != 0) {
    local_8 = *(int **)(param_1 + 0x2873c);
    local_18 = 0;
    local_14 = 0;
    local_c = 0;
    for (local_10 = 8; -1 < local_10; local_10 = local_10 + -1) {
      for (local_14 = 6; -1 < local_14; local_14 = local_14 + -1) {
        local_18 = (**(code **)(*local_8 + 0xb4))(local_10,local_14);
        if (((local_18 != 0) && (*(int *)(local_18 + 0x670) != 0)) &&
           (iVar5 = FUN_0054cd07(*(undefined4 *)(local_18 + 0x670),0x26), iVar5 == 0x11)) {
          local_c = 1;
          break;
        }
      }
      if (local_c == 1) break;
    }
    if (((local_c == 1) && (local_18 != 0)) &&
       ((local_20 = (**(code **)(*DAT_0092e654 + 8))(), *(int *)(param_1 + 0x27478) == 0 ||
        (199 < (uint)(local_20 - *(int *)(param_1 + 0x27478)))))) {
      iVar5 = DAT_013b71e8 + 0x7a8;
      sVar1 = FUN_0054cd07(iVar5,0x38);
      uVar2 = FUN_0054cd07(iVar5,0x39);
      sVar3 = FUN_0054cd07(*(undefined4 *)(local_18 + 0x670),0x38);
      uVar4 = FUN_0054cd07(*(undefined4 *)(local_18 + 0x670),0x39);
      if ((ushort)(sVar1 << 8 | uVar2) == (ushort)(sVar3 << 8 | uVar4)) {
        (**(code **)(*local_8 + 0xc0))
                  (*(undefined4 *)(*(int *)(local_18 + 0x1e4) + 0x1e4),
                   *(undefined4 *)(*(int *)(local_18 + 0x1e4) + 0x400));
        local_50 = (**(code **)(*local_8 + 0xc4))
                             (*(undefined4 *)(*(int *)(local_18 + 0x1e4) + 0x1e4));
        if (local_50 == -1) {
          local_50 = (short)*(undefined4 *)(local_18 + 0x1d0) +
                     (short)*(undefined4 *)(local_18 + 0x1d4) * 9;
        }
        _memset(local_44,0,0x24);
        local_3e = *(undefined2 *)(DAT_013b71e8 + 0xc5c);
        local_40 = 0x373;
        local_38 = 1;
        local_34 = (int)local_50;
        local_24 = 0;
        local_28 = __ftol();
        local_26 = __ftol();
        FUN_0055f2dd(local_44,0x24);
        *(int *)(param_1 + 0x27478) = local_20;
        *(undefined4 *)(DAT_005d0504 + 0x43c) = 1;
        local_1c = 0x36;
        if ((DAT_005ccf98 != 0) && (iVar5 = FUN_00429a6d(0x36), iVar5 != 0)) {
          uVar7 = 0;
          uVar6 = 0;
          FUN_00429a6d(local_1c);
          FUN_0042ad2b(uVar6,uVar7);
        }
      }
    }
  }
  return;
}


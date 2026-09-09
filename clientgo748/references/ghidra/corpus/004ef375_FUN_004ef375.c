// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004ef375 | Name: FUN_004ef375


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __fastcall FUN_004ef375(int param_1)

{
  undefined4 uVar1;
  undefined4 *puVar2;
  undefined1 local_54 [12];
  float local_48;
  float local_44;
  float local_40;
  undefined4 local_3c;
  undefined4 local_38;
  undefined4 local_34;
  int local_24;
  float local_20;
  float local_1c;
  undefined4 local_18;
  undefined4 local_14;
  float local_10;
  float local_c;
  float local_8;
  
  if (DAT_005d03f0 == 1) {
    uVar1 = 0;
  }
  else if (*(int *)(param_1 + 0x164) == 1) {
    uVar1 = 1;
  }
  else {
    FUN_00430f50(*(undefined4 *)(*(int *)(DAT_013b71e8 + 0x1b084) + 0x48),
                 *(undefined4 *)(*(int *)(DAT_013b71e8 + 0x1b084) + 0x4c),
                 *(undefined4 *)(*(int *)(DAT_013b71e8 + 0x1b084) + 0x50));
    FUN_00430f10();
    puVar2 = (undefined4 *)FUN_004b3a50(local_54,param_1 + 0xe4);
    local_3c = *puVar2;
    local_38 = puVar2[1];
    local_34 = puVar2[2];
    local_14 = *(undefined4 *)(param_1 + 0xe8);
    FUN_00430f10();
    thunk_FUN_005614e8(&local_10,&local_3c,DAT_005ccf9c + 0xa973,DAT_005ccf9c + 0xa97b,
                       DAT_005ccf9c + 0xa98b,DAT_005ccf9c + 0xa99b);
    FUN_00430f10();
    if (local_8 <= _DAT_005a3660) {
      local_20 = (float)((uint)DAT_005ccf9c[0xa975] >> 1) - local_10;
      local_1c = (float)((uint)DAT_005ccf9c[0xa976] >> 1) - local_c;
      local_18 = 0;
      (**(code **)(*DAT_005ccf9c + 0x30))(0x1b,1);
      (**(code **)(*DAT_005ccf9c + 0x34))(0,4,2);
      (**(code **)(*DAT_005ccf9c + 0x30))(0x1c,0);
      (**(code **)(*DAT_005ccf9c + 0x30))(0x89,0);
      (**(code **)(*DAT_005ccf9c + 0x30))(0x14,7);
      (**(code **)(*DAT_005ccf9c + 0x30))(0x13,3);
      (**(code **)(*DAT_005ccf9c + 0x30))(0x19,8);
      (**(code **)(*DAT_005ccf9c + 0x30))(0xf,0);
      (**(code **)(*DAT_005ccf9c + 0x30))(0xe,0);
      *(float *)(param_1 + 0xe8) = *(float *)(param_1 + 0x168) * _DAT_005a4160;
      for (local_24 = 0; local_24 < 0xc; local_24 = local_24 + 1) {
        local_40 = local_20 * *(float *)(param_1 + 0x28 + local_24 * 0x10) + local_10;
        local_48 = local_1c * *(float *)(param_1 + 0x28 + local_24 * 0x10) + local_c;
        local_44 = *(float *)(param_1 + 0x2c + local_24 * 0x10);
        *(float *)(param_1 + 0xf0) = local_40 - local_44 * *(float *)(param_1 + 0x168);
        *(float *)(param_1 + 0xf4) = local_48 - local_44 * *(float *)(param_1 + 0x168);
        *(float *)(param_1 + 0xf8) = _DAT_005a415c - (float)local_24 * _DAT_005a4740;
        *(undefined4 *)(param_1 + 0x100) = *(undefined4 *)(param_1 + 0x30 + local_24 * 0x10);
        *(float *)(param_1 + 0x10c) = local_44 * *(float *)(param_1 + 0x168) + local_40;
        *(float *)(param_1 + 0x110) = local_48 - local_44 * *(float *)(param_1 + 0x168);
        *(float *)(param_1 + 0x114) = _DAT_005a415c - (float)local_24 * _DAT_005a4740;
        *(undefined4 *)(param_1 + 0x11c) = *(undefined4 *)(param_1 + 0x30 + local_24 * 0x10);
        *(float *)(param_1 + 0x128) = local_44 * *(float *)(param_1 + 0x168) + local_40;
        *(float *)(param_1 + 300) = local_44 * *(float *)(param_1 + 0x168) + local_48;
        *(float *)(param_1 + 0x130) = _DAT_005a415c - (float)local_24 * _DAT_005a4740;
        *(undefined4 *)(param_1 + 0x138) = *(undefined4 *)(param_1 + 0x30 + local_24 * 0x10);
        *(float *)(param_1 + 0x144) = local_40 - local_44 * *(float *)(param_1 + 0x168);
        *(float *)(param_1 + 0x148) = local_44 * *(float *)(param_1 + 0x168) + local_48;
        *(float *)(param_1 + 0x14c) = _DAT_005a415c - (float)local_24 * _DAT_005a4740;
        *(undefined4 *)(param_1 + 0x154) = *(undefined4 *)(param_1 + 0x30 + local_24 * 0x10);
        uVar1 = FUN_004b925c(*(undefined4 *)(param_1 + 0x24 + local_24 * 0x10),360000);
        (**(code **)(*DAT_005ccf9c + 0x38))(0,uVar1);
        (**(code **)(*(int *)DAT_005ccf9c[0xa8f9] + 0x130))(DAT_005ccf9c[0xa8f9],0x144);
        (**(code **)(*(int *)DAT_005ccf9c[0xa8f9] + 0x120))
                  (DAT_005ccf9c[0xa8f9],6,2,param_1 + 0xf0,0x1c);
      }
      uVar1 = 1;
    }
    else {
      uVar1 = 1;
    }
  }
  return uVar1;
}


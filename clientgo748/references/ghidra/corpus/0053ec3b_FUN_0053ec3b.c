// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0053ec3b | Name: FUN_0053ec3b


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

int __fastcall FUN_0053ec3b(int param_1)

{
  float fVar1;
  float fVar2;
  undefined4 *puVar3;
  undefined4 uVar4;
  int iVar5;
  int xRight;
  int yTop;
  int xLeft;
  BOOL BVar6;
  float10 fVar7;
  undefined1 local_288 [24];
  undefined1 local_270 [36];
  undefined4 local_24c;
  tagRECT local_248;
  tagRECT local_238;
  tagRECT local_228;
  undefined1 local_218 [64];
  undefined1 local_1d8 [48];
  float local_1a8;
  float local_1a4;
  float local_1a0;
  undefined1 local_198 [64];
  undefined1 local_158 [64];
  float local_118;
  int local_114;
  int local_110;
  float local_100 [27];
  int local_94;
  int local_90;
  undefined1 local_8c [8];
  float afStack_84 [25];
  int local_20;
  undefined4 local_1c;
  undefined4 local_18;
  undefined4 local_14;
  int local_10;
  int local_c;
  undefined4 local_8;
  
  local_94 = *(int *)(DAT_013b71e8 + 0x1b084);
  local_114 = 0;
  local_110 = FUN_004b14fd(*(undefined4 *)(param_1 + 0x24),0,180000);
  if (local_110 == 0) {
    local_114 = 0;
  }
  else {
    FUN_00430f10();
    FUN_00421910(local_8c,0xc,9,FUN_00430f10);
    FUN_00421910(local_100,0xc,9,FUN_00430f10);
    FUN_00430f10();
    puVar3 = (undefined4 *)
             FUN_00430f50(*(undefined4 *)(param_1 + 0x28),0,*(undefined4 *)(param_1 + 0x2c));
    local_1c = *puVar3;
    local_18 = puVar3[1];
    local_14 = puVar3[2];
    fVar1 = *(float *)(DAT_005ccf9c + 0x2b048);
    fVar2 = *(float *)(local_110 + 0x340);
    uVar4 = FUN_00430f50(*(undefined4 *)(local_94 + 0x48),0,*(undefined4 *)(local_94 + 0x50));
    uVar4 = FUN_004d3bc0(local_270,uVar4);
    fVar7 = (float10)FUN_004d3c90(uVar4);
    if (fVar7 <= (float10)(fVar1 + fVar2)) {
      for (local_90 = 0; local_90 < 9; local_90 = local_90 + 1) {
        local_100[local_90 * 3] = *(float *)(param_1 + 0x28);
        local_100[local_90 * 3 + 2] = *(float *)(param_1 + 0x2c);
        local_100[local_90 * 3 + 1] = *(float *)(param_1 + 0x30);
      }
      if (*(int *)(param_1 + 0x24) == 3) {
        local_118 = *(float *)(local_110 + 0x340);
        local_100[3] = local_100[0x15] -
                       local_118 * *(float *)(local_110 + 0x32c) * *(float *)(param_1 + 0x38);
        local_100[5] = local_118 * *(float *)(local_110 + 0x32c) * *(float *)(param_1 + 0x38) +
                       local_100[0x14];
        local_100[4] = local_100[0xd] -
                       local_118 * *(float *)(local_110 + 0x330) * *(float *)(param_1 + 0x38);
        local_100[6] = local_118 * *(float *)(local_110 + 0x32c) * *(float *)(param_1 + 0x38) +
                       local_100[0x18];
        local_100[0xb] =
             local_100[0x1a] -
             local_118 * *(float *)(local_110 + 0x32c) * *(float *)(param_1 + 0x38);
        local_100[0x10] =
             local_118 * *(float *)(local_110 + 0x330) * *(float *)(param_1 + 0x38) +
             local_100[0x19];
        local_100[7] = local_100[4];
        local_100[8] = local_100[5];
        local_100[9] = local_100[3];
        local_100[10] = local_100[4];
        local_100[0xc] = local_100[6];
        local_100[0xd] = local_100[4];
        local_100[0xe] = local_100[0xb];
        local_100[0xf] = local_100[3];
        local_100[0x11] = local_100[5];
        local_100[0x12] = local_100[6];
        local_100[0x13] = local_100[0x10];
        local_100[0x14] = local_100[5];
        local_100[0x15] = local_100[3];
        local_100[0x16] = local_100[0x10];
        local_100[0x17] = local_100[0xb];
        local_100[0x18] = local_100[6];
        local_100[0x19] = local_100[0x10];
        local_100[0x1a] = local_100[0xb];
      }
      else {
        local_100[3] = local_100[0x15] + *(float *)(local_110 + 0x348);
        local_100[5] = -*(float *)(local_110 + 0x350) + local_100[0x14];
        local_100[4] = local_100[0xd] + *(float *)(local_110 + 0x358);
        local_100[6] = local_100[0x18] + *(float *)(local_110 + 0x344);
        local_100[0xb] = -*(float *)(local_110 + 0x34c) + local_100[0x1a];
        local_100[0x10] = local_100[0x19] + *(float *)(local_110 + 0x354);
        local_100[7] = local_100[4];
        local_100[8] = local_100[5];
        local_100[9] = local_100[3];
        local_100[10] = local_100[4];
        local_100[0xc] = local_100[6];
        local_100[0xd] = local_100[4];
        local_100[0xe] = local_100[0xb];
        local_100[0xf] = local_100[3];
        local_100[0x11] = local_100[5];
        local_100[0x12] = local_100[6];
        local_100[0x13] = local_100[0x10];
        local_100[0x14] = local_100[5];
        local_100[0x15] = local_100[3];
        local_100[0x16] = local_100[0x10];
        local_100[0x17] = local_100[0xb];
        local_100[0x18] = local_100[6];
        local_100[0x19] = local_100[0x10];
        local_100[0x1a] = local_100[0xb];
        FUN_00430f10();
        FUN_00430f10();
        FUN_00430f10();
        FUN_00430f10();
        thunk_FUN_0056230d(local_218,*(float *)(local_110 + 0x32c) * *(float *)(param_1 + 0x38),
                           *(float *)(local_110 + 0x330) * *(float *)(param_1 + 0x38),
                           *(float *)(local_110 + 0x32c) * *(float *)(param_1 + 0x38));
        thunk_FUN_00562821(local_158,*(undefined4 *)(param_1 + 0x34),0xbfc90fdb,0);
        thunk_FUN_00561a5b(local_158,DAT_005ccf9c + 0x2a66c,local_158);
        thunk_FUN_00561a5b(local_158,local_158,local_218);
        for (local_90 = 0; local_90 < 9; local_90 = local_90 + 1) {
          thunk_FUN_0056239d(local_198,local_100[local_90 * 3],local_100[local_90 * 3 + 1],
                             local_100[local_90 * 3 + 2]);
          thunk_FUN_00561a5b(local_1d8,local_158,local_198);
          local_100[local_90 * 3] = local_1a8;
          local_100[local_90 * 3 + 1] = local_1a4;
          local_100[local_90 * 3 + 2] = local_1a0;
        }
      }
      local_10 = 0;
      local_8 = 0;
      for (local_90 = 0; local_90 < 9; local_90 = local_90 + 1) {
        fVar1 = *(float *)(DAT_005ccf9c + 0x2b048) + _DAT_005a40f0;
        uVar4 = FUN_00430f50(*(undefined4 *)(local_94 + 0x48),*(undefined4 *)(local_94 + 0x4c),
                             *(undefined4 *)(local_94 + 0x50));
        uVar4 = FUN_004d3bc0(local_288,uVar4);
        fVar7 = (float10)FUN_004d3c90(uVar4);
        if (fVar7 < (float10)fVar1) {
          thunk_FUN_005614e8(local_8c + local_90 * 0xc,local_100 + local_90 * 3,
                             DAT_005ccf9c + 0x2a5cc,DAT_005ccf9c + 0x2a5ec,DAT_005ccf9c + 0x2a62c,
                             DAT_005ccf9c + 0x2a66c);
          if ((_DAT_005a4738 <= afStack_84[local_90 * 3]) &&
             (afStack_84[local_90 * 3] < _DAT_005a3660)) {
            local_c = __ftol();
            local_20 = __ftol();
            if ((0 < local_c) &&
               (((local_c < *(int *)(DAT_005ccf9c + 0x2a5d4) && (0 < local_20)) &&
                (local_20 < *(int *)(DAT_005ccf9c + 0x2a5d8))))) {
              return 1;
            }
          }
          if (((local_114 == 0) && (local_90 == 0)) && (afStack_84[0] < _DAT_005a3660)) {
            local_10 = 1;
          }
        }
      }
      if ((local_114 == 0) && (local_10 == 1)) {
        iVar5 = __ftol();
        xRight = __ftol();
        yTop = __ftol();
        xLeft = __ftol();
        SetRect(&local_248,xLeft,yTop,xRight,iVar5);
        SetRect(&local_228,0,0,*(int *)(DAT_005ccf9c + 0x2a5d4),*(int *)(DAT_005ccf9c + 0x2a5d8));
        local_24c = 0;
        for (local_90 = 1; local_90 < 9; local_90 = local_90 + 1) {
          iVar5 = __ftol();
          if (iVar5 < local_248.left) {
            local_248.left = __ftol();
          }
          iVar5 = __ftol();
          if (iVar5 < local_248.top) {
            local_248.top = __ftol();
          }
          iVar5 = __ftol();
          if (local_248.right < iVar5) {
            local_248.right = __ftol();
          }
          iVar5 = __ftol();
          if (local_248.bottom < iVar5) {
            local_248.bottom = __ftol();
          }
        }
        BVar6 = IntersectRect(&local_238,&local_248,&local_228);
        if (BVar6 == 1) {
          local_114 = 1;
        }
        else {
          local_114 = 0;
        }
      }
    }
    else {
      local_114 = 0;
    }
  }
  return local_114;
}


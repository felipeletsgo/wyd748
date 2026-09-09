// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004d2509 | Name: FUN_004d2509


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

int __fastcall FUN_004d2509(int param_1)

{
  undefined4 uVar1;
  int iVar2;
  int xRight;
  int yTop;
  int xLeft;
  BOOL BVar3;
  float10 fVar4;
  float local_260;
  float local_25c;
  undefined1 local_254 [24];
  undefined4 local_23c;
  tagRECT local_238;
  tagRECT local_228;
  tagRECT local_218;
  int local_208;
  undefined1 local_204 [64];
  int local_1c4;
  float local_1b4 [27];
  int local_148;
  int local_144;
  undefined1 local_140 [8];
  float afStack_138 [25];
  int local_d4;
  int local_d0;
  int local_cc;
  undefined1 local_c8 [48];
  float local_98;
  float local_94;
  float local_90;
  undefined1 local_88 [64];
  undefined1 local_48 [64];
  undefined4 local_8;
  
  local_148 = *(int *)(DAT_013b71e8 + 0x1b084);
  local_208 = 0;
  local_1c4 = FUN_004b14fd(*(undefined4 *)(param_1 + 0x84),0,180000);
  if (local_1c4 == 0) {
    local_208 = 0;
  }
  else {
    FUN_00430f10();
    FUN_00421910(local_140,0xc,9,FUN_00430f10);
    FUN_00421910(local_1b4,0xc,9,FUN_00430f10);
    for (local_144 = 0; local_144 < 9; local_144 = local_144 + 1) {
      local_1b4[local_144 * 3] = *(float *)(param_1 + 0x74);
      local_1b4[local_144 * 3 + 2] = *(float *)(param_1 + 0x7c);
      local_1b4[local_144 * 3 + 1] = *(float *)(param_1 + 0x78);
    }
    local_1b4[3] = local_1b4[0x15] + *(float *)(local_1c4 + 0x348);
    local_1b4[5] = -*(float *)(local_1c4 + 0x350) + local_1b4[0x14];
    local_1b4[4] = local_1b4[0xd] + *(float *)(local_1c4 + 0x358);
    local_1b4[6] = local_1b4[0x18] + *(float *)(local_1c4 + 0x344);
    local_1b4[0xb] = -*(float *)(local_1c4 + 0x34c) + local_1b4[0x1a];
    local_1b4[0x10] = local_1b4[0x19] + *(float *)(local_1c4 + 0x354);
    local_1b4[7] = local_1b4[4];
    local_1b4[8] = local_1b4[5];
    local_1b4[9] = local_1b4[3];
    local_1b4[10] = local_1b4[4];
    local_1b4[0xc] = local_1b4[6];
    local_1b4[0xd] = local_1b4[4];
    local_1b4[0xe] = local_1b4[0xb];
    local_1b4[0xf] = local_1b4[3];
    local_1b4[0x11] = local_1b4[5];
    local_1b4[0x12] = local_1b4[6];
    local_1b4[0x13] = local_1b4[0x10];
    local_1b4[0x14] = local_1b4[5];
    local_1b4[0x15] = local_1b4[3];
    local_1b4[0x16] = local_1b4[0x10];
    local_1b4[0x17] = local_1b4[0xb];
    local_1b4[0x18] = local_1b4[6];
    local_1b4[0x19] = local_1b4[0x10];
    local_1b4[0x1a] = local_1b4[0xb];
    FUN_00430f10();
    FUN_00430f10();
    FUN_00430f10();
    FUN_00430f10();
    thunk_FUN_0056230d(local_204,*(undefined4 *)(local_1c4 + 0x32c),
                       *(undefined4 *)(local_1c4 + 0x330),*(undefined4 *)(local_1c4 + 0x32c));
    thunk_FUN_00562821(local_48,*(undefined4 *)(param_1 + 0x8c),0xbfc90fdb,0);
    thunk_FUN_00561a5b(local_48,DAT_005ccf9c + 0x2a66c,local_48);
    thunk_FUN_00561a5b(local_48,local_48,local_204);
    for (local_144 = 0; local_144 < 9; local_144 = local_144 + 1) {
      thunk_FUN_0056239d(local_88,local_1b4[local_144 * 3],local_1b4[local_144 * 3 + 1],
                         local_1b4[local_144 * 3 + 2]);
      thunk_FUN_00561a5b(local_c8,local_48,local_88);
      local_1b4[local_144 * 3] = local_98;
      local_1b4[local_144 * 3 + 1] = local_94;
      local_1b4[local_144 * 3 + 2] = local_90;
    }
    local_d0 = 0;
    local_8 = 0;
    for (local_144 = 0; local_144 < 9; local_144 = local_144 + 1) {
      uVar1 = FUN_00430f50(*(undefined4 *)(local_148 + 0x48),*(undefined4 *)(local_148 + 0x4c),
                           *(undefined4 *)(local_148 + 0x50));
      uVar1 = FUN_004d3bc0(local_254,uVar1);
      fVar4 = (float10)FUN_004d3c90(uVar1);
      if (fVar4 < (float10)*(float *)(DAT_005ccf9c + 0x2b048)) {
        thunk_FUN_005614e8(local_140 + local_144 * 0xc,local_1b4 + local_144 * 3,
                           DAT_005ccf9c + 0x2a5cc,DAT_005ccf9c + 0x2a5ec,DAT_005ccf9c + 0x2a62c,
                           DAT_005ccf9c + 0x2a66c);
        if ((_DAT_005a4738 <= afStack_138[local_144 * 3]) &&
           (afStack_138[local_144 * 3] < _DAT_005a3660)) {
          local_cc = __ftol();
          local_d4 = __ftol();
          if ((0 < local_cc) &&
             (((local_cc < *(int *)(DAT_005ccf9c + 0x2a5d4) && (0 < local_d4)) &&
              (local_d4 < *(int *)(DAT_005ccf9c + 0x2a5d8))))) {
            return 1;
          }
        }
        if (((local_208 == 0) && (local_144 == 0)) && (afStack_138[0] < _DAT_005a3660)) {
          local_d0 = 1;
        }
      }
    }
    if ((local_208 == 0) && (local_d0 == 1)) {
      if (*(float *)(param_1 + 0x74) - *(float *)(local_148 + 0x48) <= _DAT_005a3534) {
        local_25c = -(*(float *)(param_1 + 0x74) - *(float *)(local_148 + 0x48));
      }
      else {
        local_25c = *(float *)(param_1 + 0x74) - *(float *)(local_148 + 0x48);
      }
      if (local_25c < _DAT_005a3658) {
        if (*(float *)(param_1 + 0x78) - *(float *)(local_148 + 0x50) <= _DAT_005a3534) {
          local_260 = -(*(float *)(param_1 + 0x78) - *(float *)(local_148 + 0x50));
        }
        else {
          local_260 = *(float *)(param_1 + 0x78) - *(float *)(local_148 + 0x50);
        }
        if (local_260 < _DAT_005a3658) {
          iVar2 = __ftol();
          xRight = __ftol();
          yTop = __ftol();
          xLeft = __ftol();
          SetRect(&local_238,xLeft,yTop,xRight,iVar2);
          SetRect(&local_218,0,0,*(int *)(DAT_005ccf9c + 0x2a5d4),*(int *)(DAT_005ccf9c + 0x2a5d8));
          local_23c = 0;
          for (local_144 = 1; local_144 < 9; local_144 = local_144 + 1) {
            iVar2 = __ftol();
            if (iVar2 < local_238.left) {
              local_238.left = __ftol();
            }
            iVar2 = __ftol();
            if (iVar2 < local_238.top) {
              local_238.top = __ftol();
            }
            iVar2 = __ftol();
            if (local_238.right < iVar2) {
              local_238.right = __ftol();
            }
            iVar2 = __ftol();
            if (local_238.bottom < iVar2) {
              local_238.bottom = __ftol();
            }
          }
          BVar3 = IntersectRect(&local_228,&local_238,&local_218);
          if (BVar3 == 1) {
            return 1;
          }
          return 0;
        }
      }
    }
    if (local_208 == 0) {
      local_208 = 0;
    }
  }
  return local_208;
}


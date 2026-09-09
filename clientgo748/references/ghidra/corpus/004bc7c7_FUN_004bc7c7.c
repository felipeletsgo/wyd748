// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004bc7c7 | Name: FUN_004bc7c7


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __thiscall FUN_004bc7c7(int param_1,char *param_2)

{
  size_t sVar1;
  undefined4 uVar2;
  float local_dc;
  float local_d8;
  float local_d4;
  float local_d0;
  float local_cc;
  float local_c8;
  float local_c4;
  float local_c0;
  float local_bc;
  float local_b8;
  float local_b4;
  float local_b0;
  undefined1 local_9c [64];
  undefined1 local_5c [6];
  char local_56;
  undefined1 local_55 [5];
  undefined4 local_50;
  int local_4c;
  uint local_48;
  int local_34 [5];
  uint local_20 [2];
  uint local_18;
  int local_14;
  int local_10;
  uint local_c;
  int local_8;
  
  local_8 = 0;
  sVar1 = _strlen(param_2);
  if (sVar1 == 0) {
    uVar2 = 0;
  }
  else {
    local_8 = FUN_0058f716(param_2,&DAT_005baa38);
    if (local_8 == 0) {
      FUN_00431159(s_Reading_Error____s_005baa74,param_2);
      uVar2 = 0;
    }
    else {
      FUN_00590d68(param_1 + 0x10,4,1,local_8);
      FUN_00590d68(param_1 + 0xc,4,1,local_8);
      FUN_00590d68(param_1 + 0x14,4,1,local_8);
      FUN_00590d68(param_1 + 0x18,0x14,*(undefined4 *)(param_1 + 0x14),local_8);
      for (local_48 = 0; local_48 < *(uint *)(param_1 + 0x14); local_48 = local_48 + 1) {
        _memset(local_5c,0,0xc);
        FUN_00590d68(local_5c,0xb,1,local_8);
        if (*(int *)(param_1 + 4) == 1) {
          if (local_56 == '.') {
            FUN_0058f078(local_55,&PTR_DAT_005baa3c);
            FUN_0058f078(local_9c,s_Effect__s_005baa40,local_5c);
          }
          else {
            FUN_0058f078(local_9c,s_Effect__s_wyt_005baa4c,local_5c);
          }
          uVar2 = FUN_004b92e0(local_9c);
          *(undefined4 *)(param_1 + 0x298 + local_48 * 4) = uVar2;
        }
        else {
          if (local_56 == '.') {
            FUN_0058f078(local_55,&PTR_DAT_005baa5c);
            FUN_0058f078(local_9c,s_mesh__s_005baa60,local_5c);
          }
          else {
            FUN_0058f078(local_9c,s_mesh__s_wyt_005baa68,local_5c);
          }
          uVar2 = FUN_004b9bb9(local_9c);
          *(undefined4 *)(param_1 + 0x298 + local_48 * 4) = uVar2;
        }
      }
      local_50 = 0;
      FUN_00590d68(local_34,4,1,local_8);
      if (local_34[0] != 0) {
        (**(code **)(**(int **)(DAT_005ccf9c + 0x2a3e4) + 0x60))
                  (*(undefined4 *)(DAT_005ccf9c + 0x2a3e4),local_34[0],8,0x65,1,param_1 + 0x31c);
        if (*(int *)(param_1 + 0x31c) == 0) {
          return 0;
        }
        (**(code **)(**(int **)(param_1 + 0x31c) + 0x2c))
                  (*(undefined4 *)(param_1 + 0x31c),0,0,&local_50,0);
        FUN_00590d68(local_50,local_34[0],1,local_8);
        (**(code **)(**(int **)(param_1 + 0x31c) + 0x30))(*(undefined4 *)(param_1 + 0x31c));
      }
      local_10 = 0;
      local_4c = 0;
      FUN_00590d68(local_20,4,1,local_8);
      local_c = *(uint *)(param_1 + 0xc) >> 2;
      local_18 = local_20[0] / *(uint *)(param_1 + 0xc);
      if (*(int *)(param_1 + 0x10) == 0x142) {
        (**(code **)(**(int **)(DAT_005ccf9c + 0x2a3e4) + 0x5c))
                  (*(undefined4 *)(DAT_005ccf9c + 0x2a3e4),local_20[0],8,
                   *(undefined4 *)(param_1 + 0x10),1,param_1 + 0x318);
      }
      else {
        *(int *)(param_1 + 0x10) = *(int *)(param_1 + 0x10) + 0x100;
        (**(code **)(**(int **)(DAT_005ccf9c + 0x2a3e4) + 0x5c))
                  (*(undefined4 *)(DAT_005ccf9c + 0x2a3e4),local_20[0] + local_18 * 8,8,
                   *(undefined4 *)(param_1 + 0x10),1,param_1 + 0x318);
      }
      if (*(int *)(param_1 + 0x318) == 0) {
        uVar2 = 0;
      }
      else {
        (**(code **)(**(int **)(param_1 + 0x318) + 0x2c))
                  (*(undefined4 *)(param_1 + 0x318),0,0,&local_10,0);
        if (*(int *)(param_1 + 0x10) == 0x142) {
          FUN_00590d68(local_10,local_20[0],1,local_8);
        }
        else {
          local_4c = local_10;
          for (local_48 = 0; local_48 < local_18; local_48 = local_48 + 1) {
            FUN_00590d68(local_4c + local_48 * (*(int *)(param_1 + 0xc) + 8),
                         *(undefined4 *)(param_1 + 0xc),1,local_8);
          }
          *(int *)(param_1 + 0xc) = *(int *)(param_1 + 0xc) + 8;
        }
        local_14 = 0;
        if (*(int *)(param_1 + 0x10) != 0x142) {
          local_14 = 2;
        }
        *(undefined4 *)(param_1 + 0x348) = *(undefined4 *)(local_10 + (local_c + local_14) * 4);
        *(undefined4 *)(param_1 + 0x344) = *(undefined4 *)(param_1 + 0x348);
        *(undefined4 *)(param_1 + 0x350) = *(undefined4 *)(local_10 + 4 + (local_c + local_14) * 4);
        *(undefined4 *)(param_1 + 0x34c) = *(undefined4 *)(param_1 + 0x350);
        *(undefined4 *)(param_1 + 0x358) = *(undefined4 *)(local_10 + 8 + (local_c + local_14) * 4);
        *(undefined4 *)(param_1 + 0x354) = *(undefined4 *)(param_1 + 0x358);
        for (local_48 = 0; local_48 < local_18 - 1; local_48 = local_48 + 1) {
          if (*(int *)(param_1 + 0x10) == 0x212) {
            *(undefined4 *)(local_10 + 0x20 + local_48 * (local_c + local_14) * 4) =
                 *(undefined4 *)(local_10 + 0x18 + local_48 * (local_c + local_14) * 4);
            *(undefined4 *)(local_10 + 0x24 + local_48 * (local_c + local_14) * 4) =
                 *(undefined4 *)(local_10 + 0x1c + local_48 * (local_c + local_14) * 4);
          }
          if (*(float *)(param_1 + 0x344) <
              *(float *)(local_10 + local_48 * (local_c + local_14) * 4)) {
            *(undefined4 *)(param_1 + 0x344) =
                 *(undefined4 *)(local_10 + local_48 * (local_c + local_14) * 4);
          }
          if (*(float *)(local_10 + local_48 * (local_c + local_14) * 4) <
              *(float *)(param_1 + 0x348)) {
            *(undefined4 *)(param_1 + 0x348) =
                 *(undefined4 *)(local_10 + local_48 * (local_c + local_14) * 4);
          }
          if (*(float *)(param_1 + 0x34c) <
              *(float *)(local_10 + 4 + local_48 * (local_c + local_14) * 4)) {
            *(undefined4 *)(param_1 + 0x34c) =
                 *(undefined4 *)(local_10 + 4 + local_48 * (local_c + local_14) * 4);
          }
          if (*(float *)(local_10 + 4 + local_48 * (local_c + local_14) * 4) <
              *(float *)(param_1 + 0x350)) {
            *(undefined4 *)(param_1 + 0x350) =
                 *(undefined4 *)(local_10 + 4 + local_48 * (local_c + local_14) * 4);
          }
          if (*(float *)(param_1 + 0x354) <
              *(float *)(local_10 + 8 + local_48 * (local_c + local_14) * 4)) {
            *(undefined4 *)(param_1 + 0x354) =
                 *(undefined4 *)(local_10 + 8 + local_48 * (local_c + local_14) * 4);
          }
          if (*(float *)(local_10 + 8 + local_48 * (local_c + local_14) * 4) <
              *(float *)(param_1 + 0x358)) {
            *(undefined4 *)(param_1 + 0x358) =
                 *(undefined4 *)(local_10 + 8 + local_48 * (local_c + local_14) * 4);
          }
        }
        if (*(float *)(param_1 + 0x344) <= _DAT_005a3534) {
          local_b0 = -*(float *)(param_1 + 0x344);
        }
        else {
          local_b0 = *(float *)(param_1 + 0x344);
        }
        if (*(float *)(param_1 + 0x340) < local_b0) {
          if (*(float *)(param_1 + 0x344) <= _DAT_005a3534) {
            local_b4 = -*(float *)(param_1 + 0x344);
          }
          else {
            local_b4 = *(float *)(param_1 + 0x344);
          }
          *(float *)(param_1 + 0x340) = local_b4;
        }
        if (*(float *)(param_1 + 0x348) <= _DAT_005a3534) {
          local_b8 = -*(float *)(param_1 + 0x348);
        }
        else {
          local_b8 = *(float *)(param_1 + 0x348);
        }
        if (*(float *)(param_1 + 0x340) < local_b8) {
          if (*(float *)(param_1 + 0x348) <= _DAT_005a3534) {
            local_bc = -*(float *)(param_1 + 0x348);
          }
          else {
            local_bc = *(float *)(param_1 + 0x348);
          }
          *(float *)(param_1 + 0x340) = local_bc;
        }
        if (*(float *)(param_1 + 0x34c) <= _DAT_005a3534) {
          local_c0 = -*(float *)(param_1 + 0x34c);
        }
        else {
          local_c0 = *(float *)(param_1 + 0x34c);
        }
        if (*(float *)(param_1 + 0x340) < local_c0) {
          if (*(float *)(param_1 + 0x34c) <= _DAT_005a3534) {
            local_c4 = -*(float *)(param_1 + 0x34c);
          }
          else {
            local_c4 = *(float *)(param_1 + 0x34c);
          }
          *(float *)(param_1 + 0x340) = local_c4;
        }
        if (*(float *)(param_1 + 0x350) <= _DAT_005a3534) {
          local_c8 = -*(float *)(param_1 + 0x350);
        }
        else {
          local_c8 = *(float *)(param_1 + 0x350);
        }
        if (*(float *)(param_1 + 0x340) < local_c8) {
          if (*(float *)(param_1 + 0x350) <= _DAT_005a3534) {
            local_cc = -*(float *)(param_1 + 0x350);
          }
          else {
            local_cc = *(float *)(param_1 + 0x350);
          }
          *(float *)(param_1 + 0x340) = local_cc;
        }
        if (*(float *)(param_1 + 0x354) <= _DAT_005a3534) {
          local_d0 = -*(float *)(param_1 + 0x354);
        }
        else {
          local_d0 = *(float *)(param_1 + 0x354);
        }
        if (*(float *)(param_1 + 0x340) < local_d0) {
          if (*(float *)(param_1 + 0x354) <= _DAT_005a3534) {
            local_d4 = -*(float *)(param_1 + 0x354);
          }
          else {
            local_d4 = *(float *)(param_1 + 0x354);
          }
          *(float *)(param_1 + 0x340) = local_d4;
        }
        if (*(float *)(param_1 + 0x358) <= _DAT_005a3534) {
          local_d8 = -*(float *)(param_1 + 0x358);
        }
        else {
          local_d8 = *(float *)(param_1 + 0x358);
        }
        if (*(float *)(param_1 + 0x340) < local_d8) {
          if (*(float *)(param_1 + 0x358) <= _DAT_005a3534) {
            local_dc = -*(float *)(param_1 + 0x358);
          }
          else {
            local_dc = *(float *)(param_1 + 0x358);
          }
          *(float *)(param_1 + 0x340) = local_dc;
        }
        uVar2 = FUN_00430f50((*(float *)(param_1 + 0x344) + *(float *)(param_1 + 0x348)) /
                             _DAT_005a365c,
                             (*(float *)(param_1 + 0x34c) + *(float *)(param_1 + 0x350)) /
                             _DAT_005a365c,
                             (*(float *)(param_1 + 0x354) + *(float *)(param_1 + 0x358)) /
                             _DAT_005a365c);
        FUN_004310a0(uVar2);
        (**(code **)(**(int **)(param_1 + 0x318) + 0x30))(*(undefined4 *)(param_1 + 0x318));
        FUN_0058f62c(local_8);
        uVar2 = 1;
      }
    }
  }
  return uVar2;
}


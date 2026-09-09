// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00547e8a | Name: FUN_00547e8a


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __fastcall FUN_00547e8a(int param_1)

{
  undefined4 uVar1;
  int local_134;
  undefined1 local_130 [64];
  int *local_f0;
  int local_ec;
  int local_e8;
  undefined4 local_e4;
  undefined4 local_e0;
  undefined4 local_dc;
  undefined4 local_d8;
  undefined4 local_d4;
  undefined4 local_d0;
  undefined4 local_cc;
  undefined4 local_c8;
  undefined4 local_c4;
  undefined4 local_c0;
  undefined4 local_bc;
  undefined4 local_a8;
  undefined4 local_a4;
  undefined4 local_a0;
  undefined4 local_9c;
  undefined4 local_98;
  undefined4 local_94;
  undefined4 local_90;
  undefined4 local_88;
  undefined1 local_84 [64];
  undefined1 local_44 [64];
  
  if (DAT_005d03f0 == 1) {
    uVar1 = 0;
  }
  else if ((((DAT_005ccfa0 == 0) || (DAT_005ccfa0 == 3)) || (DAT_005ccfa0 == 4)) &&
          (*(int *)(DAT_0067cf38 + 0x24) != 0x7531)) {
    local_ec = *(int *)(DAT_013b71e8 + 0x1b084);
    _memset(&local_c8,0,0x44);
    local_d8 = 0x3f333333;
    local_d4 = 0x3f333333;
    local_d0 = 0x3f333333;
    local_98 = 0x3e99999a;
    local_94 = 0x3e99999a;
    local_90 = 0x3e99999a;
    local_c8 = 0x3f333333;
    local_c4 = 0x3f333333;
    local_c0 = 0x3f333333;
    local_bc = local_cc;
    local_a8 = 0x3f333333;
    local_a4 = 0x3f333333;
    local_a0 = 0x3f333333;
    local_9c = local_cc;
    local_88 = 0;
    (**(code **)(*(int *)DAT_005ccf9c[0xa8f9] + 0xa8))(DAT_005ccf9c[0xa8f9],&local_c8);
    local_e4 = *(undefined4 *)(local_ec + 0x48);
    local_e0 = *(undefined4 *)(local_ec + 0x4c);
    local_dc = *(undefined4 *)(local_ec + 0x50);
    local_e8 = DAT_0067cf38;
    FUN_00430f10();
    FUN_00430f10();
    FUN_00430f10();
    thunk_FUN_0056239d(local_84,local_e4,*(float *)(param_1 + 0x30) + _DAT_005a3660,local_dc);
    thunk_FUN_00562821(local_44,*(undefined4 *)(param_1 + 0x34),0xbfc90fdb,0);
    thunk_FUN_0056230d(local_130,*(undefined4 *)(param_1 + 0x38),
                       *(float *)(param_1 + 0x38) * _DAT_005a34a0,*(undefined4 *)(param_1 + 0x38));
    thunk_FUN_00561a5b(local_44,DAT_005ccf9c + 0xa99b,local_44);
    thunk_FUN_00561a5b(local_44,local_44,local_130);
    thunk_FUN_00561a5b(local_44,local_44,local_84);
    (**(code **)(*(int *)DAT_005ccf9c[0xa8f9] + 0x94))(DAT_005ccf9c[0xa8f9],0x100,local_44);
    (**(code **)(*DAT_005ccf9c + 0x30))(0x89,0);
    (**(code **)(*DAT_005ccf9c + 0x30))(0x1c,0);
    if (DAT_005ccf9c[0xa8e4] == 1) {
      (**(code **)(*DAT_005ccf9c + 0x34))(1,0xb,1);
      (**(code **)(*DAT_005ccf9c + 0x34))(1,1,1);
    }
    else {
      (**(code **)(*DAT_005ccf9c + 0x34))(0,1,8);
      (**(code **)(*DAT_005ccf9c + 0x34))(1,0xb,0);
      if (*(int *)(param_1 + 0x164) / 10 == 0) {
        (**(code **)(*DAT_005ccf9c + 0x34))(1,1,1);
      }
      else {
        (**(code **)(*DAT_005ccf9c + 0x34))(1,1,0xc);
      }
      uVar1 = FUN_004b925c(*(undefined4 *)(param_1 + 0x160),360000);
      (**(code **)(*DAT_005ccf9c + 0x38))(1,uVar1);
    }
    (**(code **)(*DAT_005ccf9c + 0x30))(0x14,6);
    local_f0 = (int *)FUN_004b14fd(*(undefined4 *)(param_1 + 0x24),1,180000);
    if (local_f0 == (int *)0x0) {
      uVar1 = 0;
    }
    else {
      (**(code **)(*local_f0 + 0xc))(0,0);
      if (*(int *)(param_1 + 0x164) == 3) {
        uVar1 = FUN_004b14fd(0x15,1,180000);
        *(undefined4 *)(param_1 + 0x170) = uVar1;
        if (*(int *)(param_1 + 0x170) != 0) {
          (**(code **)(*DAT_005ccf9c + 0x30))(0x14,2);
          (**(code **)(*DAT_005ccf9c + 0x34))(0,4,2);
          (**(code **)(*DAT_005ccf9c + 0x34))(0,1,4);
          (**(code **)(*DAT_005ccf9c + 0x34))(0,4,4);
          (**(code **)(**(int **)(param_1 + 0x170) + 0xc))(0,0);
        }
      }
      (**(code **)(*DAT_005ccf9c + 0x34))(0,1,4);
      (**(code **)(*DAT_005ccf9c + 0x34))(1,0xb,1);
      (**(code **)(*DAT_005ccf9c + 0x34))(1,1,1);
      (**(code **)(*DAT_005ccf9c + 0x30))(0x1c,DAT_005ccf9c[0xac0e]);
      (**(code **)(*DAT_005ccf9c + 0x30))(0x89,1);
      if ((*(int *)(param_1 + 0x160) == 0x44) || (local_f0[0xa6] == 0x44)) {
        uVar1 = 1;
      }
      else {
        if (((*(int *)(param_1 + 0x164) == 3) || (*(int *)(param_1 + 0x164) == 0xd)) ||
           ((*(int *)(param_1 + 0x164) == 10 && (*(int *)(param_1 + 0x160) == 0x43)))) {
          for (local_134 = 0; local_134 < 0x14; local_134 = local_134 + 1) {
            (**(code **)(*(int *)(param_1 + 0x174 + local_134 * 0x180) + 0x24))();
          }
          (**(code **)(*(int *)(param_1 + 0x1f74) + 0x24))();
          (**(code **)(*(int *)(param_1 + 0x20f4) + 0x24))();
        }
        uVar1 = 1;
      }
    }
  }
  else {
    (**(code **)(*DAT_005ccf9c + 0x34))(0,1,4);
    (**(code **)(*DAT_005ccf9c + 0x30))(0x8c,3);
    uVar1 = 0;
  }
  return uVar1;
}


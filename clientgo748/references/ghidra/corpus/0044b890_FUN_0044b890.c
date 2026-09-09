// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0044b890 | Name: FUN_0044b890


void __thiscall FUN_0044b890(int param_1,int param_2)

{
  int iVar1;
  undefined4 uVar2;
  undefined4 uVar3;
  undefined4 *local_fc;
  int local_f8;
  int aiStack_f4 [15];
  int aiStack_b8 [15];
  int local_7c;
  int local_78;
  int local_74;
  undefined1 local_70 [4];
  undefined2 local_6c;
  undefined2 local_6a;
  int local_64;
  int local_60;
  int *local_5c;
  int local_58;
  int *local_54;
  int *local_50;
  int *local_4c;
  int local_48;
  int *local_44;
  int local_40;
  int *local_3c;
  int *local_38;
  int *local_34;
  int *local_30;
  int *local_2c;
  int *local_28;
  undefined4 local_24;
  int *local_20;
  int *local_1c;
  int *local_18;
  int *local_14;
  undefined4 local_10;
  int *local_c;
  int *local_8;
  
  DAT_005b12bc = 0xffff;
  local_3c = *(int **)(param_1 + 0x285f0);
  local_34 = *(int **)(param_1 + 0x285d8);
  local_1c = *(int **)(param_1 + 0x27b34);
  local_30 = *(int **)(param_1 + 0x285d4);
  local_18 = *(int **)(param_1 + 0x27c7c);
  local_14 = *(int **)(param_1 + 0x27c84);
  local_44 = *(int **)(param_1 + 0x285dc);
  local_38 = *(int **)(param_1 + 0x27bdc);
  local_4c = *(int **)(param_1 + 0x27be4);
  local_24 = (**(code **)(**(int **)(param_1 + 0x28) + 0x48))(0x259);
  local_10 = (**(code **)(**(int **)(param_1 + 0x28) + 0x48))(0x269);
  local_c = (int *)(**(code **)(**(int **)(param_1 + 0x28) + 0x48))(0x126);
  local_20 = (int *)(**(code **)(**(int **)(param_1 + 0x28) + 0x48))(0x127);
  local_8 = (int *)(**(code **)(**(int **)(param_1 + 0x28) + 0x48))(0x125);
  local_2c = (int *)(**(code **)(**(int **)(param_1 + 0x28) + 0x48))(0x26b);
  local_28 = (int *)(**(code **)(**(int **)(param_1 + 0x28) + 0x48))(0x25b);
  *(undefined1 *)((int)local_2c + 0xcdd) = 1;
  *(undefined1 *)((int)local_28 + 0xcdd) = 1;
  local_40 = 0;
  iVar1 = FUN_0040c0f0();
  if ((iVar1 == 1) && (param_2 == 0)) {
    local_40 = 1;
  }
  local_48 = param_2;
  if (param_2 == 1) {
    (**(code **)(*DAT_005ccec0 + 0x98))();
    (**(code **)(*local_4c + 0x60))(0);
    local_50 = *(int **)(param_1 + 0x27c70);
    iVar1 = FUN_0040c0f0();
    if ((iVar1 == 1) && (*(int *)(param_1 + 0x27b38) != 0)) {
      FUN_0044ae38(0,0);
    }
    (**(code **)(*local_50 + 0x60))(0);
    (**(code **)(*local_18 + 0x60))(0);
    (**(code **)(*local_34 + 0x60))(0);
    (**(code **)(*local_3c + 0x60))(local_48);
    (**(code **)(*local_1c + 0x60))(local_48);
    (**(code **)(*local_14 + 0x60))(0);
    (**(code **)(*local_44 + 0x60))(0);
    (**(code **)(*local_30 + 0x60))(0);
    (**(code **)(*local_38 + 0x60))(0);
    (**(code **)(*local_c + 0x8c))(0);
    (**(code **)(*local_20 + 0x8c))(0);
    (**(code **)(*local_8 + 0x8c))(0);
    iVar1 = FUN_0040c0f0();
    if (iVar1 == 1) {
      (**(code **)(*local_14 + 0x60))(0);
    }
    if (DAT_005b892c != 2) {
      uVar2 = __ftol();
      *(undefined4 *)(DAT_005ccf9c + 0x2a5e4) = uVar2;
    }
  }
  else {
    local_5c = *(int **)(param_1 + 0x27c78);
    local_54 = *(int **)(param_1 + 0x2873c);
    for (local_58 = 0; local_58 < 7; local_58 = local_58 + 1) {
      for (local_60 = 0; local_60 < 9; local_60 = local_60 + 1) {
        local_64 = (**(code **)(*local_54 + 0xb4))(local_60,local_58);
        if ((local_64 != 0) && (*(int *)(local_64 + 0x94) == -0x10000)) {
          *(undefined4 *)(local_64 + 0x94) = 0xffffffff;
        }
      }
    }
    (**(code **)(*local_2c + 0x80))(s_0_005b7d4c,0);
    (**(code **)(*local_28 + 0x80))(s_0_005b7d58,0);
    (**(code **)(*local_3c + 0x60))(local_48);
    (**(code **)(**(int **)(param_1 + 0x27b34) + 0x60))(0);
    (**(code **)(*local_5c + 0x60))(0);
    if ((DAT_005b892c != 2) && (iVar1 = FUN_0040c0f0(), iVar1 == 0)) {
      *(undefined4 *)(DAT_005ccf9c + 0x2a5e4) = 0;
    }
  }
  if ((DAT_005ccf98 != 0) && (iVar1 = FUN_00429a6d(0x33), iVar1 != 0)) {
    uVar3 = 0;
    uVar2 = 0;
    FUN_00429a6d(0x33);
    FUN_0042ad2b(uVar2,uVar3);
  }
  if ((local_48 == 0) && (*(short *)(DAT_013b71e8 + 0xcfa) != 0)) {
    *(undefined2 *)(DAT_013b71e8 + 0xcfa) = 0;
    *(undefined1 *)(DAT_013b71e8 + 0xcf8) = 0;
    uVar2 = (**(code **)(**(int **)(DAT_013b71e0 + 0xf4) + 8))();
    *(undefined4 *)(param_1 + 0x274d0) = uVar2;
    if (local_40 == 1) {
      _memset(local_70,0,0xc);
      local_6c = 900;
      local_6a = *(undefined2 *)(*(int *)(param_1 + 0x4c) + 0x20);
      FUN_0055f2dd(local_70,0xc);
    }
  }
  if (local_48 == 0) {
    _memset((void *)(DAT_013b71e8 + 0xc60),0,0x9c);
    for (local_74 = 0; local_74 < 0xf; local_74 = local_74 + 1) {
      *(undefined1 *)(DAT_013b71e8 + local_74 + 0xce4) = 0xff;
    }
  }
  if (local_48 == 1) {
    *(undefined4 *)(*(int *)(param_1 + 0x2873c) + 0x400) = 7;
    FUN_00447f6f(0);
    *(undefined2 *)(DAT_013b71e8 + 0xc64) = 899;
    *(undefined2 *)(DAT_013b71e8 + 0xc66) = *(undefined2 *)(*(int *)(param_1 + 0x4c) + 0x20);
  }
  else {
    *(undefined4 *)(*(int *)(param_1 + 0x2873c) + 0x400) = 0;
    FUN_00447f6f(1);
    _memset((void *)(DAT_013b71e8 + 0xc60),0,0x9c);
    for (local_f8 = 0; local_f8 < 0xf; local_f8 = local_f8 + 1) {
      *(undefined1 *)(DAT_013b71e8 + local_f8 + 0xce4) = 0xff;
    }
    for (local_f8 = 0; local_f8 < 0xf; local_f8 = local_f8 + 1) {
      local_fc = (undefined4 *)0x0;
      iVar1 = (**(code **)(**(int **)(param_1 + 0x28) + 0x48))(local_f8 + 0x2000);
      aiStack_f4[local_f8] = iVar1;
      if (aiStack_f4[local_f8] != 0) {
        local_fc = (undefined4 *)(**(code **)(*(int *)aiStack_f4[local_f8] + 0xa4))(0,0);
      }
      if ((DAT_005ccec0[0x7a] != 0) && ((undefined4 *)DAT_005ccec0[0x7a] == local_fc)) {
        DAT_005ccec0[0x7a] = 0;
      }
      if (local_fc != (undefined4 *)0x0) {
        if (local_fc != (undefined4 *)0x0) {
          (**(code **)*local_fc)(1);
        }
        local_fc = (undefined4 *)0x0;
      }
      iVar1 = (**(code **)(**(int **)(param_1 + 0x28) + 0x48))(local_f8 + 0x2100);
      aiStack_b8[local_f8] = iVar1;
      if (aiStack_b8[local_f8] != 0) {
        local_fc = (undefined4 *)(**(code **)(*(int *)aiStack_b8[local_f8] + 0xa4))(0,0);
      }
      if ((DAT_005ccec0[0x7a] != 0) && ((undefined4 *)DAT_005ccec0[0x7a] == local_fc)) {
        DAT_005ccec0[0x7a] = 0;
      }
      if ((local_fc != (undefined4 *)0x0) && (local_fc != (undefined4 *)0x0)) {
        (**(code **)*local_fc)(1);
      }
    }
    local_7c = (**(code **)(**(int **)(param_1 + 0x28) + 0x48))(0x259);
    local_78 = (**(code **)(**(int **)(param_1 + 0x28) + 0x48))(0x269);
    *(undefined4 *)(local_7c + 0x1e8) = 0;
    *(undefined4 *)(local_78 + 0x1e8) = 0;
    uVar2 = (**(code **)(**(int **)(DAT_013b71e0 + 0xf4) + 8))();
    *(undefined4 *)(param_1 + 0x274d0) = uVar2;
    iVar1 = FUN_0040c0f0();
    if (iVar1 == 1) {
      FUN_00447691();
    }
  }
  return;
}


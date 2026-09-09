// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00499f0e | Name: FUN_00499f0e


void __thiscall FUN_00499f0e(int param_1,int param_2,int param_3)

{
  undefined4 *puVar1;
  void *pvVar2;
  int iVar3;
  undefined4 uVar4;
  uint uVar5;
  undefined4 local_164;
  undefined4 local_158;
  int local_124;
  int local_120;
  undefined1 local_11c [128];
  undefined4 local_9c;
  undefined1 local_98 [128];
  int local_18;
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_005a0186;
  local_10 = ExceptionList;
  if (*(int *)(param_1 + 0x54) == 1) {
    return;
  }
  if (*(int *)(param_1 + 0x40) == 0) {
    return;
  }
  ExceptionList = &local_10;
  *(undefined4 *)(param_1 + 0x200a0) = 0;
  if ((*(int *)(param_1 + 0x24) == 30000) &&
     (local_18 = param_1,
     (**(code **)(**(int **)(param_1 + 0x2791c) + 0x60))(*(undefined4 *)(param_1 + 0x200a0)),
     *(int *)(local_18 + 0x27c98) != 0)) {
    (**(code **)(**(int **)(local_18 + 0x27c98) + 0x8c))(*(undefined4 *)(param_1 + 0x200a0));
  }
  local_14 = 0;
  if (*(int *)(*(int *)(param_1 + 0x40) + 0xac) != 0) {
    local_14 = *(int *)(*(int *)(param_1 + 0x40) + 0xac);
  }
  if (*(int *)(*(int *)(param_1 + 0x40) + 0xb0) != 0) {
    local_14 = *(int *)(*(int *)(param_1 + 0x40) + 0xb0);
  }
  if (*(int *)(*(int *)(param_1 + 0x40) + 0xb4) != 0) {
    local_14 = *(int *)(*(int *)(param_1 + 0x40) + 0xb4);
  }
  if (*(int *)(*(int *)(param_1 + 0x40) + 0xb8) != 0) {
    local_14 = *(int *)(*(int *)(param_1 + 0x40) + 0xb8);
  }
  if (((local_14 == 0) &&
      ((param_2 != *(int *)(*(int *)(param_1 + 0x40) + 0x20a20) ||
       (param_3 != *(int *)(*(int *)(param_1 + 0x40) + 0x20a24))))) ||
     ((local_14 != 0 &&
      (((param_2 != *(int *)(*(int *)(param_1 + 0x40) + 0x20a20) &&
        (param_2 != *(int *)(local_14 + 0x20a20))) ||
       ((param_3 != *(int *)(*(int *)(param_1 + 0x40) + 0x20a24) &&
        (param_3 != *(int *)(local_14 + 0x20a24))))))))) {
    FUN_0058f078(local_11c,s_env_Field_02d_02d_trn_005b90f0,param_2,param_3);
    FUN_0058f078(local_98,s_env_Field_02d_02d_dat_005b9108,param_2,param_3);
    pvVar2 = operator_new(0x20a70);
    local_8 = 0;
    if (pvVar2 == (void *)0x0) {
      local_158 = 0;
    }
    else {
      local_158 = FUN_00532390();
    }
    local_8 = 0xffffffff;
    local_9c = 0;
    iVar3 = FUN_00533dd7(local_11c);
    if (iVar3 == 0) {
      if (*(int *)(param_1 + 0x54) == 0) {
        FUN_0049ed58(10,0,0,0,0);
      }
      *(undefined4 *)(param_1 + 0x54) = 1;
      ExceptionList = local_10;
      return;
    }
    local_9c = *(undefined4 *)(param_1 + 0x40);
    *(undefined4 *)(param_1 + 0x40) = local_158;
    for (local_124 = 0; local_124 < 2; local_124 = local_124 + 1) {
      if (*(int *)(param_1 + 0x70 + local_124 * 4) != 0) {
        puVar1 = *(undefined4 **)(param_1 + 0x70 + local_124 * 4);
        if (puVar1 != (undefined4 *)0x0) {
          (**(code **)*puVar1)(1);
        }
        *(undefined4 *)(param_1 + 0x70 + local_124 * 4) = 0;
      }
      if (*(int *)(param_1 + 0x68 + local_124 * 4) != 0) {
        puVar1 = *(undefined4 **)(param_1 + 0x68 + local_124 * 4);
        if (puVar1 != (undefined4 *)0x0) {
          (**(code **)*puVar1)(1);
        }
        *(undefined4 *)(param_1 + 0x68 + local_124 * 4) = 0;
      }
    }
    DAT_005b8924 = param_2 << 7;
    DAT_005b8928 = param_3 << 7;
    *(undefined4 *)(param_1 + 0x78) = 0;
    *(undefined4 *)(param_1 + 0x68) = *(undefined4 *)(param_1 + 0x40);
    pvVar2 = operator_new(0x404c);
    local_8 = 1;
    if (pvVar2 == (void *)0x0) {
      local_164 = 0;
    }
    else {
      local_164 = FUN_00540ea3(*(undefined4 *)(param_1 + 0x40));
    }
    local_8 = 0xffffffff;
    *(undefined4 *)(param_1 + 0x70) = local_164;
    iVar3 = FUN_00541065(local_98);
    if (iVar3 == 0) {
      FUN_00431159(s_DataFile_Not_Found____s_005b9120,local_98);
      if (*(int *)(param_1 + 0x54) == 0) {
        FUN_0049ed58(0xb,0,0,0,0);
      }
      *(undefined4 *)(param_1 + 0x54) = 1;
      ExceptionList = local_10;
      return;
    }
    _memset((void *)(param_1 + 0x94),0,4);
    for (local_120 = 0; local_120 < 0x80; local_120 = local_120 + 1) {
      FUN_0058f220(param_1 + 0x94 + local_120 * 0x100,
                   *(int *)(param_1 + 0x40) + 0xc0cc + local_120 * 0x80,0x80);
    }
    FUN_00534968();
    FUN_0054ac09(*(undefined4 *)(param_1 + 0x70));
    FUN_0054ac09(*(undefined4 *)(param_1 + 0x68));
    FUN_0054f18b(param_1 + 0x94,0x100);
    FUN_0058f220(param_1 + 0x10094,param_1 + 0x94,0x10000);
    FUN_00499e99(local_11c);
    *(undefined4 *)(param_1 + 0x200ac) = 0;
    uVar4 = (**(code **)(*DAT_0092e654 + 8))();
    *(undefined4 *)(param_1 + 0x200a8) = uVar4;
  }
  else if (((local_14 != 0) && (param_2 == *(int *)(local_14 + 0x20a20))) &&
          (param_3 == *(int *)(local_14 + 0x20a24))) {
    *(int *)(param_1 + 0x40) = local_14;
    uVar5 = *(int *)(param_1 + 0x78) + 1U & 0x80000001;
    if ((int)uVar5 < 0) {
      uVar5 = (uVar5 - 1 | 0xfffffffe) + 1;
    }
    *(uint *)(param_1 + 0x78) = uVar5;
    FUN_00534968();
  }
  if (*(int *)(param_1 + 0x24) == 30000) {
    if ((DAT_005ccf98 != 0) && (iVar3 = FUN_00429a6d(6), iVar3 != 0)) {
      FUN_00429a6d(6);
      iVar3 = FUN_0042afd0();
      if (iVar3 != 0) {
        FUN_00429a6d(6);
        FUN_0042aef6();
      }
    }
    if (((DAT_005ccfa0 == 0) || (DAT_005ccfa0 == 3)) || (DAT_005ccfa0 == 4)) {
      if ((DAT_005ccfa0 == 3) || (DAT_005ccfa0 == 4)) {
        *(undefined4 *)(*(int *)(param_1 + 0x26ea0) + 0x28) = 0;
        *(undefined4 *)(*(int *)(param_1 + 0x26ea4) + 0x28) = 0;
      }
      FUN_004819d5(DAT_005d03e4);
      *(undefined4 *)(*(int *)(DAT_013b71e8 + 0x1b084) + 0x28) = 0x3f490fdb;
      *(undefined4 *)(*(int *)(param_1 + 0x27d98) + 0x94) = 0xd0ffffff;
    }
    else {
      DAT_005d03e4 = 0;
      *(undefined4 *)(*(int *)(param_1 + 0x26e9c) + 0x28) = 0;
      *(undefined4 *)(*(int *)(param_1 + 0x26ea0) + 0x28) = 0;
      *(undefined4 *)(*(int *)(param_1 + 0x26ea4) + 0x28) = 0;
      *(undefined4 *)(param_1 + 0x274e8) = 1;
      FUN_004431e4(0);
      *(undefined4 *)(*(int *)(param_1 + 0x27d98) + 0x94) = 0xd0ffffff;
    }
  }
  ExceptionList = local_10;
  return;
}


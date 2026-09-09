// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0048150a | Name: FUN_0048150a


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __thiscall FUN_0048150a(int param_1,int param_2,int param_3)

{
  int iVar1;
  void *pvVar2;
  undefined4 uVar3;
  undefined4 uVar4;
  undefined4 uVar5;
  undefined4 uVar6;
  undefined4 uVar7;
  undefined4 uVar8;
  undefined4 uVar9;
  int local_170;
  int local_16c;
  int local_168;
  int local_164;
  undefined1 local_134 [128];
  undefined4 local_b4;
  int local_b0;
  int local_ac;
  undefined1 local_a8 [128];
  undefined4 local_28;
  int local_24;
  int local_20;
  int local_1c;
  int local_18;
  undefined4 local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  iVar1 = DAT_013b71e8;
  local_8 = 0xffffffff;
  puStack_c = &LAB_0059f9c7;
  local_10 = ExceptionList;
  local_24 = DAT_013b71e8 + 0x6ec;
  local_20 = param_2 - *(int *)(DAT_013b71e8 + 0x708);
  local_1c = *(int *)(DAT_013b71e8 + 0xc50) - param_3;
  if ((local_20 != 0) || (local_1c != 0)) {
    ExceptionList = &local_10;
    *(int *)(DAT_013b71e8 + 0x708) = param_2;
    *(int *)(DAT_013b71e8 + 0xc50) = param_3;
    FUN_0058f220(*(int *)(param_1 + 0x4c) + 0x444,iVar1 + 0x72c,0x1c);
    FUN_004431e4(0);
    if ((0 < local_20) &&
       (iVar1 = FUN_00430d79(*(undefined4 *)(*(int *)(param_1 + 0x4c) + 0x28),
                             *(float *)(*(int *)(param_1 + 0x4c) + 0x30) + _DAT_005a3660,
                             *(undefined4 *)(*(int *)(param_1 + 0x4c) + 0x2c),&local_14,&local_18),
       iVar1 != 0)) {
      local_28 = 0;
      FUN_0058f078(local_a8,s_Exp___d_005b87e0,local_20);
      pvVar2 = operator_new(0x2a0);
      local_8 = 0;
      if (pvVar2 == (void *)0x0) {
        local_164 = 0;
      }
      else {
        uVar9 = 1;
        uVar8 = 0;
        uVar7 = 0x4b0;
        uVar6 = 1;
        uVar4 = 0x3f000000;
        uVar3 = 0xffff8866;
        uVar5 = local_28;
        iVar1 = __ftol(0xffff8866,0x3f000000,local_28,1,0x4b0,0,1);
        local_164 = FUN_00422930(local_a8,local_14,local_18 + iVar1,uVar3,uVar4,uVar5,uVar6,uVar7,
                                 uVar8,uVar9);
      }
      local_8 = 0xffffffff;
      local_ac = local_164;
      if (local_164 != 0) {
        FUN_0054ac09(local_164);
      }
      FUN_0058f078(local_a8,&DAT_00a38b78,local_20);
      if (*(int *)(param_1 + 0x28ec8) == 0) {
        pvVar2 = operator_new(0xe50);
        local_8 = 1;
        if (pvVar2 == (void *)0x0) {
          local_168 = 0;
        }
        else {
          local_168 = FUN_00407203(local_a8,0xffccaaff,0,0,0x43960000,0x41800000,0,0x77777777,1,0);
        }
        local_8 = 0xffffffff;
        local_b0 = local_168;
        if ((local_168 != 0) && (*(int *)(param_1 + 0x27ae8) != 0)) {
          FUN_00408d33(local_168);
        }
      }
    }
    if ((0 < local_1c) &&
       (iVar1 = FUN_00430d79(*(undefined4 *)(*(int *)(param_1 + 0x4c) + 0x28),
                             *(float *)(*(int *)(param_1 + 0x4c) + 0x30) + _DAT_005a3660,
                             *(undefined4 *)(*(int *)(param_1 + 0x4c) + 0x2c),&local_14,&local_18),
       iVar1 != 0)) {
      local_b4 = 0;
      FUN_0058f078(local_134,s_Cp___d_005b87e8,local_1c);
      pvVar2 = operator_new(0x2a0);
      local_8 = 2;
      if (pvVar2 == (void *)0x0) {
        local_16c = 0;
      }
      else {
        uVar9 = 1;
        uVar8 = 0;
        uVar7 = 0x5dc;
        uVar6 = 1;
        uVar4 = 0x3f800000;
        uVar3 = 0xffff8866;
        uVar5 = local_b4;
        iVar1 = __ftol(0xffff8866,0x3f800000,local_b4,1,0x5dc,0,1);
        local_16c = FUN_00422930(local_134,local_14,local_18 + iVar1,uVar3,uVar4,uVar5,uVar6,uVar7,
                                 uVar8,uVar9);
      }
      local_8 = 0xffffffff;
      if (local_16c != 0) {
        FUN_0054ac09(local_16c);
      }
      FUN_0058f078(local_134,&DAT_00a3d9f8,local_1c);
      if (*(int *)(param_1 + 0x28ec8) == 0) {
        pvVar2 = operator_new(0xe50);
        local_8 = 3;
        if (pvVar2 == (void *)0x0) {
          local_170 = 0;
        }
        else {
          local_170 = FUN_00407203(local_134,0xffccaaff,0,0,0x43960000,0x41800000,0,0x77777777,1,0);
        }
        local_8 = 0xffffffff;
        if ((*(int *)(param_1 + 0x27ae8) != 0) && (local_170 != 0)) {
          FUN_00408d33(local_170);
        }
      }
    }
  }
  ExceptionList = local_10;
  return;
}


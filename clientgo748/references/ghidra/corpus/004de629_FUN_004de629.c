// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004de629 | Name: FUN_004de629


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __fastcall FUN_004de629(int param_1)

{
  byte bVar1;
  int iVar2;
  int *piVar3;
  int *piVar4;
  undefined1 *puVar5;
  undefined4 uVar6;
  undefined4 uVar7;
  undefined1 local_13c [12];
  undefined1 local_130 [12];
  undefined1 local_124 [12];
  undefined1 local_118 [12];
  undefined1 local_10c [12];
  undefined1 local_100 [12];
  undefined1 local_f4 [12];
  undefined1 local_e8 [12];
  float local_dc;
  int local_d8;
  undefined1 local_d4 [64];
  int local_94;
  int local_90;
  byte local_8c;
  undefined3 uStack_8b;
  float local_88;
  float local_84;
  float local_80;
  float local_70;
  float local_6c;
  int local_68 [16];
  float local_28;
  int local_24;
  int local_20;
  float local_1c;
  float local_18;
  float local_14;
  undefined1 local_10 [12];
  
  if (DAT_0092e660 != 1) {
    if (*(float *)(param_1 + 0xd64) <= (float)_DAT_005a4cf0) {
      if (*(int *)(param_1 + 0x28) == 0) {
        return 1;
      }
    }
    else {
      if (*(int *)(param_1 + 0x28) == 0) {
        if (DAT_005ccf98 == 0) {
          return 1;
        }
        iVar2 = FUN_00429a6d(0x71);
        if (iVar2 == 0) {
          return 1;
        }
        FUN_00429a6d(0x71);
        iVar2 = FUN_0042afd0();
        if (iVar2 == 0) {
          return 1;
        }
        FUN_00429a6d(0x71);
        FUN_0042aef6();
        return 1;
      }
      if ((DAT_005ccf98 != 0) && (iVar2 = FUN_00429a6d(0x71), iVar2 != 0)) {
        FUN_00429a6d(0x71);
        iVar2 = FUN_0042afd0();
        if (iVar2 == 0) {
          uVar7 = 1;
          uVar6 = 0;
          FUN_00429a6d(0x71);
          FUN_0042ad2b(uVar6,uVar7);
        }
      }
    }
    (**(code **)(*DAT_005ccf9c + 0x30))(0x1b,1);
    (**(code **)(*DAT_005ccf9c + 0x30))(0x1c,0);
    (**(code **)(*DAT_005ccf9c + 0x30))(0x14,2);
    (**(code **)(*DAT_005ccf9c + 0x30))(0x13,5);
    (**(code **)(*DAT_005ccf9c + 0x30))(0x19,8);
    (**(code **)(*DAT_005ccf9c + 0x30))(0xf,0);
    (**(code **)(*DAT_005ccf9c + 0x30))(0xe,0);
    (**(code **)(*DAT_005ccf9c + 0x30))(0x89,0);
    FUN_00430f20(0,0);
    local_20 = (**(code **)(**(int **)(DAT_013b71e8 + 0x1b084) + 0x4c))();
    if ((DAT_0067cf38 == 0) || (iVar2 = FUN_00494dcf(), iVar2 != 0x7534)) {
      if (local_20 == 0) {
        local_70 = *(float *)(*(int *)(DAT_013b71e8 + 0x1b084) + 0x78) + _DAT_005a4bd8;
        local_6c = *(float *)(*(int *)(DAT_013b71e8 + 0x1b084) + 0x7c) + _DAT_005a4bd8;
      }
      else {
        local_70 = *(float *)(local_20 + 0x28);
        local_6c = *(float *)(local_20 + 0x2c);
      }
    }
    else {
      local_70 = *(float *)(*(int *)(DAT_013b71e8 + 0x1b084) + 0x48) - _DAT_005a34a0;
      local_6c = *(float *)(*(int *)(DAT_013b71e8 + 0x1b084) + 0x50) + _DAT_005a4cec;
    }
    uVar6 = FUN_004b925c(2,360000);
    (**(code **)(*DAT_005ccf9c + 0x38))(0,uVar6);
    piVar3 = DAT_005ccf9c + 0xa98b;
    piVar4 = local_68;
    for (iVar2 = 0x10; iVar2 != 0; iVar2 = iVar2 + -1) {
      *piVar4 = *piVar3;
      piVar3 = piVar3 + 1;
      piVar4 = piVar4 + 1;
    }
    FUN_00430f50(DAT_005ccf9c[0xa98b],DAT_005ccf9c[0xa98f],DAT_005ccf9c[0xa993]);
    FUN_00430f50(DAT_005ccf9c[0xa98c],DAT_005ccf9c[0xa990],DAT_005ccf9c[0xa994]);
    FUN_00430f50(-local_1c,-local_18,-local_14);
    FUN_00430f50(-local_88,-local_84,-local_80);
    uVar6 = *(undefined4 *)(param_1 + 0xd64);
    puVar5 = local_f4;
    FUN_00493d50(local_e8,&local_88);
    uVar6 = FUN_004c8c60(puVar5,uVar6);
    FUN_004310a0(uVar6);
    uVar6 = *(undefined4 *)(param_1 + 0xd64);
    puVar5 = local_10c;
    FUN_00493d50(local_100,&local_88);
    uVar6 = FUN_004c8c60(puVar5,uVar6);
    FUN_004310a0(uVar6);
    uVar6 = *(undefined4 *)(param_1 + 0xd64);
    puVar5 = local_124;
    FUN_00493d50(local_118,local_10);
    uVar6 = FUN_004c8c60(puVar5,uVar6);
    FUN_004310a0(uVar6);
    uVar6 = *(undefined4 *)(param_1 + 0xd64);
    puVar5 = local_13c;
    FUN_00493d50(local_130,local_10);
    uVar6 = FUN_004c8c60(puVar5,uVar6);
    FUN_004310a0(uVar6);
    local_28 = 0.0;
    if ((DAT_0067cf38 == 0) || (iVar2 = FUN_00494dcf(), iVar2 != 0x7534)) {
      if (local_20 != 0) {
        local_28 = *(float *)(local_20 + 0x30);
      }
    }
    else {
      local_28 = *(float *)(*(int *)(DAT_013b71e8 + 0x1b084) + 0x4c) - _DAT_005a365c;
    }
    for (local_24 = 0; local_24 < 200; local_24 = local_24 + 1) {
      iVar2 = __ftol();
      local_94 = (int)(iVar2 + (iVar2 >> 0x1f & 3U)) >> 2;
      iVar2 = __ftol();
      local_90 = (int)(iVar2 + (iVar2 >> 0x1f & 3U)) >> 2;
      if ((0x3ff < local_94) || (local_94 < 0)) {
        local_94 = 0;
      }
      if ((0x3ff < local_90) || (local_90 < 0)) {
        local_90 = 0;
      }
      _local_8c = CONCAT31(uStack_8b,(&DAT_00934178)[local_94 + local_90 * 0x400]);
      if (((&DAT_00934178)[local_94 + local_90 * 0x400] & 8) == 0) {
        bVar1 = __ftol();
        local_d8._0_1_ = bVar1;
        if (_DAT_005a3664 + local_28 < *(float *)(param_1 + 0x3a8 + local_24 * 0xc)) {
          local_d8._0_1_ = 0xaa;
        }
        if (*(float *)(param_1 + 0x3a8 + local_24 * 0xc) < local_28) {
          local_d8._0_1_ = 0;
        }
        *(uint *)(param_1 + 0xd10) =
             local_d8 << 0x18 | (uint)(byte)local_d8 << 0x10 | (uint)(byte)local_d8 << 8 |
             (uint)(byte)local_d8;
        *(undefined4 *)(param_1 + 0xd58) = *(undefined4 *)(param_1 + 0xd10);
        *(undefined4 *)(param_1 + 0xd40) = *(undefined4 *)(param_1 + 0xd58);
        *(undefined4 *)(param_1 + 0xd28) = *(undefined4 *)(param_1 + 0xd40);
        local_dc = -*(float *)(*(int *)(DAT_013b71e8 + 0x1b084) + 0x2c);
        FUN_00430f10();
        thunk_FUN_0056239d(local_d4,*(undefined4 *)(param_1 + 0x3a4 + local_24 * 0xc),
                           *(undefined4 *)(param_1 + 0x3a8 + local_24 * 0xc),
                           *(undefined4 *)(param_1 + 0x3ac + local_24 * 0xc));
        (**(code **)(*(int *)DAT_005ccf9c[0xa8f9] + 0x94))(DAT_005ccf9c[0xa8f9],0x100,local_d4);
        (**(code **)(*(int *)DAT_005ccf9c[0xa8f9] + 0x130))(DAT_005ccf9c[0xa8f9],0x142);
        (**(code **)(*(int *)DAT_005ccf9c[0xa8f9] + 0x120))
                  (DAT_005ccf9c[0xa8f9],6,2,param_1 + 0xd04,0x18);
      }
    }
    (**(code **)(*DAT_005ccf9c + 0x30))(0x89,1);
    (**(code **)(*DAT_005ccf9c + 0x30))(0x13,2);
    (**(code **)(*DAT_005ccf9c + 0x30))(0x19,7);
    (**(code **)(*DAT_005ccf9c + 0x30))(0xf,1);
    (**(code **)(*DAT_005ccf9c + 0x30))(0xe,1);
    (**(code **)(*DAT_005ccf9c + 0x30))(0x14,6);
  }
  return 1;
}


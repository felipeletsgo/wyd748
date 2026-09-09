// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004dd081 | Name: FUN_004dd081


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __fastcall FUN_004dd081(int param_1)

{
  undefined4 uVar1;
  int iVar2;
  int *piVar3;
  int *piVar4;
  undefined4 uVar5;
  int local_94;
  int local_90;
  float local_84;
  float local_80;
  float local_7c;
  float local_78;
  float local_70;
  float local_6c;
  float local_68;
  int local_64 [16];
  int local_24;
  int local_20;
  float local_1c;
  float local_18;
  float local_14;
  float local_10;
  float local_8;
  
  if (DAT_005d03f0 == 1) {
    uVar1 = 0;
  }
  else if (DAT_0092e660 == 1) {
    uVar1 = 1;
  }
  else if (*(int *)(param_1 + 0x28) == 0) {
    if (DAT_005ccf98 != 0) {
      iVar2 = FUN_00429a6d(0x65);
      if (iVar2 != 0) {
        FUN_00429a6d(0x65);
        iVar2 = FUN_0042afd0();
        if (iVar2 != 0) {
          FUN_00429a6d(0x65);
          FUN_0042aef6();
        }
      }
    }
    uVar1 = 1;
  }
  else {
    if (DAT_005ccf98 != 0) {
      iVar2 = FUN_00429a6d(0x65);
      if (iVar2 != 0) {
        FUN_00429a6d(0x65);
        iVar2 = FUN_0042afd0();
        if (iVar2 == 0) {
          uVar5 = 1;
          uVar1 = 0;
          FUN_00429a6d(0x65);
          FUN_0042ad2b(uVar1,uVar5);
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
    (**(code **)(*DAT_005ccf9c + 0x30))(0x16,1);
    FUN_00430f20(0,0);
    local_20 = (**(code **)(**(int **)(DAT_013b71e8 + 0x1b084) + 0x4c))();
    if (local_20 == 0) {
      local_6c = *(float *)(*(int *)(DAT_013b71e8 + 0x1b084) + 0x78) + _DAT_005a4bd8;
      local_68 = *(float *)(*(int *)(DAT_013b71e8 + 0x1b084) + 0x7c) + _DAT_005a4bd8;
    }
    else {
      local_6c = *(float *)(local_20 + 0x28);
      local_68 = *(float *)(local_20 + 0x2c);
    }
    piVar3 = DAT_005ccf9c + 0xa98b;
    piVar4 = local_64;
    for (iVar2 = 0x10; iVar2 != 0; iVar2 = iVar2 + -1) {
      *piVar4 = *piVar3;
      piVar3 = piVar3 + 1;
      piVar4 = piVar4 + 1;
    }
    FUN_00430f50(DAT_005ccf9c[0xa98b],DAT_005ccf9c[0xa98f],DAT_005ccf9c[0xa993]);
    FUN_00430f50(DAT_005ccf9c[0xa98c],DAT_005ccf9c[0xa990],DAT_005ccf9c[0xa994]);
    FUN_00430f50(-local_1c,-local_18,-local_14);
    FUN_00430f50(-local_84,-local_80,-local_7c);
    *(float *)(param_1 + 0x3a4) = (local_78 + local_84) * 0.008;
    *(float *)(param_1 + 0x3ac) = (local_70 + local_7c) * 0.008;
    *(float *)(param_1 + 0x3bc) = (local_1c + local_84) * 0.008;
    *(float *)(param_1 + 0x3c4) = (local_14 + local_7c) * 0.008;
    *(float *)(param_1 + 0x3d4) = (local_1c + local_10) * 0.008;
    *(float *)(param_1 + 0x3dc) = (local_14 + local_8) * 0.008;
    *(float *)(param_1 + 0x3ec) = (local_78 + local_10) * 0.008;
    *(float *)(param_1 + 0x3f4) = (local_70 + local_8) * 0.008;
    uVar1 = FUN_004b925c(9,360000);
    (**(code **)(*DAT_005ccf9c + 0x38))(0,uVar1);
    for (local_24 = 0; local_24 < 0x32; local_24 = local_24 + 1) {
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
      if (((&DAT_00934178)[local_94 + local_90 * 0x400] & 8) == 0) {
        thunk_FUN_0056239d(param_1 + 0x2c,local_6c + *(float *)(param_1 + 0x14c + local_24 * 0xc),
                           *(undefined4 *)(param_1 + 0x150 + local_24 * 0xc),
                           local_68 + *(float *)(param_1 + 0x154 + local_24 * 0xc));
        (**(code **)(*(int *)DAT_005ccf9c[0xa8f9] + 0x94))
                  (DAT_005ccf9c[0xa8f9],0x100,param_1 + 0x2c);
        (**(code **)(*(int *)DAT_005ccf9c[0xa8f9] + 0x130))(DAT_005ccf9c[0xa8f9],0x142);
        (**(code **)(*(int *)DAT_005ccf9c[0xa8f9] + 0x120))
                  (DAT_005ccf9c[0xa8f9],6,2,param_1 + 0x3a4,0x18);
      }
    }
    (**(code **)(*DAT_005ccf9c + 0x30))(0x16,3);
    (**(code **)(*DAT_005ccf9c + 0x30))(0x89,1);
    (**(code **)(*DAT_005ccf9c + 0x30))(0x13,2);
    (**(code **)(*DAT_005ccf9c + 0x30))(0x19,7);
    (**(code **)(*DAT_005ccf9c + 0x30))(0xf,1);
    (**(code **)(*DAT_005ccf9c + 0x30))(0xe,1);
    (**(code **)(*DAT_005ccf9c + 0x30))(0x14,6);
    uVar1 = 1;
  }
  return uVar1;
}


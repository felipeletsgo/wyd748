// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0055d6e6 | Name: FUN_0055d6e6


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __fastcall FUN_0055d6e6(int param_1)

{
  int *piVar1;
  int iVar2;
  int iVar3;
  undefined4 uVar4;
  undefined4 local_14;
  undefined4 local_10;
  undefined4 local_c;
  int local_8;
  
  local_8 = (**(code **)(**(int **)(*(int *)(param_1 + 0xe0) + 0x2a3e4) + 0xc))
                      (*(undefined4 *)(*(int *)(param_1 + 0xe0) + 0x2a3e4));
  if ((-1 < local_8) || (local_8 != -0x7789f798)) {
    (**(code **)(**(int **)(param_1 + 0xe0) + 0x24))
              (0,0,*(undefined4 *)(param_1 + 0x10),*(undefined4 *)(param_1 + 0x14));
    (**(code **)(**(int **)(param_1 + 0xe0) + 0x28))(1);
    if (DAT_005b892c != 2) {
      (**(code **)(**(int **)(param_1 + 0xe0) + 0x2c))(0);
      uVar4 = __ftol();
      (**(code **)(**(int **)(param_1 + 0xe0) + 0x24))
                (0,0,*(int *)(param_1 + 0x10) - *(int *)(*(int *)(param_1 + 0xe0) + 0x2a5e4),uVar4);
      (**(code **)(**(int **)(param_1 + 0xe0) + 0x28))(0);
    }
    (**(code **)(**(int **)(param_1 + 0xe0) + 0x40))(1);
    piVar1 = *(int **)(*(int *)(param_1 + 0xf8) + 0x1b084);
    (**(code **)(*piVar1 + 0x3c))(&local_14);
    (**(code **)(**(int **)(param_1 + 0xe0) + 0x44))
              (piVar1[0x12],piVar1[0x13],piVar1[0x14],local_14,local_10,local_c);
    (**(code **)(**(int **)(param_1 + 0xf8) + 0x48))();
    iVar2 = DAT_0067cf38;
    if ((((DAT_0067cf38 != 0) && (iVar3 = FUN_00494dcf(), iVar3 == 0x7531)) && (DAT_005b892c != 2))
       && (((iVar3 = (**(code **)(**(int **)(iVar2 + 0x28) + 0x48))(0x606), iVar3 != 0 &&
            (_DAT_005a4340 * DAT_005b4910 + *(float *)(iVar3 + 0x50) + _DAT_005a4104 <
             (float)*(uint *)(param_1 + 0x14))) && (iVar3 = FUN_0040c0f0(), iVar3 == 1)))) {
      (**(code **)(**(int **)(param_1 + 0xe0) + 0x2c))(0);
      uVar4 = __ftol();
      uVar4 = __ftol(uVar4);
      iVar3 = __ftol(uVar4);
      iVar3 = __ftol(iVar3 + 2);
      (**(code **)(**(int **)(param_1 + 0xe0) + 0x24))(iVar3 + 2);
      (**(code **)(**(int **)(param_1 + 0xf8) + 0x50))(*(undefined4 *)(iVar2 + 0x26ef8));
      (**(code **)(**(int **)(param_1 + 0xe0) + 0x28))(0);
    }
    (**(code **)(**(int **)(param_1 + 0xe0) + 0x2c))(0);
    (**(code **)(**(int **)(param_1 + 0xe0) + 0x24))
              (0,0,*(undefined4 *)(param_1 + 0x10),*(undefined4 *)(param_1 + 0x14));
    (**(code **)(**(int **)(param_1 + 0xe0) + 0x28))(0);
    (**(code **)(**(int **)(param_1 + 0xf8) + 0x44))();
    (**(code **)(**(int **)(param_1 + 0xe0) + 0x2c))(1);
    if (DAT_0092e640 != 0) {
      FUN_004b7f0d();
    }
    if (DAT_006703a8 != 0) {
      FUN_004afff7();
    }
  }
  return 1;
}


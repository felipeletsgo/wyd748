// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004481c5 | Name: FUN_004481c5


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __thiscall FUN_004481c5(int param_1,int param_2)

{
  int *piVar1;
  int *piVar2;
  int *piVar3;
  int *piVar4;
  int iVar5;
  int iVar6;
  undefined4 uVar7;
  undefined4 uVar8;
  
  DAT_005b12bc = 0xffff;
  piVar1 = *(int **)(param_1 + 0x285d4);
  piVar2 = *(int **)(param_1 + 0x27b34);
  piVar3 = *(int **)(param_1 + 0x27c7c);
  piVar4 = *(int **)(param_1 + 0x285d8);
  iVar6 = *(int *)(param_1 + 0x285f0);
  iVar5 = *(int *)(param_1 + 0x27b38);
  if (DAT_005b892c == 2) {
    if (param_2 == 0) {
      *(undefined4 *)(*(int *)(param_1 + 0x2873c) + 0x400) = 0;
      FUN_00447f6f(1);
    }
    else {
      (**(code **)(*DAT_005ccec0 + 0x98))();
      (**(code **)(*piVar3 + 0x60))(0);
      (**(code **)(*piVar4 + 0x60))(0);
      if (iVar5 != 0) {
        iVar5 = FUN_0040c0f0();
        if (iVar5 == 1) {
          FUN_0044ae38(0,0);
        }
      }
      if (iVar6 != 0) {
        iVar6 = FUN_0040c0f0();
        if (iVar6 == 1) {
          FUN_0044b890(0);
        }
      }
      *(undefined4 *)(*(int *)(param_1 + 0x2873c) + 0x400) = 3;
      FUN_00447f6f(0);
    }
    (**(code **)(*piVar1 + 0x60))(param_2);
    if (param_2 != 0) {
      (**(code **)(*piVar2 + 0x70))
                (*(float *)(*(int *)(param_1 + 0x27b34) + 0x54) * _DAT_005a34a0 +
                 (float)*(uint *)(DAT_005ccf9c + 0x2a504) * _DAT_005a34a0 + _DAT_005a3670,
                 (float)*(uint *)(DAT_005ccf9c + 0x2a508) * _DAT_005a34a0 -
                 *(float *)(*(int *)(param_1 + 0x27b34) + 0x58) * _DAT_005a34a0);
    }
    (**(code **)(*piVar2 + 0x60))(param_2);
  }
  else if (param_2 == 1) {
    (**(code **)(*DAT_005ccec0 + 0x98))();
    (**(code **)(**(int **)(param_1 + 0x27c70) + 0x60))(0);
    (**(code **)(*piVar3 + 0x60))(0);
    (**(code **)(*piVar4 + 0x60))(0);
    if (iVar5 != 0) {
      iVar5 = FUN_0040c0f0();
      if (iVar5 == 1) {
        FUN_0044ae38(0,0);
      }
    }
    if (iVar6 != 0) {
      iVar6 = FUN_0040c0f0();
      if (iVar6 == 1) {
        FUN_0044b890(0);
      }
    }
    (**(code **)(*piVar1 + 0x60))(1);
    (**(code **)(*piVar2 + 0x60))(1);
    uVar7 = __ftol();
    *(undefined4 *)(DAT_005ccf9c + 0x2a5e4) = uVar7;
    *(undefined4 *)(*(int *)(param_1 + 0x2873c) + 0x400) = 3;
    FUN_00447f6f(0);
  }
  else {
    (**(code **)(*piVar1 + 0x60))(param_2);
    (**(code **)(*piVar2 + 0x60))(param_2);
    *(undefined4 *)(DAT_005ccf9c + 0x2a5e4) = 0;
    *(undefined4 *)(*(int *)(param_1 + 0x2873c) + 0x400) = 0;
    FUN_00447f6f(1);
  }
  if (DAT_005ccf98 != 0) {
    iVar6 = FUN_00429a6d(0x33);
    if (iVar6 != 0) {
      uVar8 = 0;
      uVar7 = 0;
      FUN_00429a6d(0x33);
      FUN_0042ad2b(uVar7,uVar8);
    }
  }
  return;
}


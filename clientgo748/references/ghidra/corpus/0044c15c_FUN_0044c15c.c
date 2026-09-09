// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0044c15c | Name: FUN_0044c15c


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __fastcall FUN_0044c15c(int param_1)

{
  int *piVar1;
  int *piVar2;
  int *piVar3;
  int *piVar4;
  int *piVar5;
  int *piVar6;
  int *piVar7;
  int *piVar8;
  int iVar9;
  int iVar10;
  int iVar11;
  undefined4 uVar12;
  undefined4 uVar13;
  
  DAT_005b12bc = 0xffff;
  piVar1 = *(int **)(param_1 + 0x285d8);
  iVar11 = *(int *)(param_1 + 0x285f0);
  iVar10 = *(int *)(param_1 + 0x27b38);
  piVar2 = *(int **)(param_1 + 0x27b34);
  piVar3 = *(int **)(param_1 + 0x27c7c);
  piVar4 = *(int **)(param_1 + 0x285d4);
  piVar5 = *(int **)(param_1 + 0x285dc);
  piVar6 = *(int **)(param_1 + 0x27c84);
  piVar7 = *(int **)(param_1 + 0x27bdc);
  piVar8 = *(int **)(param_1 + 0x27be4);
  iVar9 = FUN_0040c0f0();
  if (DAT_005b892c == 2) {
    (**(code **)(*piVar6 + 0x60))(piVar5[10] == 0);
    if (iVar9 != 0) {
      (**(code **)(*piVar5 + 0x60))(0);
      (**(code **)(*piVar6 + 0x60))(0);
      *(undefined4 *)(DAT_005ccf9c + 0x2a5e4) = 0;
      (**(code **)(*piVar6 + 0x70))
                ((float)*(uint *)(DAT_005ccf9c + 0x2a504) * _DAT_005a34a0 -
                 (float)piVar6[0x15] * _DAT_005a34a0,
                 (float)*(uint *)(DAT_005ccf9c + 0x2a508) * _DAT_005a34a0 -
                 (float)piVar6[0x16] * _DAT_005a34a0);
    }
    else {
      (**(code **)(**(int **)(param_1 + 0x27c70) + 0x60))(0);
      (**(code **)(*piVar1 + 0x60))(0);
      if ((iVar10 != 0) && (iVar10 = FUN_0040c0f0(), iVar10 == 1)) {
        FUN_0044ae38(0,0);
      }
      if ((iVar11 != 0) && (iVar11 = FUN_0040c0f0(), iVar11 == 1)) {
        FUN_0044b890(0);
      }
      (**(code **)(*piVar2 + 0x60))(0);
      (**(code **)(*piVar3 + 0x60))(0);
      (**(code **)(*piVar4 + 0x60))(0);
      (**(code **)(*piVar5 + 0x60))(1);
      (**(code **)(*piVar6 + 0x60))(1);
      (**(code **)(*piVar7 + 0x60))(0);
      (**(code **)(*piVar8 + 0x60))(0);
      (**(code **)(*piVar6 + 0x70))
                ((float)piVar6[0x15] * _DAT_005a34a0 +
                 (float)*(uint *)(DAT_005ccf9c + 0x2a504) * _DAT_005a34a0 + _DAT_005a3670,
                 (float)*(uint *)(DAT_005ccf9c + 0x2a508) * _DAT_005a34a0 -
                 (float)piVar6[0x16] * _DAT_005a34a0);
    }
  }
  else if (iVar9 != 0) {
    (**(code **)(*piVar5 + 0x60))(0);
    (**(code **)(*piVar6 + 0x60))(0);
    *(undefined4 *)(DAT_005ccf9c + 0x2a5e4) = 0;
  }
  else {
    (**(code **)(**(int **)(param_1 + 0x27c70) + 0x60))(0);
    (**(code **)(*piVar1 + 0x60))(0);
    if ((iVar10 != 0) && (iVar10 = FUN_0040c0f0(), iVar10 == 1)) {
      FUN_0044ae38(0,0);
    }
    if ((iVar11 != 0) && (iVar11 = FUN_0040c0f0(), iVar11 == 1)) {
      FUN_0044b890(0);
    }
    (**(code **)(*piVar2 + 0x60))(0);
    (**(code **)(*piVar3 + 0x60))(0);
    (**(code **)(*piVar4 + 0x60))(0);
    (**(code **)(*piVar7 + 0x60))(0);
    (**(code **)(*piVar5 + 0x60))(1);
    (**(code **)(*piVar6 + 0x60))(1);
    (**(code **)(*piVar8 + 0x60))(0);
    uVar12 = __ftol();
    *(undefined4 *)(DAT_005ccf9c + 0x2a5e4) = uVar12;
  }
  if ((DAT_005ccf98 != 0) && (iVar11 = FUN_00429a6d(0x33), iVar11 != 0)) {
    uVar13 = 0;
    uVar12 = 0;
    FUN_00429a6d(0x33);
    FUN_0042ad2b(uVar12,uVar13);
  }
  return;
}


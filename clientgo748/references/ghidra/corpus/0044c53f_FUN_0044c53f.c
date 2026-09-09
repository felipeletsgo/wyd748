// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0044c53f | Name: FUN_0044c53f


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __fastcall FUN_0044c53f(int param_1)

{
  int iVar1;
  int *piVar2;
  int iVar3;
  int *piVar4;
  int *piVar5;
  int *piVar6;
  int *piVar7;
  int *piVar8;
  int iVar9;
  int *piVar10;
  int iVar11;
  bool bVar12;
  undefined4 uVar13;
  undefined4 uVar14;
  
  DAT_005b12bc = 0xffff;
  piVar10 = *(int **)(param_1 + 0x285d8);
  iVar11 = *(int *)(param_1 + 0x27b38);
  iVar1 = *(int *)(param_1 + 0x285f0);
  piVar2 = *(int **)(param_1 + 0x27b34);
  iVar3 = *(int *)(param_1 + 0x27c7c);
  piVar4 = *(int **)(param_1 + 0x285d4);
  piVar5 = *(int **)(param_1 + 0x285dc);
  piVar6 = *(int **)(param_1 + 0x27c84);
  piVar7 = *(int **)(param_1 + 0x27bdc);
  piVar8 = *(int **)(param_1 + 0x27be4);
  iVar9 = *(int *)(iVar3 + 0x28);
  bVar12 = piVar6[10] == 0;
  if (DAT_005b892c == 2) {
    (**(code **)(*piVar6 + 0x60))(piVar6[10] == 0);
    if (bVar12) {
      (**(code **)(*piVar6 + 0x70))
                ((float)*(uint *)(DAT_005ccf9c + 0x2a504) * _DAT_005a34a0 -
                 (float)piVar6[0x15] * _DAT_005a34a0,
                 (float)*(uint *)(DAT_005ccf9c + 0x2a508) * _DAT_005a34a0 -
                 (float)piVar6[0x16] * _DAT_005a34a0);
    }
    else {
      (**(code **)(*piVar4 + 0x60))(0);
      (**(code **)(*piVar10 + 0x60))(0);
      (**(code **)(*piVar7 + 0x60))(0);
      (**(code **)(*piVar8 + 0x60))(0);
      (**(code **)(*piVar5 + 0x60))(0);
      (**(code **)(**(int **)(param_1 + 0x58) + 0x60))(0);
      (**(code **)(*DAT_005ccec0 + 0x98))();
    }
  }
  else {
    if (bVar12) {
      (**(code **)(**(int **)(param_1 + 0x27c70) + 0x60))(0);
      (**(code **)(*piVar2 + 0x60))(0);
      if ((iVar11 != 0) && (iVar11 = FUN_0040c0f0(), iVar11 == 1)) {
        FUN_0044ae38(0,0);
      }
      if ((iVar1 != 0) && (iVar11 = FUN_0040c0f0(), iVar11 == 1)) {
        FUN_0044b890(0);
      }
      if (iVar9 == 1) {
        uVar13 = __ftol();
        *(undefined4 *)(DAT_005ccf9c + 0x2a5e4) = uVar13;
        FUN_004012c6((float)*(uint *)(DAT_005ccf9c + 0x2a504) -
                     (float)*(int *)(DAT_005ccf9c + 0x2a5e4),*(undefined4 *)(iVar3 + 0x50));
      }
      else {
        uVar13 = __ftol();
        *(undefined4 *)(DAT_005ccf9c + 0x2a5e4) = uVar13;
      }
    }
    else {
      (**(code **)(*piVar4 + 0x60))(0);
      (**(code **)(*piVar10 + 0x60))(0);
      (**(code **)(*piVar7 + 0x60))(0);
      (**(code **)(*piVar8 + 0x60))(0);
      (**(code **)(*piVar5 + 0x60))(0);
      (**(code **)(**(int **)(param_1 + 0x58) + 0x60))(0);
      if (iVar9 == 1) {
        uVar13 = __ftol();
        *(undefined4 *)(DAT_005ccf9c + 0x2a5e4) = uVar13;
        FUN_004012c6((float)*(uint *)(DAT_005ccf9c + 0x2a504) -
                     (float)*(int *)(DAT_005ccf9c + 0x2a5e4),*(undefined4 *)(iVar3 + 0x50));
      }
      else {
        *(undefined4 *)(DAT_005ccf9c + 0x2a5e4) = 0;
      }
      (**(code **)(*DAT_005ccec0 + 0x98))();
    }
    piVar10 = (int *)(**(code **)(**(int **)(param_1 + 0x28) + 0x48))(0x126);
    (**(code **)(*piVar10 + 0x8c))(piVar2[10]);
    piVar10 = (int *)(**(code **)(**(int **)(param_1 + 0x28) + 0x48))(0x127);
    (**(code **)(*piVar10 + 0x8c))(bVar12);
    (**(code **)(*piVar6 + 0x60))(bVar12);
  }
  if ((DAT_005ccf98 != 0) && (iVar11 = FUN_00429a6d(0x33), iVar11 != 0)) {
    uVar14 = 0;
    uVar13 = 0;
    FUN_00429a6d(0x33);
    FUN_0042ad2b(uVar13,uVar14);
  }
  return;
}


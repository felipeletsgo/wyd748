// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00447691 | Name: FUN_00447691


void __fastcall FUN_00447691(int param_1)

{
  int *piVar1;
  int *piVar2;
  int *piVar3;
  int *piVar4;
  int *piVar5;
  int *piVar6;
  int *piVar7;
  int *piVar8;
  int *piVar9;
  int *piVar10;
  int *piVar11;
  int iVar12;
  int iVar13;
  int iVar14;
  int *piVar15;
  int iVar16;
  bool bVar17;
  undefined4 uVar18;
  undefined4 uVar19;
  
  DAT_005b12bc = 0xffff;
  piVar15 = *(int **)(param_1 + 0x285d4);
  piVar1 = *(int **)(param_1 + 0x285d8);
  piVar2 = *(int **)(param_1 + 0x27b34);
  iVar16 = *(int *)(param_1 + 0x27c7c);
  piVar3 = *(int **)(param_1 + 0x27c84);
  piVar4 = *(int **)(param_1 + 0x285dc);
  iVar14 = *(int *)(param_1 + 0x27b38);
  piVar5 = *(int **)(param_1 + 0x27b6c);
  piVar6 = *(int **)(param_1 + 0x27b90);
  piVar7 = *(int **)(param_1 + 0x27bb4);
  piVar8 = *(int **)(param_1 + 0x27be8);
  piVar9 = *(int **)(param_1 + 0x27c10);
  piVar10 = *(int **)(param_1 + 0x27c38);
  piVar11 = *(int **)(param_1 + 0x27bdc);
  iVar12 = *(int *)(param_1 + 0x27be4);
  iVar13 = FUN_0040c0f0();
  if (iVar13 == 1) {
    piVar2[10] = 0;
  }
  iVar13 = FUN_0040c0f0();
  if (iVar13 == 1) {
    *(undefined4 *)(iVar16 + 0x28) = 0;
  }
  iVar13 = *(int *)(iVar16 + 0x28);
  bVar17 = piVar2[10] == 0;
  if ((iVar14 != 0) && (iVar14 = FUN_0040c0f0(), iVar14 == 1)) {
    FUN_0044ae38(0,0);
  }
  if ((iVar12 != 0) && (iVar14 = FUN_0040c0f0(), iVar14 == 1)) {
    FUN_0044a745(0,0);
  }
  if (DAT_005b892c == 2) {
    (**(code **)(*piVar2 + 0x60))(!bVar17);
    if (!bVar17) {
      FUN_004487e2();
      FUN_004489c5();
      FUN_00448c38();
      FUN_00448e0b();
      FUN_00448fde();
      FUN_004491b1();
      (**(code **)(*piVar1 + 0x60))(0);
      (**(code **)(*piVar15 + 0x60))(0);
      (**(code **)(*piVar5 + 0x60))(0);
      (**(code **)(*piVar6 + 0x60))(0);
      (**(code **)(*piVar7 + 0x60))(0);
      (**(code **)(*piVar8 + 0x60))(0);
      if (piVar9 != (int *)0x0) {
        (**(code **)(*piVar9 + 0x60))(0);
      }
      if (piVar10 != (int *)0x0) {
        (**(code **)(*piVar10 + 0x60))(0);
      }
      (**(code **)(*piVar11 + 0x60))(0);
      FUN_00447f47();
      (**(code **)(**(int **)(param_1 + 0x27c78) + 0x60))(0);
      iVar16 = FUN_0040c0f0();
      if (iVar16 == 1) {
        FUN_0044b890(0);
      }
      (**(code **)(*DAT_005ccec0 + 0x98))();
    }
  }
  else if (bVar17) {
    (**(code **)(*DAT_005ccec0 + 0x98))();
    (**(code **)(**(int **)(param_1 + 0x27c70) + 0x60))(0);
    (**(code **)(*piVar3 + 0x60))(0);
    if (iVar13 == 1) {
      uVar18 = __ftol();
      *(undefined4 *)(DAT_005ccf9c + 0x2a5e4) = uVar18;
      FUN_004012c6((float)*(uint *)(DAT_005ccf9c + 0x2a504) -
                   (float)*(int *)(DAT_005ccf9c + 0x2a5e4),*(undefined4 *)(iVar16 + 0x50));
    }
    else {
      iVar16 = FUN_0040c0f0();
      if (iVar16 == 0) {
        uVar18 = __ftol();
        *(undefined4 *)(DAT_005ccf9c + 0x2a5e4) = uVar18;
      }
    }
  }
  else {
    FUN_004487e2();
    FUN_004489c5();
    FUN_00448c38();
    FUN_00448e0b();
    FUN_00448fde();
    FUN_004491b1();
    (**(code **)(**(int **)(param_1 + 0x27b6c) + 0x60))(0);
    (**(code **)(**(int **)(param_1 + 0x27b90) + 0x60))(0);
    (**(code **)(**(int **)(param_1 + 0x27bb4) + 0x60))(0);
    (**(code **)(**(int **)(param_1 + 0x27be8) + 0x60))(0);
    if (*(int *)(param_1 + 0x27c10) != 0) {
      (**(code **)(**(int **)(param_1 + 0x27c10) + 0x60))(0);
    }
    if (*(int *)(param_1 + 0x27c38) != 0) {
      (**(code **)(**(int **)(param_1 + 0x27c38) + 0x60))(0);
    }
    if (*(int *)(param_1 + 0x278b4) != 0) {
      (**(code **)(**(int **)(param_1 + 0x278b4) + 0x60))(0);
    }
    (**(code **)(*piVar4 + 0x60))(0);
    (**(code **)(*piVar15 + 0x60))(0);
    (**(code **)(**(int **)(param_1 + 0x27bdc) + 0x60))(0);
    FUN_00447f47();
    (**(code **)(*piVar1 + 0x60))(0);
    (**(code **)(**(int **)(param_1 + 0x58) + 0x60))(0);
    (**(code **)(**(int **)(param_1 + 0x27c78) + 0x60))(0);
    iVar14 = FUN_0040c0f0();
    if (iVar14 == 1) {
      FUN_0044b890(0);
    }
    if (iVar13 == 1) {
      uVar18 = __ftol();
      *(undefined4 *)(DAT_005ccf9c + 0x2a5e4) = uVar18;
      FUN_004012c6((float)*(uint *)(DAT_005ccf9c + 0x2a504) -
                   (float)*(int *)(DAT_005ccf9c + 0x2a5e4),*(undefined4 *)(iVar16 + 0x50));
    }
    else {
      *(undefined4 *)(DAT_005ccf9c + 0x2a5e4) = 0;
    }
    (**(code **)(*DAT_005ccec0 + 0x98))();
  }
  piVar15 = (int *)(**(code **)(**(int **)(param_1 + 0x28) + 0x48))(0x127);
  (**(code **)(*piVar15 + 0x8c))(piVar3[10]);
  (**(code **)(*piVar2 + 0x60))(bVar17);
  if (((DAT_005ccf98 != 0) && (DAT_005ccf98 != 0)) && (iVar16 = FUN_00429a6d(0x33), iVar16 != 0)) {
    uVar19 = 0;
    uVar18 = 0;
    FUN_00429a6d(0x33);
    FUN_0042ad2b(uVar18,uVar19);
  }
  return;
}


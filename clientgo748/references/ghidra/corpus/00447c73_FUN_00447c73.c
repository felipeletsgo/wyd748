// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00447c73 | Name: FUN_00447c73


void __fastcall FUN_00447c73(int param_1)

{
  int *piVar1;
  int *piVar2;
  int *piVar3;
  int *piVar4;
  int *piVar5;
  int iVar6;
  int iVar7;
  int iVar8;
  int iVar9;
  int iVar10;
  bool bVar11;
  undefined4 uVar12;
  undefined4 uVar13;
  
  DAT_005b12bc = 0xffff;
  piVar1 = *(int **)(param_1 + 0x285d4);
  piVar2 = *(int **)(param_1 + 0x27c7c);
  piVar3 = *(int **)(param_1 + 0x285d8);
  piVar4 = *(int **)(param_1 + 0x285dc);
  iVar10 = *(int *)(param_1 + 0x27c84);
  iVar9 = *(int *)(param_1 + 0x285f0);
  piVar5 = *(int **)(param_1 + 0x27bdc);
  iVar6 = *(int *)(param_1 + 0x27be4);
  iVar7 = *(int *)(*(int *)(param_1 + 0x27b34) + 0x28);
  bVar11 = piVar2[10] == 0;
  if (DAT_005b892c == 2) {
    (**(code **)(*piVar2 + 0x60))(bVar11);
  }
  else {
    if ((*(int *)(param_1 + 0x27b38) != 0) && (iVar8 = FUN_0040c0f0(), iVar8 == 1)) {
      FUN_0044ae38(0,0);
    }
    if ((iVar9 != 0) && (iVar9 = FUN_0040c0f0(), iVar9 == 1)) {
      FUN_0044b890(0);
    }
    if ((iVar6 != 0) && (iVar9 = FUN_0040c0f0(), iVar9 == 1)) {
      FUN_0044a745(0,0);
    }
    if (bVar11) {
      (**(code **)(*DAT_005ccec0 + 0x98))();
      (**(code **)(**(int **)(param_1 + 0x27c70) + 0x60))(0);
      if ((iVar7 == 1) || (*(int *)(iVar10 + 0x28) == 1)) {
        (**(code **)(*piVar3 + 0x60))(0);
        (**(code **)(*piVar1 + 0x60))(0);
        FUN_00447f47();
        (**(code **)(*piVar4 + 0x60))(0);
        (**(code **)(*piVar5 + 0x60))(0);
        uVar12 = __ftol();
        *(undefined4 *)(DAT_005ccf9c + 0x2a5e4) = uVar12;
        FUN_004012c6((float)*(uint *)(DAT_005ccf9c + 0x2a504) -
                     (float)*(int *)(DAT_005ccf9c + 0x2a5e4),piVar2[0x14]);
      }
      else {
        uVar12 = __ftol();
        *(undefined4 *)(DAT_005ccf9c + 0x2a5e4) = uVar12;
        FUN_004012c6((float)*(uint *)(DAT_005ccf9c + 0x2a504) -
                     (float)*(int *)(DAT_005ccf9c + 0x2a5e4),piVar2[0x14]);
      }
    }
    else if ((iVar7 == 1) || (*(int *)(iVar10 + 0x28) == 1)) {
      uVar12 = __ftol();
      *(undefined4 *)(DAT_005ccf9c + 0x2a5e4) = uVar12;
    }
    else {
      *(undefined4 *)(DAT_005ccf9c + 0x2a5e4) = 0;
    }
  }
  (**(code **)(*piVar2 + 0x60))(bVar11);
  if ((DAT_005ccf98 != 0) && (iVar10 = FUN_00429a6d(0x33), iVar10 != 0)) {
    uVar13 = 0;
    uVar12 = 0;
    FUN_00429a6d(0x33);
    FUN_0042ad2b(uVar12,uVar13);
  }
  return;
}


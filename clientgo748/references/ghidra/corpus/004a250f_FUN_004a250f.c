// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004a250f | Name: FUN_004a250f


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __thiscall FUN_004a250f(int param_1,int param_2)

{
  int *piVar1;
  int *piVar2;
  int *piVar3;
  int *piVar4;
  int *piVar5;
  int *piVar6;
  int iVar7;
  
  *(int *)(param_1 + 0x26ef4) = param_2;
  if (param_2 == 1) {
    FUN_004a24b4(s_UI_SelCamAction_005b9394);
    piVar1 = (int *)(**(code **)(**(int **)(param_1 + 0x28) + 0x48))(0x501);
    piVar2 = (int *)(**(code **)(**(int **)(param_1 + 0x28) + 0x48))(0x502);
    if (piVar1 != (int *)0x0) {
      (**(code **)(*piVar1 + 0x60))(1);
    }
    if (DAT_005b892c == 2) {
      (**(code **)(**(int **)(param_1 + 0x26ed4) + 0x60))(1);
      (**(code **)(**(int **)(param_1 + 0x26ed8) + 0x60))(1);
      (**(code **)(**(int **)(param_1 + 0x26ed4) + 0x70))
                (0x41f00000,(float)*(uint *)(DAT_005ccf9c + 0x2a508) - _DAT_005a4408);
      (**(code **)(**(int **)(param_1 + 0x26ed8) + 0x70))
                (0x41f00000,(float)*(uint *)(DAT_005ccf9c + 0x2a508) - _DAT_005a452c);
      (**(code **)(**(int **)(param_1 + 0x26edc) + 0x70))
                (0x41f00000,(float)*(uint *)(DAT_005ccf9c + 0x2a508) - _DAT_005a3c68);
      (**(code **)(*piVar2 + 0x70))
                (((float)*(uint *)(DAT_005ccf9c + 0x2a504) * _DAT_005a4150 -
                 (float)piVar2[0x15] * _DAT_005a34a0) +
                 (float)*(uint *)(DAT_005ccf9c + 0x2a504) * _DAT_005a34a0,
                 (float)*(uint *)(DAT_005ccf9c + 0x2a508) * _DAT_005a34a0 -
                 (float)piVar2[0x16] * _DAT_005a34a0);
      if (piVar2 != (int *)0x0) {
        (**(code **)(*piVar2 + 0x60))(0);
      }
    }
    else {
      (**(code **)(*piVar2 + 0x70))(0x435a0000,0x4408c000);
      if (piVar2 != (int *)0x0) {
        (**(code **)(*piVar2 + 0x60))(1);
      }
    }
    piVar1 = (int *)(**(code **)(**(int **)(param_1 + 0x28) + 0x48))(0x601);
    piVar2 = (int *)(**(code **)(**(int **)(param_1 + 0x28) + 0x48))(0x602);
    piVar3 = (int *)(**(code **)(**(int **)(param_1 + 0x28) + 0x48))(0x603);
    piVar4 = (int *)(**(code **)(**(int **)(param_1 + 0x28) + 0x48))(0x604);
    piVar5 = (int *)(**(code **)(**(int **)(param_1 + 0x28) + 0x48))(0x605);
    piVar6 = (int *)(**(code **)(**(int **)(param_1 + 0x28) + 0x48))(0x606);
    if (piVar1 != (int *)0x0) {
      (**(code **)(*piVar1 + 0x60))(0);
    }
    if (piVar2 != (int *)0x0) {
      (**(code **)(*piVar2 + 0x60))(0);
    }
    if (piVar3 != (int *)0x0) {
      (**(code **)(*piVar3 + 0x60))(0);
    }
    if (piVar4 != (int *)0x0) {
      (**(code **)(*piVar4 + 0x60))(0);
    }
    if (piVar5 != (int *)0x0) {
      (**(code **)(*piVar5 + 0x60))(0);
    }
    if (piVar6 != (int *)0x0) {
      (**(code **)(*piVar6 + 0x60))(0);
    }
  }
  else {
    FUN_004a24b4(s_UI_CreCamAction_005b93a4);
    piVar1 = (int *)(**(code **)(**(int **)(param_1 + 0x28) + 0x48))(0x501);
    piVar2 = (int *)(**(code **)(**(int **)(param_1 + 0x28) + 0x48))(0x502);
    if (piVar1 != (int *)0x0) {
      (**(code **)(*piVar1 + 0x60))(0);
    }
    if (piVar2 != (int *)0x0) {
      (**(code **)(*piVar2 + 0x60))(0);
    }
    piVar1 = (int *)(**(code **)(**(int **)(param_1 + 0x28) + 0x48))(0x601);
    piVar2 = (int *)(**(code **)(**(int **)(param_1 + 0x28) + 0x48))(0x602);
    piVar3 = (int *)(**(code **)(**(int **)(param_1 + 0x28) + 0x48))(0x603);
    piVar4 = (int *)(**(code **)(**(int **)(param_1 + 0x28) + 0x48))(0x604);
    piVar5 = (int *)(**(code **)(**(int **)(param_1 + 0x28) + 0x48))(0x605);
    piVar6 = (int *)(**(code **)(**(int **)(param_1 + 0x28) + 0x48))(0x606);
    if (DAT_005b892c == 2) {
      (**(code **)(**(int **)(param_1 + 0x26ed4) + 0x60))(0);
      (**(code **)(**(int **)(param_1 + 0x26ed8) + 0x60))(0);
      (**(code **)(*piVar6 + 0x70))
                (((float)*(uint *)(DAT_005ccf9c + 0x2a504) * _DAT_005a4150 -
                 (float)piVar6[0x15] * _DAT_005a34a0) +
                 (float)*(uint *)(DAT_005ccf9c + 0x2a504) * _DAT_005a34a0,
                 (float)*(uint *)(DAT_005ccf9c + 0x2a508) * _DAT_005a34a0 -
                 (float)piVar6[0x16] * _DAT_005a34a0);
    }
    else {
      (**(code **)(*piVar6 + 0x70))(0x435a0000,0x44160000);
      if (piVar6 != (int *)0x0) {
        (**(code **)(*piVar6 + 0x60))(1);
      }
    }
    if (piVar1 != (int *)0x0) {
      (**(code **)(*piVar1 + 0x60))(1);
    }
    if (piVar2 != (int *)0x0) {
      (**(code **)(*piVar2 + 0x60))(1);
    }
    if (piVar3 != (int *)0x0) {
      (**(code **)(*piVar3 + 0x60))(1);
    }
    if (piVar4 != (int *)0x0) {
      (**(code **)(*piVar4 + 0x60))(1);
    }
    if (piVar5 != (int *)0x0) {
      (**(code **)(*piVar5 + 0x60))(1);
    }
    iVar7 = (**(code **)(**(int **)(param_1 + 0x28) + 0x48))(0x1212);
    if (iVar7 != 0) {
      (**(code **)(**(int **)(param_1 + 0x28) + 0x40))(iVar7);
    }
  }
  return;
}


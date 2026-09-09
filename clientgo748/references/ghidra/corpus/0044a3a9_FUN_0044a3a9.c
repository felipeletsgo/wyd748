// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0044a3a9 | Name: FUN_0044a3a9


void FUN_0044a3a9(int param_1)

{
  int iVar1;
  undefined4 uVar2;
  undefined4 uVar3;
  int extraout_var;
  int iVar4;
  int *piVar5;
  int local_8;
  
  iVar1 = FUN_0040c0f0();
  iVar4 = extraout_var;
  if (iVar1 == 1) {
    FUN_00447594();
  }
  if (((param_1 == 1) && (*(int *)(iVar4 + 0x27c84) != 0)) &&
     (*(int *)(*(int *)(iVar4 + 0x27c84) + 0x28) == 1)) {
    FUN_0044c53f();
  }
  if (((param_1 == 1) && (*(int *)(iVar4 + 0x27c7c) != 0)) &&
     (*(int *)(*(int *)(iVar4 + 0x27c7c) + 0x28) == 1)) {
    FUN_00447c73();
  }
  if (((param_1 == 1) && (*(int *)(iVar4 + 0x285d8) != 0)) &&
     (*(int *)(*(int *)(iVar4 + 0x285d8) + 0x28) == 1)) {
    FUN_004484f3(1);
  }
  if (((param_1 == 1) && (*(int *)(iVar4 + 0x27b38) != 0)) &&
     (*(int *)(*(int *)(iVar4 + 0x27b38) + 0x28) == 1)) {
    FUN_0044ae38(0,0);
  }
  if (DAT_005b892c == 2) {
    if (param_1 == 1) {
      if ((*(int *)(iVar4 + 0x27b34) != 0) && (*(int *)(*(int *)(iVar4 + 0x27b34) + 0x28) == 0)) {
        FUN_00447691();
      }
      if (*(int *)(iVar4 + 0x27be0) != 0) {
        FUN_0049e50f(*(undefined4 *)(iVar4 + 0x27be0),s_UI_hellStoredesc_txt_005b7ce8,0,0x14);
      }
      uVar3 = 1;
      (**(code **)(**(int **)(iVar4 + 0x27bdc) + 0x60))(1);
      *(undefined4 *)(DAT_005ccf9c + 0x2a5e4) = 0;
      *(undefined4 *)(*(int *)(iVar4 + 0x2873c) + 0x400) = 3;
      FUN_00447f6f(0);
    }
    else {
      if ((*(int *)(iVar4 + 0x27b34) != 0) && (*(int *)(*(int *)(iVar4 + 0x27b34) + 0x28) == 1)) {
        FUN_00447691();
      }
      *(undefined4 *)(DAT_005ccf9c + 0x2a5e4) = 0;
      uVar3 = 0;
      (**(code **)(**(int **)(iVar4 + 0x27bdc) + 0x60))(0);
      *(undefined4 *)(iVar4 + 0x275d8) = 0;
      *(undefined4 *)(iVar4 + 0x275dc) = 0;
      *(undefined4 *)(*(int *)(iVar4 + 0x2873c) + 0x400) = 0;
      FUN_00447f6f(1);
    }
  }
  else if (param_1 == 1) {
    if ((*(int *)(iVar4 + 0x27b34) != 0) && (*(int *)(*(int *)(iVar4 + 0x27b34) + 0x28) == 0)) {
      FUN_00447691();
    }
    if (*(int *)(iVar4 + 0x27be0) != 0) {
      FUN_0049e50f(*(undefined4 *)(iVar4 + 0x27be0),s_UI_hellStoredesc_txt_005b7d00,0,0x14);
    }
    uVar3 = 1;
    (**(code **)(**(int **)(iVar4 + 0x27bdc) + 0x60))(1);
    uVar2 = __ftol();
    *(undefined4 *)(DAT_005ccf9c + 0x2a5e4) = uVar2;
    *(undefined4 *)(*(int *)(iVar4 + 0x2873c) + 0x400) = 3;
    FUN_00447f6f(0);
  }
  else {
    if ((*(int *)(iVar4 + 0x27b34) != 0) && (*(int *)(*(int *)(iVar4 + 0x27b34) + 0x28) == 1)) {
      FUN_00447691();
    }
    *(undefined4 *)(DAT_005ccf9c + 0x2a5e4) = 0;
    uVar3 = 0;
    (**(code **)(**(int **)(iVar4 + 0x27bdc) + 0x60))(0);
    *(undefined4 *)(iVar4 + 0x275d8) = 0;
    *(undefined4 *)(iVar4 + 0x275dc) = 0;
    *(undefined4 *)(*(int *)(iVar4 + 0x2873c) + 0x400) = 0;
    FUN_00447f6f(1);
  }
  if (param_1 == 0) {
    uVar2 = *(undefined4 *)(iVar4 + 0x27bdc);
    piVar5 = (int *)0x0;
    for (local_8 = 0; local_8 < 4; local_8 = local_8 + 1) {
      piVar5 = (int *)(**(code **)(**(int **)(iVar4 + 0x28) + 0x48))
                                (local_8 + 0x1831,uVar3,iVar4,piVar5,uVar2);
      if (piVar5 != (int *)0x0) {
        (**(code **)(*piVar5 + 0x8c))(0);
      }
    }
  }
  return;
}


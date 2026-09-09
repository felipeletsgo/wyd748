// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00426ca6 | Name: FUN_00426ca6


undefined4 __thiscall
FUN_00426ca6(int param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4,
            undefined4 *param_5)

{
  int iVar1;
  
  if ((((*(uint *)(param_1 + 0x2a5a8) < 0x11) && (*(int *)(param_1 + 0x2a5ac) == 0)) &&
      (iVar1 = (**(code **)(**(int **)(param_1 + 0x2a3e0) + 0x28))
                         (*(undefined4 *)(param_1 + 0x2a3e0),param_2,param_3,param_4,2,1,0x50),
      -1 < iVar1)) &&
     (iVar1 = (**(code **)(**(int **)(param_1 + 0x2a3e0) + 0x30))
                        (*(undefined4 *)(param_1 + 0x2a3e0),param_2,param_3,param_4,param_4,0x50),
     -1 < iVar1)) {
    *param_5 = 0x50;
    return 1;
  }
  if (((*(uint *)(param_1 + 0x2a5a8) < 0x10) && (*(uint *)(param_1 + 0x2a5ac) < 2)) &&
     ((iVar1 = (**(code **)(**(int **)(param_1 + 0x2a3e0) + 0x28))
                         (*(undefined4 *)(param_1 + 0x2a3e0),param_2,param_3,param_4,2,1,0x49),
      -1 < iVar1 &&
      (iVar1 = (**(code **)(**(int **)(param_1 + 0x2a3e0) + 0x30))
                         (*(undefined4 *)(param_1 + 0x2a3e0),param_2,param_3,param_4,param_4,0x49),
      -1 < iVar1)))) {
    *param_5 = 0x49;
    return 1;
  }
  if (((*(uint *)(param_1 + 0x2a5a8) < 0x19) && (*(int *)(param_1 + 0x2a5ac) == 0)) &&
     ((iVar1 = (**(code **)(**(int **)(param_1 + 0x2a3e0) + 0x28))
                         (*(undefined4 *)(param_1 + 0x2a3e0),param_2,param_3,param_4,2,1,0x4d),
      -1 < iVar1 &&
      (iVar1 = (**(code **)(**(int **)(param_1 + 0x2a3e0) + 0x30))
                         (*(undefined4 *)(param_1 + 0x2a3e0),param_2,param_3,param_4,param_4,0x4d),
      -1 < iVar1)))) {
    *param_5 = 0x4d;
    return 1;
  }
  if ((((*(uint *)(param_1 + 0x2a5a8) < 0x19) && (*(uint *)(param_1 + 0x2a5ac) < 9)) &&
      (iVar1 = (**(code **)(**(int **)(param_1 + 0x2a3e0) + 0x28))
                         (*(undefined4 *)(param_1 + 0x2a3e0),param_2,param_3,param_4,2,1,0x4b),
      -1 < iVar1)) &&
     (iVar1 = (**(code **)(**(int **)(param_1 + 0x2a3e0) + 0x30))
                        (*(undefined4 *)(param_1 + 0x2a3e0),param_2,param_3,param_4,param_4,0x4b),
     -1 < iVar1)) {
    *param_5 = 0x4b;
    return 1;
  }
  if (((*(uint *)(param_1 + 0x2a5a8) < 0x19) && (*(uint *)(param_1 + 0x2a5ac) < 5)) &&
     ((iVar1 = (**(code **)(**(int **)(param_1 + 0x2a3e0) + 0x28))
                         (*(undefined4 *)(param_1 + 0x2a3e0),param_2,param_3,param_4,2,1,0x4f),
      -1 < iVar1 &&
      (iVar1 = (**(code **)(**(int **)(param_1 + 0x2a3e0) + 0x30))
                         (*(undefined4 *)(param_1 + 0x2a3e0),param_2,param_3,param_4,param_4,0x4f),
      -1 < iVar1)))) {
    *param_5 = 0x4f;
    return 1;
  }
  if (((*(uint *)(param_1 + 0x2a5a8) < 0x21) && (*(int *)(param_1 + 0x2a5ac) == 0)) &&
     ((iVar1 = (**(code **)(**(int **)(param_1 + 0x2a3e0) + 0x28))
                         (*(undefined4 *)(param_1 + 0x2a3e0),param_2,param_3,param_4,2,1,0x47),
      -1 < iVar1 &&
      (iVar1 = (**(code **)(**(int **)(param_1 + 0x2a3e0) + 0x30))
                         (*(undefined4 *)(param_1 + 0x2a3e0),param_2,param_3,param_4,param_4,0x47),
      -1 < iVar1)))) {
    *param_5 = 0x47;
    return 1;
  }
  return 0;
}


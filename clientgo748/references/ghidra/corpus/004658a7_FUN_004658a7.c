// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004658a7 | Name: FUN_004658a7


void __thiscall FUN_004658a7(int param_1,int param_2,undefined2 param_3,undefined2 param_4)

{
  int *piVar1;
  int *piVar2;
  int iVar3;
  int *piVar4;
  short local_10;
  
  piVar4 = *(int **)(param_1 + 0x27c78);
  piVar1 = (int *)(**(code **)(**(int **)(param_1 + 0x28) + 0x48))(0x276);
  piVar2 = (int *)(**(code **)(**(int **)(param_1 + 0x28) + 0x48))(0x273);
  if (((param_2 != 0) && (*(int *)(param_1 + 0x2873c) != 0)) &&
     ((iVar3 = (**(code **)(*DAT_0092e654 + 8))(), *(int *)(param_1 + 0x27478) == 0 ||
      (199 < (uint)(iVar3 - *(int *)(param_1 + 0x27478)))))) {
    (**(code **)(**(int **)(param_1 + 0x2873c) + 0xc0))
              (*(undefined4 *)(*(int *)(param_2 + 0x1e4) + 0x1e4),
               *(undefined4 *)(*(int *)(param_2 + 0x1e4) + 0x400));
    local_10 = (**(code **)(**(int **)(param_1 + 0x2873c) + 0xc4))
                         (*(undefined4 *)(*(int *)(param_2 + 0x1e4) + 0x1e4));
    if (local_10 == -1) {
      local_10 = (short)*(undefined4 *)(param_2 + 0x1d0) +
                 (short)*(undefined4 *)(param_2 + 0x1d4) * 9;
    }
    _memset((void *)(param_1 + 0x2719c),0,0x34);
    *(undefined2 *)(param_1 + 0x271a2) = *(undefined2 *)(DAT_013b71e8 + 0xc5c);
    *(undefined2 *)(param_1 + 0x271a0) = 0x3cc;
    *(undefined4 *)(param_1 + 0x271a8) = 1;
    *(int *)(param_1 + 0x271ac) = (int)local_10;
    *(undefined2 *)(param_1 + 0x271bc) = 0;
    *(undefined2 *)(param_1 + 0x271b8) = param_3;
    *(undefined2 *)(param_1 + 0x271ba) = param_4;
    if ((piVar1 != (int *)0x0) && (piVar2 != (int *)0x0)) {
      *(undefined4 *)(param_1 + 0x26ee0) = 6;
      (**(code **)(*piVar1 + 0x80))(&DAT_00a3eff8,0);
      (**(code **)(**(int **)(param_1 + 0x28) + 0x40))(piVar2);
      (**(code **)(*piVar4 + 0x60))(1);
      piVar4 = (int *)(**(code **)(**(int **)(param_1 + 0x28) + 0x48))(0x23e);
      if (piVar4 != (int *)0x0) {
        (**(code **)(*piVar4 + 0x60))(1);
      }
      if (DAT_005b892c != 2) {
        (**(code **)(*piVar2 + 0x70))(0x41c80000,0x42240000);
      }
      piVar2[0x414] = 0x10;
    }
  }
  return;
}


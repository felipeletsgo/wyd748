// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0056a9c4 | Name: FUN_0056a9c4


undefined4 __fastcall FUN_0056a9c4(byte *param_1)

{
  int *piVar1;
  
  piVar1 = *(int **)(param_1 + 8);
  if ((piVar1 != (int *)0x0) || (piVar1 = *(int **)(param_1 + 4), piVar1 != (int *)0x0)) {
    (**(code **)(*piVar1 + 0x28))(piVar1);
  }
  if ((((*(int *)(param_1 + 4) != 0) && (*(int *)(param_1 + 8) != 0)) &&
      (piVar1 = *(int **)(param_1 + 0xc), piVar1 != (int *)0x0)) && ((*param_1 & 1) == 0)) {
    (**(code **)(*piVar1 + 0x70))(piVar1,*(int *)(param_1 + 8),0,0,*(int *)(param_1 + 4),0);
  }
  piVar1 = *(int **)(param_1 + 8);
  if (piVar1 != (int *)0x0) {
    (**(code **)(*piVar1 + 8))(piVar1);
    param_1[8] = 0;
    param_1[9] = 0;
    param_1[10] = 0;
    param_1[0xb] = 0;
  }
  piVar1 = *(int **)(param_1 + 0xc);
  if (piVar1 != (int *)0x0) {
    (**(code **)(*piVar1 + 8))(piVar1);
    param_1[0xc] = 0;
    param_1[0xd] = 0;
    param_1[0xe] = 0;
    param_1[0xf] = 0;
  }
  param_1[4] = 0;
  param_1[5] = 0;
  param_1[6] = 0;
  param_1[7] = 0;
  return 0;
}


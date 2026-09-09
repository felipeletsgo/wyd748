// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004b37c9 | Name: FUN_004b37c9


void __thiscall FUN_004b37c9(int *param_1,int param_2)

{
  undefined4 uVar1;
  
  param_1[0x6c22] = DAT_0067cf38;
  DAT_0067cf38 = param_2;
  if (param_1[0x6c22] != 0) {
    *(undefined1 *)(param_1[0x6c22] + 0x14) = 1;
    uVar1 = (**(code **)(**(int **)(*(int *)(param_1[0x6c22] + 0x84) + 0x1e4) + 0x88))(0);
    (**(code **)(**(int **)(*(int *)(DAT_0067cf38 + 0x84) + 0x1e4) + 0x80))(uVar1);
    uVar1 = (**(code **)(**(int **)(*(int *)(param_1[0x6c22] + 0x84) + 0x1e8) + 0x88))(0);
    (**(code **)(**(int **)(*(int *)(DAT_0067cf38 + 0x84) + 0x1e8) + 0x80))(uVar1);
    *(undefined4 *)(*(int *)(DAT_0067cf38 + 0x84) + 0x28) =
         *(undefined4 *)(*(int *)(param_1[0x6c22] + 0x84) + 0x28);
    *(undefined4 *)(*(int *)(DAT_0067cf38 + 0x84) + 500) =
         *(undefined4 *)(*(int *)(param_1[0x6c22] + 0x84) + 500);
    *(int *)(*(int *)(DAT_0067cf38 + 0x84) + 0x1f8) =
         *(int *)(*(int *)(param_1[0x6c22] + 0x84) + 0x1f8) + 6000;
    (**(code **)(*param_1 + 100))(param_1[0x6c22]);
  }
  return;
}


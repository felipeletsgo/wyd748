// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004b7ca2 | Name: FUN_004b7ca2


void __fastcall FUN_004b7ca2(int param_1)

{
  undefined4 local_8;
  
  for (local_8 = 0; local_8 < 0x200; local_8 = local_8 + 1) {
    if (*(int *)(*(int *)(param_1 + 0xe8328) + local_8 * 4) != 0) {
      (**(code **)(**(int **)(*(int *)(param_1 + 0xe8328) + local_8 * 4) + 8))
                (*(undefined4 *)(*(int *)(param_1 + 0xe8328) + local_8 * 4));
      *(undefined4 *)(*(int *)(param_1 + 0xe8328) + local_8 * 4) = 0;
    }
  }
  for (local_8 = 0; local_8 < 0x200; local_8 = local_8 + 1) {
    if (*(int *)(*(int *)(param_1 + 0xe832c) + local_8 * 4) != 0) {
      (**(code **)(**(int **)(*(int *)(param_1 + 0xe832c) + local_8 * 4) + 8))
                (*(undefined4 *)(*(int *)(param_1 + 0xe832c) + local_8 * 4));
      *(undefined4 *)(*(int *)(param_1 + 0xe832c) + local_8 * 4) = 0;
    }
  }
  for (local_8 = 0; local_8 < 0x800; local_8 = local_8 + 1) {
    if (*(int *)(*(int *)(param_1 + 0xe8330) + local_8 * 4) != 0) {
      (**(code **)(**(int **)(*(int *)(param_1 + 0xe8330) + local_8 * 4) + 8))
                (*(undefined4 *)(*(int *)(param_1 + 0xe8330) + local_8 * 4));
      *(undefined4 *)(*(int *)(param_1 + 0xe8330) + local_8 * 4) = 0;
    }
  }
  for (local_8 = 0; local_8 < 0x200; local_8 = local_8 + 1) {
    if (*(int *)(*(int *)(param_1 + 0xe8334) + local_8 * 4) != 0) {
      (**(code **)(**(int **)(*(int *)(param_1 + 0xe8334) + local_8 * 4) + 8))
                (*(undefined4 *)(*(int *)(param_1 + 0xe8334) + local_8 * 4));
      *(undefined4 *)(*(int *)(param_1 + 0xe8334) + local_8 * 4) = 0;
    }
  }
  for (local_8 = 0; local_8 < 0x48; local_8 = local_8 + 1) {
    if (*(int *)(*(int *)(param_1 + 0xe8338) + local_8 * 4) != 0) {
      (**(code **)(**(int **)(*(int *)(param_1 + 0xe8338) + local_8 * 4) + 8))
                (*(undefined4 *)(*(int *)(param_1 + 0xe8338) + local_8 * 4));
      *(undefined4 *)(*(int *)(param_1 + 0xe8338) + local_8 * 4) = 0;
    }
  }
  for (local_8 = 0; local_8 < 0x48; local_8 = local_8 + 1) {
    if (*(int *)(*(int *)(param_1 + 0xe8338) + local_8 * 4) != 0) {
      (**(code **)(**(int **)(*(int *)(param_1 + 0xe8338) + local_8 * 4) + 8))
                (*(undefined4 *)(*(int *)(param_1 + 0xe8338) + local_8 * 4));
      *(undefined4 *)(*(int *)(param_1 + 0xe8338) + local_8 * 4) = 0;
    }
  }
  return;
}


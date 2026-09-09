// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004b7f0d | Name: FUN_004b7f0d


void __fastcall FUN_004b7f0d(int param_1)

{
  DWORD DVar1;
  int local_c;
  
  DVar1 = timeGetTime();
  if (*(int *)(param_1 + 0x324) + 900U <= DVar1) {
    *(DWORD *)(param_1 + 0x324) = DVar1;
    for (local_c = 0; local_c < 0x200; local_c = local_c + 1) {
      if (((*(int *)(*(int *)(param_1 + 0xe8328) + local_c * 4) != 0) &&
          (*(uint *)(param_1 + 0x142c + local_c * 0x108) <
           DVar1 - *(int *)(param_1 + 0x1428 + local_c * 0x108))) &&
         (*(int *)(*(int *)(param_1 + 0xe8328) + local_c * 4) != 0)) {
        (**(code **)(**(int **)(*(int *)(param_1 + 0xe8328) + local_c * 4) + 8))
                  (*(undefined4 *)(*(int *)(param_1 + 0xe8328) + local_c * 4));
        *(undefined4 *)(*(int *)(param_1 + 0xe8328) + local_c * 4) = 0;
      }
    }
    for (local_c = 0; local_c < 0x200; local_c = local_c + 1) {
      if (((*(int *)(*(int *)(param_1 + 0xe832c) + local_c * 4) != 0) &&
          (*(uint *)(param_1 + 0x2242c + local_c * 0x108) <
           DVar1 - *(int *)(param_1 + 0x22428 + local_c * 0x108))) &&
         (*(int *)(*(int *)(param_1 + 0xe832c) + local_c * 4) != 0)) {
        (**(code **)(**(int **)(*(int *)(param_1 + 0xe832c) + local_c * 4) + 8))
                  (*(undefined4 *)(*(int *)(param_1 + 0xe832c) + local_c * 4));
        *(undefined4 *)(*(int *)(param_1 + 0xe832c) + local_c * 4) = 0;
      }
    }
    for (local_c = 0; local_c < 0x200; local_c = local_c + 1) {
      if (((*(int *)(*(int *)(param_1 + 0xe8334) + local_c * 4) != 0) &&
          (*(uint *)(param_1 + 0xc742c + local_c * 0x108) <
           DVar1 - *(int *)(param_1 + 0xc7428 + local_c * 0x108))) &&
         (*(int *)(*(int *)(param_1 + 0xe8334) + local_c * 4) != 0)) {
        (**(code **)(**(int **)(*(int *)(param_1 + 0xe8334) + local_c * 4) + 8))
                  (*(undefined4 *)(*(int *)(param_1 + 0xe8334) + local_c * 4));
        *(undefined4 *)(*(int *)(param_1 + 0xe8334) + local_c * 4) = 0;
      }
    }
    for (local_c = 0; local_c < 0x800; local_c = local_c + 1) {
      if (((*(int *)(*(int *)(param_1 + 0xe8330) + local_c * 4) != 0) &&
          (*(uint *)(param_1 + 0x4342c + local_c * 0x108) <
           DVar1 - *(int *)(param_1 + 0x43428 + local_c * 0x108))) &&
         (*(int *)(*(int *)(param_1 + 0xe8330) + local_c * 4) != 0)) {
        (**(code **)(**(int **)(*(int *)(param_1 + 0xe8330) + local_c * 4) + 8))
                  (*(undefined4 *)(*(int *)(param_1 + 0xe8330) + local_c * 4));
        *(undefined4 *)(*(int *)(param_1 + 0xe8330) + local_c * 4) = 0;
      }
    }
    for (local_c = 0; local_c < 0x48; local_c = local_c + 1) {
      if (((*(int *)(*(int *)(param_1 + 0xe8338) + local_c * 4) != 0) &&
          (5000 < DVar1 - *(int *)(param_1 + 0xe833c + local_c * 4))) &&
         (*(int *)(*(int *)(param_1 + 0xe8338) + local_c * 4) != 0)) {
        (**(code **)(**(int **)(*(int *)(param_1 + 0xe8338) + local_c * 4) + 8))
                  (*(undefined4 *)(*(int *)(param_1 + 0xe8338) + local_c * 4));
        *(undefined4 *)(*(int *)(param_1 + 0xe8338) + local_c * 4) = 0;
      }
    }
  }
  return;
}


// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004bab41 | Name: FUN_004bab41


void __fastcall FUN_004bab41(int param_1)

{
  undefined4 local_8;
  
  for (local_8 = 0; local_8 < 0x40; local_8 = local_8 + 1) {
    if (*(int *)(param_1 + 4 + local_8 * 0xc) != 0) {
      (**(code **)(**(int **)(param_1 + 4 + local_8 * 0xc) + 8))
                (*(undefined4 *)(param_1 + 4 + local_8 * 0xc));
      *(undefined4 *)(param_1 + 4 + local_8 * 0xc) = 0;
    }
    *(undefined4 *)(param_1 + 4 + local_8 * 0xc) = 0;
    *(undefined4 *)(param_1 + 8 + local_8 * 0xc) = 0xffffffff;
    *(undefined4 *)(param_1 + 0xc + local_8 * 0xc) = 0;
  }
  return;
}


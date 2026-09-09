// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004ac81e | Name: FUN_004ac81e


void __thiscall FUN_004ac81e(int param_1,int param_2)

{
  undefined4 local_8;
  
  if (param_2 < 0) {
    for (local_8 = 0; local_8 < 0x32; local_8 = local_8 + 1) {
      if (*(int *)(param_1 + 0x26f0c + local_8 * 4) != 0) {
        FUN_00520216(*(undefined4 *)(param_1 + 0x27b8c + local_8 * 8),
                     *(undefined4 *)(param_1 + 0x27b90 + local_8 * 8),0x20,0);
      }
    }
  }
  if (*(int *)(param_1 + 0x26f0c + param_2 * 4) != 0) {
    FUN_00520216(*(undefined4 *)(param_1 + 0x27b8c + param_2 * 8),
                 *(undefined4 *)(param_1 + 0x27b90 + param_2 * 8),0x20,0);
  }
  return;
}


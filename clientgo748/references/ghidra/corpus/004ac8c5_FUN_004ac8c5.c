// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004ac8c5 | Name: FUN_004ac8c5


void __fastcall FUN_004ac8c5(int param_1)

{
  undefined4 local_8;
  
  for (local_8 = 0; local_8 < 0x12; local_8 = local_8 + 1) {
    if (*(int *)(param_1 + 0x26f0c + local_8 * 4) != 0) {
      (**(code **)(*DAT_013b71e8 + 100))(*(undefined4 *)(param_1 + 0x26f0c + local_8 * 4));
      *(undefined4 *)(param_1 + 0x26f0c + local_8 * 4) = 0;
    }
  }
  for (local_8 = 0x1a; local_8 < 0x28; local_8 = local_8 + 1) {
    if (*(int *)(param_1 + 0x26f0c + local_8 * 4) != 0) {
      (**(code **)(*DAT_013b71e8 + 100))(*(undefined4 *)(param_1 + 0x26f0c + local_8 * 4));
      *(undefined4 *)(param_1 + 0x26f0c + local_8 * 4) = 0;
    }
  }
  return;
}


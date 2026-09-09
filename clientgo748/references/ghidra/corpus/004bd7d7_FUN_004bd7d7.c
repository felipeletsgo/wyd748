// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004bd7d7 | Name: FUN_004bd7d7


void __fastcall FUN_004bd7d7(int param_1)

{
  int local_8;
  
  if ((*(int *)(param_1 + 0xb4) != *(int *)(param_1 + 0xb8)) && (*(int *)(param_1 + 0xb4) != 0)) {
    if (*(int *)(param_1 + 0xb4) != 0) {
      FUN_004c5ae0(1);
    }
    *(undefined4 *)(param_1 + 0xb4) = 0;
  }
  for (local_8 = 0; local_8 < 0x50; local_8 = local_8 + 1) {
    if (*(int *)(param_1 + 0xb8 + local_8 * 4) != 0) {
      if (*(int *)(param_1 + 0xb8 + local_8 * 4) != 0) {
        FUN_004c5ae0(1);
      }
      *(undefined4 *)(param_1 + 0xb8 + local_8 * 4) = 0;
    }
  }
  for (local_8 = 0; local_8 < 2; local_8 = local_8 + 1) {
    if (*(int *)(param_1 + 0x2f8 + local_8 * 4) != 0) {
      (**(code **)(*DAT_013b71e8 + 100))(*(undefined4 *)(param_1 + 0x2f8 + local_8 * 4));
      *(undefined4 *)(param_1 + 0x2f8 + local_8 * 4) = 0;
    }
  }
  return;
}


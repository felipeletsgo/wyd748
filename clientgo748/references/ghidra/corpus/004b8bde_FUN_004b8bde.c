// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004b8bde | Name: FUN_004b8bde


void __fastcall FUN_004b8bde(int param_1)

{
  undefined4 local_8;
  
  for (local_8 = 0; local_8 < 0x200; local_8 = local_8 + 1) {
    if (*(int *)(param_1 + 0x32c + local_8 * 8) != 0) {
      FUN_0058ea3a(*(undefined4 *)(param_1 + 0x32c + local_8 * 8));
      *(undefined4 *)(param_1 + 0x32c + local_8 * 8) = 0;
    }
  }
  return;
}


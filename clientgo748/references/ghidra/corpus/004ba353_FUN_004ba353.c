// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004ba353 | Name: FUN_004ba353


undefined4 __fastcall FUN_004ba353(int param_1)

{
  undefined4 local_8;
  
  for (local_8 = 0; local_8 < 0x48; local_8 = local_8 + 1) {
    *(undefined4 *)(*(int *)(param_1 + 0xe8338) + local_8 * 4) = 0;
  }
  return 1;
}


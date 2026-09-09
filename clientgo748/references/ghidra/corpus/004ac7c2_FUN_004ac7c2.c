// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004ac7c2 | Name: FUN_004ac7c2


void __fastcall FUN_004ac7c2(int param_1)

{
  undefined4 local_8;
  
  for (local_8 = 0; local_8 < 0x32; local_8 = local_8 + 1) {
    if (*(int *)(param_1 + 0x26f0c + local_8 * 4) != 0) {
      FUN_0051dba4(*(undefined4 *)(param_1 + 0x2700c + local_8 * 0x3c),0);
    }
  }
  return;
}


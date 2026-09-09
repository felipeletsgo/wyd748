// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004baae7 | Name: FUN_004baae7


void __fastcall FUN_004baae7(int param_1)

{
  undefined4 local_8;
  
  for (local_8 = 0; local_8 < 0x40; local_8 = local_8 + 1) {
    *(undefined4 *)(param_1 + 4 + local_8 * 0xc) = 0;
    *(undefined4 *)(param_1 + 8 + local_8 * 0xc) = 0xffffffff;
    *(undefined4 *)(param_1 + 0xc + local_8 * 0xc) = 0;
  }
  return;
}


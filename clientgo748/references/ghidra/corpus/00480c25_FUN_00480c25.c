// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00480c25 | Name: FUN_00480c25


void __fastcall FUN_00480c25(int param_1)

{
  undefined4 uVar1;
  
  uVar1 = FUN_0054e06c(DAT_013b71e8 + 0x768);
  *(undefined4 *)(param_1 + 0x26e78) = uVar1;
  return;
}


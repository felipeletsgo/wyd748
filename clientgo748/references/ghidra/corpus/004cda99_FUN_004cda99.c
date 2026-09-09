// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004cda99 | Name: FUN_004cda99


void __fastcall FUN_004cda99(int param_1)

{
  undefined4 uVar1;
  
  uVar1 = __ftol();
  *(undefined4 *)(param_1 + 0x138) = uVar1;
  uVar1 = __ftol();
  *(undefined4 *)(param_1 + 0x13c) = uVar1;
  return;
}


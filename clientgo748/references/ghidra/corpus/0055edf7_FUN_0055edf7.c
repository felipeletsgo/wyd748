// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0055edf7 | Name: FUN_0055edf7


void __fastcall FUN_0055edf7(int param_1)

{
  HBITMAP pHVar1;
  
  pHVar1 = LoadBitmapA(DAT_013b7224,(LPCSTR)0xab);
  *(HBITMAP *)(param_1 + 0x110) = pHVar1;
  return;
}


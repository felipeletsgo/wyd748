// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004aeb4c | Name: FUN_004aeb4c


BOOL __fastcall FUN_004aeb4c(int param_1)

{
  HIMC pHVar1;
  BOOL BVar2;
  
  pHVar1 = ImmGetContext(*(HWND *)(param_1 + 0x444));
  BVar2 = ImmGetOpenStatus(pHVar1);
  ImmReleaseContext(*(HWND *)(param_1 + 0x444),pHVar1);
  return BVar2;
}


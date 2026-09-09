// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004aeb8d | Name: FUN_004aeb8d


void __thiscall FUN_004aeb8d(int param_1,BOOL param_2)

{
  HIMC pHVar1;
  
  pHVar1 = ImmGetContext(*(HWND *)(param_1 + 0x444));
  ImmSetOpenStatus(pHVar1,param_2);
  ImmReleaseContext(*(HWND *)(param_1 + 0x444),pHVar1);
  return;
}


// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004ae9c2 | Name: FUN_004ae9c2


void __fastcall FUN_004ae9c2(int param_1)

{
  LONG LVar1;
  HIMC pHVar2;
  
  GlobalFree(*(HGLOBAL *)(param_1 + 0xa58));
  *(undefined4 *)(param_1 + 0xa58) = 0;
  LVar1 = GetWindowLongA(*(HWND *)(param_1 + 0x444),0);
  *(LONG *)(param_1 + 0x450) = LVar1;
  pHVar2 = ImmAssociateContext(*(HWND *)(param_1 + 0x444),*(HIMC *)(param_1 + 0x450));
  ImmDestroyContext(pHVar2);
  return;
}


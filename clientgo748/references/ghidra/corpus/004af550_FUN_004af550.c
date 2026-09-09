// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004af550 | Name: FUN_004af550


void __thiscall FUN_004af550(int param_1,undefined4 param_2,uint param_3)

{
  HIMC pHVar1;
  LONG LVar2;
  
  pHVar1 = ImmGetContext(*(HWND *)(param_1 + 0x444));
  if (pHVar1 != (HIMC)0x0) {
    if ((param_3 & 8) == 0) {
      *(undefined1 *)(param_1 + 0x454) = 0;
    }
    else {
      LVar2 = ImmGetCompositionStringA(pHVar1,8,(LPVOID)(param_1 + 0x454),0x200);
      *(undefined1 *)(param_1 + LVar2 + 0x454) = 0;
    }
    ImmReleaseContext(*(HWND *)(param_1 + 0x444),pHVar1);
    (**(code **)(*DAT_013b71e8 + 0x20))(param_1 + 0x454);
  }
  return;
}


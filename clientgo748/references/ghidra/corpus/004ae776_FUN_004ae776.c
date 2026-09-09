// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004ae776 | Name: FUN_004ae776


undefined4 __fastcall FUN_004ae776(int param_1)

{
  HGLOBAL pvVar1;
  HIMC pHVar2;
  HWND hWnd;
  CHAR local_88 [128];
  HIMC local_8;
  
  if (*(int *)(param_1 + 0xa58) == 0) {
    pvVar1 = GlobalAlloc(0x40,0x1c);
    *(HGLOBAL *)(param_1 + 0xa58) = pvVar1;
  }
  local_8 = ImmCreateContext();
  pHVar2 = ImmAssociateContext(*(HWND *)(param_1 + 0x444),local_8);
  *(HIMC *)(param_1 + 0x450) = pHVar2;
  SetWindowLongA(*(HWND *)(param_1 + 0x444),0,*(LONG *)(param_1 + 0x450));
  ImmReleaseContext(*(HWND *)(param_1 + 0x444),local_8);
  SendMessageA(*(HWND *)(DAT_013b71e0 + 4),0x281,0,-0x7ffffff2);
  hWnd = ImmGetDefaultIMEWnd(*(HWND *)(param_1 + 0x444));
  GetWindowTextA(hWnd,local_88,0x80);
  return 1;
}


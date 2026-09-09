// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0042896f | Name: FUN_0042896f


void __thiscall FUN_0042896f(int param_1,LPARAM param_2)

{
  undefined4 uVar1;
  
  if (*(int *)(param_1 + 0x14) == 0) {
    uVar1 = MCIWndCreateA(*(undefined4 *)(DAT_013b71e0 + 4),DAT_013b7224,8,param_2);
    *(undefined4 *)(param_1 + 0x14) = uVar1;
    ShowWindow(*(HWND *)(param_1 + 0x14),0);
  }
  else {
    SendMessageA(*(HWND *)(param_1 + 0x14),0x499,0,param_2);
  }
  if (*(int *)(param_1 + 0x14) != 0) {
    SendMessageA(*(HWND *)(param_1 + 0x14),0x806,0,0);
  }
  return;
}


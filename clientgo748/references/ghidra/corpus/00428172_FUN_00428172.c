// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00428172 | Name: FUN_00428172


undefined4 __fastcall FUN_00428172(int param_1)

{
  if (*(int *)(param_1 + 0x2a384) == 0) {
    SetWindowLongA(*(HWND *)(param_1 + 0x2a3d8),-0x10,-0x6ff80000);
  }
  else {
    SetWindowLongA(*(HWND *)(param_1 + 0x2a3d8),-0x10,*(LONG *)(param_1 + 0x2a4e0));
  }
  return 0;
}


// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0059dfe0 | Name: FUN_0059dfe0


void FUN_0059dfe0(HWND param_1)

{
  int *piVar1;
  undefined4 *hMem;
  
  hMem = (undefined4 *)GetWindowLongA(param_1,-0x15);
  if (hMem != (undefined4 *)0x0) {
    piVar1 = (int *)*hMem;
    (**(code **)(*piVar1 + 0x18))(piVar1,1);
    (**(code **)(*piVar1 + 8))(piVar1);
    GlobalFree(hMem);
  }
  return;
}


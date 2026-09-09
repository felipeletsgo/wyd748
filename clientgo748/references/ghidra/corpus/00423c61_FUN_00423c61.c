// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00423c61 | Name: FUN_00423c61


void FUN_00423c61(void)

{
  char cVar1;
  
  if (DAT_005ccf84 != '\0') {
    cVar1 = FUN_00423b25();
    if (cVar1 == '\0') {
      UnhookWindowsHookEx(DAT_005ccf80);
    }
    DAT_005ccf84 = '\0';
  }
  return;
}


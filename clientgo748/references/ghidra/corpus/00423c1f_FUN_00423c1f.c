// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00423c1f | Name: FUN_00423c1f


void FUN_00423c1f(HINSTANCE param_1)

{
  char cVar1;
  
  if (DAT_005ccf84 != '\x01') {
    cVar1 = FUN_00423b25();
    if (cVar1 == '\0') {
      DAT_005ccf80 = SetWindowsHookExA(0xd,FUN_00423b74,param_1,0);
    }
    DAT_005ccf84 = '\x01';
  }
  return;
}


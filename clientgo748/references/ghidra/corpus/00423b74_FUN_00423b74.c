// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00423b74 | Name: FUN_00423b74


LRESULT FUN_00423b74(int param_1,WPARAM param_2,int *param_3)

{
  SHORT SVar1;
  LRESULT LVar2;
  
  if (-1 < param_1) {
    SVar1 = GetAsyncKeyState(0x11);
    if ((*param_3 == 0x1b) && (SVar1 < 0)) {
      return 1;
    }
    if ((*param_3 == 0x1b) && ((param_3[2] & 0x20U) != 0)) {
      return 1;
    }
    if (*param_3 == 0x5b) {
      return 1;
    }
    if (*param_3 == 0x5c) {
      return 1;
    }
    if (*param_3 == 0x5d) {
      return 1;
    }
  }
  LVar2 = CallNextHookEx(DAT_005ccf80,param_1,param_2,(LPARAM)param_3);
  return LVar2;
}


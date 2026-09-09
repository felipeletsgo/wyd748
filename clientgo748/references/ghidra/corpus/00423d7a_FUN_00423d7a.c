// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00423d7a | Name: FUN_00423d7a


undefined4 __fastcall FUN_00423d7a(SOCKET *param_1)

{
  param_1[3] = 0;
  param_1[6] = 0;
  param_1[4] = 0;
  param_1[5] = 0;
  param_1[7] = 0;
  if (*param_1 != 0) {
    closesocket(*param_1);
  }
  *param_1 = 0;
  return 1;
}


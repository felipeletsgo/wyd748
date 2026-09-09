// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00591e87 | Name: FUN_00591e87


char * FUN_00591e87(char *param_1)

{
  _SYSTEMTIME local_18;
  
  GetLocalTime(&local_18);
  param_1[8] = '\0';
  param_1[5] = '/';
  param_1[2] = '/';
  *param_1 = (char)((ulonglong)(uint)local_18.wMonth / 10) + '0';
  param_1[1] = (char)((ulonglong)(uint)local_18.wMonth % 10) + '0';
  param_1[3] = (char)((ulonglong)local_18.wDay / 10) + '0';
  param_1[4] = (char)((ulonglong)local_18.wDay % 10) + '0';
  param_1[6] = (char)(((uint)local_18.wYear % 100) / 10) + '0';
  param_1[7] = (char)(((uint)local_18.wYear % 100) % 10) + '0';
  return param_1;
}


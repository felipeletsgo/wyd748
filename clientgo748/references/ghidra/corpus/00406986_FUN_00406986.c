// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00406986 | Name: FUN_00406986


void __thiscall FUN_00406986(int *param_1,LPCSTR param_2)

{
  int iVar1;
  size_t local_10;
  
  if (*param_2 == '\0') {
    _memset(param_1 + 0x393,0,0xff);
    (**(code **)(*param_1 + 0x54))();
  }
  else {
    iVar1 = lstrlenA(param_2);
    if (param_1[0x414] < iVar1) {
      local_10 = param_1[0x414];
    }
    else {
      local_10 = lstrlenA(param_2);
    }
    _strncpy((char *)(param_1 + 0x393),param_2,local_10);
    *(undefined1 *)((int)param_1 + local_10 + 0xe4c) = 0;
    (**(code **)(*param_1 + 0x54))();
  }
  return;
}


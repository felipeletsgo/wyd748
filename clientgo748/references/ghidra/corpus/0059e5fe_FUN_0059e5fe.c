// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0059e5fe | Name: FUN_0059e5fe


char * FUN_0059e5fe(char *param_1)

{
  char cVar1;
  size_t _Size;
  int iVar2;
  char *pcVar3;
  bool bVar4;
  void *local_c;
  
  local_c = (void *)0x0;
  if (DAT_013b82e4 == 0) {
    cVar1 = *param_1;
    pcVar3 = param_1;
    while (cVar1 != '\0') {
      cVar1 = *pcVar3;
      if (('`' < cVar1) && (cVar1 < '{')) {
        *pcVar3 = cVar1 + -0x20;
      }
      pcVar3 = pcVar3 + 1;
      cVar1 = *pcVar3;
    }
  }
  else {
    InterlockedIncrement((LONG *)&DAT_013b8728);
    bVar4 = DAT_013b8724 == 0;
    if (!bVar4) {
      InterlockedDecrement((LONG *)&DAT_013b8728);
      FUN_00596aad(0x13);
    }
    if (DAT_013b82e4 == 0) {
      if (bVar4) {
        InterlockedDecrement((LONG *)&DAT_013b8728);
      }
      else {
        FUN_00596b0e(0x13);
      }
      cVar1 = *param_1;
      pcVar3 = param_1;
      while (cVar1 != '\0') {
        cVar1 = *pcVar3;
        if (('`' < cVar1) && (cVar1 < '{')) {
          *pcVar3 = cVar1 + -0x20;
        }
        pcVar3 = pcVar3 + 1;
        cVar1 = *pcVar3;
      }
    }
    else {
      _Size = FUN_0059b2e2(DAT_013b82e4,0x200,param_1,0xffffffff,0,0,0,1);
      if (((_Size != 0) && (local_c = _malloc(_Size), local_c != (void *)0x0)) &&
         (iVar2 = FUN_0059b2e2(DAT_013b82e4,0x200,param_1,0xffffffff,local_c,_Size,0,1), iVar2 != 0)
         ) {
        FUN_0058ee20(param_1,local_c);
      }
      if (bVar4) {
        InterlockedDecrement((LONG *)&DAT_013b8728);
      }
      else {
        FUN_00596b0e(0x13);
      }
      FUN_0058ffd8(local_c);
    }
  }
  return param_1;
}


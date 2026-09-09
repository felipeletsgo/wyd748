// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00599c9c | Name: FUN_00599c9c


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_00599c9c(void)

{
  void *pvVar1;
  char *pcVar2;
  int local_c;
  int local_8;
  
  if (DAT_013b8734 == 0) {
    FUN_00599b6f();
  }
  GetModuleFileNameA((HMODULE)0x0,&DAT_013b81a0,0x104);
  _DAT_013b8014 = &DAT_013b81a0;
  pcVar2 = &DAT_013b81a0;
  if (*DAT_013b872c != '\0') {
    pcVar2 = DAT_013b872c;
  }
  FUN_00599d35(pcVar2,0,0,&local_8,&local_c);
  pvVar1 = _malloc(local_c + local_8 * 4);
  if (pvVar1 == (void *)0x0) {
    __amsg_exit(8);
  }
  FUN_00599d35(pcVar2,pvVar1,(void *)((int)pvVar1 + local_8 * 4),&local_8,&local_c);
  _DAT_013b7ffc = pvVar1;
  _DAT_013b7ff8 = local_8 + -1;
  return;
}


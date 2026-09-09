// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00599be3 | Name: FUN_00599be3


void FUN_00599be3(void)

{
  char cVar1;
  size_t sVar2;
  undefined4 *puVar3;
  void *pvVar4;
  int iVar5;
  char *pcVar6;
  
  if (DAT_013b8734 == 0) {
    FUN_00599b6f();
  }
  iVar5 = 0;
  for (pcVar6 = DAT_013b8028; *pcVar6 != '\0'; pcVar6 = pcVar6 + sVar2 + 1) {
    if (*pcVar6 != '=') {
      iVar5 = iVar5 + 1;
    }
    sVar2 = _strlen(pcVar6);
  }
  puVar3 = _malloc(iVar5 * 4 + 4);
  DAT_013b8004 = puVar3;
  if (puVar3 == (undefined4 *)0x0) {
    __amsg_exit(9);
  }
  cVar1 = *DAT_013b8028;
  pcVar6 = DAT_013b8028;
  while (cVar1 != '\0') {
    sVar2 = _strlen(pcVar6);
    if (*pcVar6 != '=') {
      pvVar4 = _malloc(sVar2 + 1);
      *puVar3 = pvVar4;
      if (pvVar4 == (void *)0x0) {
        __amsg_exit(9);
      }
      FUN_0058ee20(*puVar3,pcVar6);
      puVar3 = puVar3 + 1;
    }
    pcVar6 = pcVar6 + sVar2 + 1;
    cVar1 = *pcVar6;
  }
  FUN_0058ffd8(DAT_013b8028);
  DAT_013b8028 = (char *)0x0;
  *puVar3 = 0;
  DAT_013b8730 = 1;
  return;
}


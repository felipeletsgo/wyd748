// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004b1b0b | Name: FUN_004b1b0b


void __thiscall FUN_004b1b0b(undefined4 param_1,char *param_2)

{
  char cVar1;
  char *pcVar2;
  int iVar3;
  char *pcVar4;
  
  FUN_004b194c(param_2,0x2b,0x20,param_1);
  pcVar2 = param_2;
  pcVar4 = param_2;
  while (param_2 = pcVar2, *param_2 != '\0') {
    if (*param_2 == '%') {
      pcVar2 = param_2 + 1;
      iVar3 = FUN_00591280((int)*pcVar2);
      if ((iVar3 != 0) && (iVar3 = FUN_00591280((int)param_2[2]), iVar3 != 0)) {
        cVar1 = FUN_004b1bbe(pcVar2);
        *pcVar4 = cVar1;
        pcVar4 = pcVar4 + 1;
        pcVar2 = param_2 + 3;
      }
    }
    else {
      *pcVar4 = *param_2;
      pcVar4 = pcVar4 + 1;
      pcVar2 = param_2 + 1;
    }
  }
  *pcVar4 = '\0';
  return;
}


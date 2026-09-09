// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 005905ef | Name: FUN_005905ef


uint FUN_005905ef(byte param_1,char *param_2)

{
  char cVar1;
  int iVar2;
  int iVar3;
  char *pcVar4;
  uint uVar5;
  
  pcVar4 = param_2;
  if (param_2[1] == ':') {
    pcVar4 = param_2 + 2;
  }
  cVar1 = *pcVar4;
  if ((((cVar1 == '\\') || (cVar1 == '/')) && (pcVar4[1] == '\0')) ||
     (((param_1 & 0x10) != 0 || (uVar5 = 0x8000, cVar1 == '\0')))) {
    uVar5 = 0x4040;
  }
  uVar5 = uVar5 | (uint)(~param_1 & 1 | 2) << 7;
  iVar2 = FUN_00596d99(param_2,0x2e);
  if (iVar2 != 0) {
    iVar3 = FUN_00596c04(iVar2,&DAT_005aa004);
    if (iVar3 != 0) {
      iVar3 = FUN_00596c04(iVar2,&DAT_005a9ffc);
      if (iVar3 != 0) {
        iVar3 = FUN_00596c04(iVar2,&DAT_005a9ff4);
        if (iVar3 != 0) {
          iVar2 = FUN_00596c04(iVar2,&DAT_005a9fec);
          if (iVar2 != 0) goto LAB_00590690;
        }
      }
    }
    uVar5 = uVar5 | 0x40;
  }
LAB_00590690:
  return (uVar5 & 0x1c0) >> 6 | uVar5 | uVar5 >> 3 & 0x38;
}


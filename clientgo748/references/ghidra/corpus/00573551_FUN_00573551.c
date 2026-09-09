// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00573551 | Name: FUN_00573551


void FUN_00573551(int param_1,int param_2)

{
  int *piVar1;
  int iVar2;
  undefined4 uVar3;
  uint uVar4;
  int iVar5;
  char *pcVar6;
  
  iVar2 = param_1;
  if (*(byte *)(param_1 + 0x11c) < 8) {
    uVar4 = (uint)*(byte *)(param_1 + 0x11c);
    iVar5 = -uVar4 + 8;
    FUN_00575739(param_1,uVar4 + 0x20 + param_2,iVar5);
    *(undefined1 *)(iVar2 + 0x11c) = 8;
    iVar5 = FUN_005757a0(param_2 + 0x20,uVar4,iVar5);
    if (iVar5 != 0) {
      if ((uVar4 < 4) && (iVar5 = FUN_005757a0(param_2 + 0x20,uVar4,-uVar4 + 4), iVar5 != 0)) {
        pcVar6 = "Not a PNG file";
      }
      else {
        pcVar6 = "PNG file corrupted by ASCII conversion";
      }
      FUN_005733d1(iVar2,pcVar6);
    }
  }
  piVar1 = (int *)(iVar2 + 0x10c);
  while( true ) {
    while( true ) {
      while( true ) {
        while( true ) {
          FUN_00575739(iVar2,&param_1,4);
          uVar3 = FUN_00579a92(&param_1);
          FUN_00575840(iVar2);
          FUN_00579ab5(iVar2,piVar1,4);
          if (*piVar1 != DAT_005a824c) break;
          FUN_0057a411(iVar2,param_2,uVar3);
        }
        if (*piVar1 != DAT_005a8264) break;
        FUN_0057a607(iVar2,param_2,uVar3);
      }
      if (*piVar1 != DAT_005a825c) break;
      FUN_0057a731(iVar2,param_2,uVar3);
    }
    if (*piVar1 == DAT_005a8254) break;
    if (*piVar1 == DAT_005a826c) {
      FUN_0057a77b();
    }
    else if (*piVar1 == DAT_005a8274) {
      FUN_0057a886();
    }
    else if (*piVar1 == DAT_005a827c) {
      FUN_0057a975();
    }
    else {
      FUN_0057ab2c(iVar2,param_2,uVar3);
    }
  }
  if ((*(uint *)(iVar2 + 0x58) & 1) == 0) {
    pcVar6 = "Missing IHDR before IDAT";
  }
  else {
    if ((*(char *)(iVar2 + 0x116) != '\x03') || ((*(uint *)(iVar2 + 0x58) & 2) != 0))
    goto LAB_005736b5;
    pcVar6 = "Missing PLTE before IDAT";
  }
  FUN_005733d1(iVar2,pcVar6);
LAB_005736b5:
  *(uint *)(iVar2 + 0x58) = *(uint *)(iVar2 + 0x58) | 4;
  *(undefined4 *)(iVar2 + 0xfc) = uVar3;
  return;
}


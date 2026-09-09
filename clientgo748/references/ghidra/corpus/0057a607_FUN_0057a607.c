// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0057a607 | Name: FUN_0057a607


void FUN_0057a607(int param_1,int param_2,uint param_3)

{
  uint uVar1;
  int iVar2;
  int iVar3;
  int iVar4;
  int iVar5;
  undefined1 *puVar6;
  char *pcVar7;
  int local_8;
  
  iVar2 = param_1;
  uVar1 = *(uint *)(param_1 + 0x58);
  if ((uVar1 & 1) == 0) {
    pcVar7 = "Missing IHDR before PLTE";
LAB_0057a633:
    FUN_005733d1(param_1,pcVar7);
LAB_0057a63b:
    *(uint *)(iVar2 + 0x58) = *(uint *)(iVar2 + 0x58) | 2;
    if (param_3 % 3 != 0) {
      pcVar7 = "Invalid palette chunk";
      if (*(char *)(iVar2 + 0x116) != '\x03') goto LAB_0057a65b;
      FUN_005733d1(iVar2,"Invalid palette chunk");
    }
    iVar4 = (int)param_3 / 3;
    iVar5 = FUN_005757e8(iVar2,iVar4,3);
    *(byte *)(iVar2 + 0x5d) = *(byte *)(iVar2 + 0x5d) | 0x10;
    if (0 < iVar4) {
      puVar6 = (undefined1 *)(iVar5 + 2);
      local_8 = iVar4;
      do {
        FUN_00579ab5(iVar2,&param_1,3);
        puVar6[-2] = (undefined1)param_1;
        puVar6[-1] = param_1._1_1_;
        *puVar6 = param_1._2_1_;
        puVar6 = puVar6 + 3;
        local_8 = local_8 + -1;
      } while (local_8 != 0);
    }
    FUN_0057a385(iVar2,0);
    iVar3 = param_2;
    *(int *)(iVar2 + 0x104) = iVar5;
    *(short *)(iVar2 + 0x108) = (short)iVar4;
    FUN_0057567a(iVar2,param_2,iVar5,iVar4);
    if ((((*(char *)(iVar2 + 0x116) == '\x03') && (iVar3 != 0)) &&
        ((*(byte *)(iVar3 + 8) & 0x10) != 0)) &&
       (*(ushort *)(iVar2 + 0x108) < *(ushort *)(iVar2 + 0x10a))) {
      FUN_005733ef(iVar2,"Truncating incorrect tRNS chunk length");
      *(ushort *)(iVar2 + 0x10a) = *(ushort *)(iVar2 + 0x108);
    }
  }
  else {
    if ((uVar1 & 4) == 0) {
      if ((uVar1 & 2) != 0) {
        pcVar7 = "Duplicate PLTE chunk";
        goto LAB_0057a633;
      }
      goto LAB_0057a63b;
    }
    pcVar7 = "Invalid PLTE after IDAT";
LAB_0057a65b:
    FUN_005733ef(iVar2,pcVar7);
    FUN_0057a385(iVar2,param_3);
  }
  return;
}


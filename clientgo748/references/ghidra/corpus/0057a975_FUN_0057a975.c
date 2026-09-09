// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0057a975 | Name: FUN_0057a975


void FUN_0057a975(int param_1,int param_2,uint param_3)

{
  char cVar1;
  undefined4 uVar2;
  int iVar3;
  char *pcVar4;
  undefined4 local_c;
  ushort local_8;
  
  if ((*(uint *)(param_1 + 0x58) & 1) == 0) {
    FUN_005733d1(param_1,"Missing IHDR before tRNS");
  }
  else {
    if ((*(uint *)(param_1 + 0x58) & 4) != 0) {
      pcVar4 = "Invalid tRNS after IDAT";
LAB_0057a9ea:
      FUN_005733ef(param_1,pcVar4);
      FUN_0057a385(param_1,param_3);
      return;
    }
    if ((param_2 != 0) && ((*(byte *)(param_2 + 8) & 0x10) != 0)) {
      pcVar4 = "Duplicate tRNS chunk";
      goto LAB_0057a9ea;
    }
  }
  cVar1 = *(char *)(param_1 + 0x116);
  if (cVar1 == '\x03') {
    if ((*(byte *)(param_1 + 0x58) & 2) == 0) {
      FUN_005733ef(param_1,"Missing PLTE before tRNS");
LAB_0057a9bb:
      if (param_3 != 0) {
        uVar2 = FUN_005799ef(param_1,param_3);
        *(byte *)(param_1 + 0x5d) = *(byte *)(param_1 + 0x5d) | 0x20;
        *(undefined4 *)(param_1 + 0x15c) = uVar2;
        FUN_00579ab5(param_1,uVar2,param_3);
        *(short *)(param_1 + 0x10a) = (short)param_3;
        goto LAB_0057aae1;
      }
      FUN_005733ef(param_1,"Zero length tRNS chunk");
    }
    else {
      if (param_3 <= *(ushort *)(param_1 + 0x108)) goto LAB_0057a9bb;
      FUN_005733ef(param_1,"Incorrect tRNS chunk length");
    }
LAB_0057ab1e:
    FUN_0057a385(param_1,param_3);
  }
  else {
    if (cVar1 == '\x02') {
      if (param_3 != 6) {
LAB_0057aaac:
        pcVar4 = "Incorrect tRNS chunk length";
LAB_0057ab15:
        FUN_005733ef(param_1,pcVar4);
        goto LAB_0057ab1e;
      }
      FUN_00579ab5(param_1,&local_c,6);
      *(short *)(param_1 + 0x162) = (short)((local_c & 0xff) * 0x100 + (local_c >> 8 & 0xff));
      *(ushort *)(param_1 + 0x164) = (ushort)local_c._2_1_ * 0x100 + (ushort)local_c._3_1_;
      *(ushort *)(param_1 + 0x166) = local_8 * 0x100 + (local_8 >> 8);
    }
    else {
      if (cVar1 != '\0') {
        pcVar4 = "tRNS chunk not allowed with alpha channel";
        goto LAB_0057ab15;
      }
      if (param_3 != 2) goto LAB_0057aaac;
      FUN_00579ab5(param_1,&local_c,2);
      *(ushort *)(param_1 + 0x168) = (ushort)local_c * 0x100 + ((ushort)local_c >> 8);
    }
    *(undefined2 *)(param_1 + 0x10a) = 1;
LAB_0057aae1:
    iVar3 = FUN_0057a385(param_1,0);
    if (iVar3 == 0) {
      FUN_005756f2(param_1,param_2,*(undefined4 *)(param_1 + 0x15c),*(undefined2 *)(param_1 + 0x10a)
                   ,param_1 + 0x160);
    }
  }
  return;
}


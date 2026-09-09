// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00596d99 | Name: FUN_00596d99


byte * FUN_00596d99(byte *param_1,uint param_2)

{
  byte bVar1;
  ushort uVar2;
  byte *pbVar3;
  byte bVar4;
  byte *pbVar5;
  bool bVar6;
  
  pbVar5 = (byte *)0x0;
  if (DAT_013b83cc == 0) {
    pbVar5 = (byte *)_strrchr((char *)param_1,param_2);
  }
  else {
    FUN_00596aad(0x19);
    do {
      bVar4 = *param_1;
      if ((*(byte *)((int)&DAT_013b84e0 + bVar4 + 1) & 4) == 0) {
        bVar6 = param_2 == bVar4;
LAB_00596df4:
        pbVar3 = param_1;
        if (bVar6) {
          pbVar5 = param_1;
        }
      }
      else {
        bVar1 = param_1[1];
        pbVar3 = param_1 + 1;
        if (bVar1 == 0) {
          bVar6 = pbVar5 == (byte *)0x0;
          param_1 = pbVar3;
          bVar4 = bVar1;
          goto LAB_00596df4;
        }
        uVar2 = CONCAT11(bVar4,bVar1);
        bVar4 = bVar1;
        if (param_2 == uVar2) {
          pbVar5 = param_1;
        }
      }
      param_1 = pbVar3 + 1;
    } while (bVar4 != 0);
    FUN_00596b0e(0x19);
  }
  return pbVar5;
}


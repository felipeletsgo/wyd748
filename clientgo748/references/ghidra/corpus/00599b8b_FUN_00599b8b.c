// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00599b8b | Name: FUN_00599b8b


byte * FUN_00599b8b(void)

{
  byte bVar1;
  int iVar2;
  byte *pbVar3;
  byte *pbVar4;
  
  if (DAT_013b8734 == 0) {
    FUN_00599b6f();
  }
  bVar1 = *DAT_013b872c;
  pbVar4 = DAT_013b872c;
  if (bVar1 == 0x22) {
    while( true ) {
      pbVar3 = pbVar4;
      bVar1 = pbVar3[1];
      pbVar4 = pbVar3 + 1;
      if ((bVar1 == 0x22) || (bVar1 == 0)) break;
      iVar2 = FUN_0059c9bb(bVar1);
      if (iVar2 != 0) {
        pbVar4 = pbVar3 + 2;
      }
    }
    if (*pbVar4 == 0x22) goto LAB_00599bc8;
  }
  else {
    while (0x20 < bVar1) {
      bVar1 = pbVar4[1];
      pbVar4 = pbVar4 + 1;
    }
  }
  for (; (*pbVar4 != 0 && (*pbVar4 < 0x21)); pbVar4 = pbVar4 + 1) {
LAB_00599bc8:
  }
  return pbVar4;
}


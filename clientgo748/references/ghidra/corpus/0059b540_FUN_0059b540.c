// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0059b540 | Name: FUN_0059b540


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

char FUN_0059b540(byte *param_1,byte *param_2)

{
  bool bVar1;
  int iVar2;
  byte bVar3;
  byte bVar4;
  byte bVar5;
  char cVar6;
  uint uVar7;
  
  iVar2 = _DAT_013b8728;
  if (DAT_013b82e4 == 0) {
    bVar5 = 0xff;
    do {
      do {
        if (bVar5 == 0) {
          return '\0';
        }
        bVar5 = *param_2;
        param_2 = param_2 + 1;
        bVar4 = *param_1;
        param_1 = param_1 + 1;
      } while (bVar4 == bVar5);
      bVar3 = bVar5 + 0xbf + (-((byte)(bVar5 + 0xbf) < 0x1a) & 0x20U) + 0x41;
      bVar4 = bVar4 + 0xbf;
      bVar5 = bVar4 + (-(bVar4 < 0x1a) & 0x20U) + 0x41;
    } while (bVar5 == bVar3);
    cVar6 = (bVar5 < bVar3) * -2 + '\x01';
  }
  else {
    LOCK();
    _DAT_013b8728 = _DAT_013b8728 + 1;
    UNLOCK();
    bVar1 = 0 < DAT_013b8724;
    if (bVar1) {
      LOCK();
      UNLOCK();
      _DAT_013b8728 = iVar2;
      FUN_00596aad(0x13);
    }
    uVar7 = (uint)bVar1;
    bVar5 = 0xff;
    do {
      do {
        cVar6 = '\0';
        if (bVar5 == 0) goto LAB_0059b5ef;
        bVar5 = *param_2;
        param_2 = param_2 + 1;
        bVar4 = *param_1;
        param_1 = param_1 + 1;
      } while (bVar5 == bVar4);
      bVar4 = FUN_0059a669(bVar4,bVar5);
      bVar5 = FUN_0059a669();
    } while (bVar4 == bVar5);
    cVar6 = (bVar4 < bVar5) * -2 + '\x01';
LAB_0059b5ef:
    if (uVar7 == 0) {
      LOCK();
      _DAT_013b8728 = _DAT_013b8728 + -1;
      UNLOCK();
    }
    else {
      FUN_00596b0e(0x13);
    }
  }
  return cVar6;
}


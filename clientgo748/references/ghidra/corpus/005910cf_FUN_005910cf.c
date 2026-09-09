// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 005910cf | Name: FUN_005910cf


int FUN_005910cf(byte *param_1)

{
  uint uVar1;
  uint uVar2;
  int iVar3;
  uint uVar4;
  byte *pbVar5;
  
  while( true ) {
    if (DAT_005cc31c < 2) {
      uVar1 = (byte)PTR_DAT_005cc110[(uint)*param_1 * 2] & 8;
    }
    else {
      uVar1 = FUN_005980ea(*param_1,8);
    }
    if (uVar1 == 0) break;
    param_1 = param_1 + 1;
  }
  uVar1 = (uint)*param_1;
  pbVar5 = param_1 + 1;
  if ((uVar1 == 0x2d) || (uVar4 = uVar1, uVar1 == 0x2b)) {
    uVar4 = (uint)*pbVar5;
    pbVar5 = param_1 + 2;
  }
  iVar3 = 0;
  while( true ) {
    if (DAT_005cc31c < 2) {
      uVar2 = (byte)PTR_DAT_005cc110[uVar4 * 2] & 4;
    }
    else {
      uVar2 = FUN_005980ea(uVar4,4);
    }
    if (uVar2 == 0) break;
    iVar3 = (uVar4 - 0x30) + iVar3 * 10;
    uVar4 = (uint)*pbVar5;
    pbVar5 = pbVar5 + 1;
  }
  if (uVar1 == 0x2d) {
    iVar3 = -iVar3;
  }
  return iVar3;
}


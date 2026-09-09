// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00597030 | Name: FUN_00597030


uint FUN_00597030(byte *param_1,byte *param_2)

{
  byte bVar1;
  uint uVar2;
  byte *pbVar3;
  byte *pbVar4;
  
  if (DAT_013b83cc == 0) {
    uVar2 = FUN_0059a5c0(param_1,param_2);
    return uVar2;
  }
  FUN_00596aad(0x19);
  bVar1 = *param_1;
  while (bVar1 != 0) {
    bVar1 = *param_2;
    pbVar3 = param_2;
    while (bVar1 != 0) {
      bVar1 = *pbVar3;
      if ((*(byte *)((int)&DAT_013b84e0 + bVar1 + 1) & 4) == 0) {
        pbVar4 = pbVar3;
        if (bVar1 == *param_1) break;
      }
      else if (((bVar1 == *param_1) && (pbVar3[1] == param_1[1])) ||
              (pbVar4 = pbVar3 + 1, pbVar3[1] == 0)) break;
      pbVar3 = pbVar4 + 1;
      bVar1 = *pbVar3;
    }
    if ((*pbVar3 != 0) ||
       (((*(byte *)((int)&DAT_013b84e0 + *param_1 + 1) & 4) != 0 &&
        (param_1 = param_1 + 1, *param_1 == 0)))) break;
    param_1 = param_1 + 1;
    bVar1 = *param_1;
  }
  FUN_00596b0e(0x19);
  return -(uint)(*param_1 != 0) & (uint)param_1;
}


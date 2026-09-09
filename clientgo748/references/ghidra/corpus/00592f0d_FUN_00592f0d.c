// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00592f0d | Name: FUN_00592f0d


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

float10 FUN_00592f0d(double param_1)

{
  double dVar1;
  uint uVar2;
  int iVar3;
  float10 fVar4;
  undefined4 uVar5;
  
  uVar2 = FUN_005989ad(DAT_005c9c2c,0xffff);
  if ((param_1._6_2_ & 0x7ff0) == 0x7ff0) {
    iVar3 = FUN_00598875();
    if (0 < iVar3) {
      if (iVar3 < 3) {
        FUN_005989ad(uVar2,0xffff);
        return (float10)param_1;
      }
      if (iVar3 == 3) {
        fVar4 = (float10)FUN_00598194();
        return fVar4;
      }
    }
    dVar1 = param_1 + _DAT_005a4c28;
    uVar5 = 8;
  }
  else {
    fVar4 = (float10)__frnd(SUB84(param_1,0),(int)((ulonglong)param_1 >> 0x20));
    dVar1 = (double)fVar4;
    if ((dVar1 == param_1) || ((uVar2 & 0x20) != 0)) {
      FUN_005989ad(uVar2,0xffff);
      return (float10)dVar1;
    }
    uVar5 = 0x10;
  }
  fVar4 = (float10)FUN_005981e8(uVar5,0xb,param_1,dVar1,uVar2);
  return fVar4;
}


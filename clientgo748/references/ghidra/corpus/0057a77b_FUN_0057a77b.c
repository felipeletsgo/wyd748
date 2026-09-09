// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0057a77b | Name: FUN_0057a77b


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_0057a77b(int param_1,int param_2,int param_3)

{
  uint uVar1;
  float fVar2;
  int iVar3;
  int iVar4;
  int iVar5;
  
  iVar4 = param_2;
  iVar3 = param_1;
  uVar1 = *(uint *)(param_1 + 0x58);
  if ((uVar1 & 1) == 0) {
    FUN_005733d1(param_1,"Missing IHDR before gAMA");
  }
  else {
    if ((uVar1 & 4) != 0) goto LAB_0057a7ac;
    if ((uVar1 & 2) == 0) {
      if (((param_2 != 0) && ((*(uint *)(param_2 + 8) & 1) != 0)) &&
         ((*(uint *)(param_2 + 8) & 0x800) == 0)) goto LAB_0057a7ac;
    }
    else {
      FUN_005733ef(param_1,"Out of place gAMA chunk");
    }
  }
  if (param_3 == 4) {
    FUN_00579ab5(iVar3,&param_1,4);
    iVar5 = FUN_0057a385(iVar3,0);
    if (iVar5 != 0) {
      return;
    }
    iVar5 = FUN_00579a92();
    if (iVar5 == 0) {
      return;
    }
    if ((*(uint *)(iVar4 + 8) & 0x800) != 0) {
      fVar2 = (float)iVar5;
      if (iVar5 < 0) {
        fVar2 = fVar2 + _DAT_005a67dc;
      }
      if ((float)_DAT_005a8d68 < ABS(fVar2 - (float)_DAT_005a8d70)) {
        FUN_005733ef(iVar3,"Ignoring incorrect gAMA value when sRGB is also present");
        return;
      }
    }
    fVar2 = (float)iVar5;
    if (iVar5 < 0) {
      fVar2 = fVar2 + _DAT_005a67dc;
    }
    fVar2 = fVar2 * _DAT_005a6948;
    *(float *)(iVar3 + 0x130) = fVar2;
    FUN_005755be(iVar3,iVar4,(double)fVar2);
    return;
  }
LAB_0057a7ac:
  FUN_005733ef(iVar3);
  FUN_0057a385(iVar3,param_3);
  return;
}


// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0056365b | Name: FUN_0056365b


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __thiscall FUN_0056365b(undefined4 param_1,float *param_2,float *param_3)

{
  float fVar1;
  int iVar2;
  float *extraout_ECX;
  float fVar3;
  
  fVar1 = param_3[1] * param_3[1] + *param_3 * *param_3;
  iVar2 = FUN_00560f73(fVar1,0x3f800000,param_1,param_1);
  if (iVar2 == 0) {
    if (fVar1 <= (float)PTR_DAT_005a6810) {
      *param_2 = 0.0;
      param_2[1] = 0.0;
      return;
    }
    fVar3 = (_DAT_005a67d8 / SQRT(fVar1)) * extraout_ECX[1];
    *param_2 = (_DAT_005a67d8 / SQRT(fVar1)) * *extraout_ECX;
  }
  else {
    if (param_2 == extraout_ECX) {
      return;
    }
    *param_2 = *extraout_ECX;
    fVar3 = extraout_ECX[1];
  }
  param_2[1] = fVar3;
  return;
}


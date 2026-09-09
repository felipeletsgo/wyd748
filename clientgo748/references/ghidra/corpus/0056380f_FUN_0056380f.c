// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0056380f | Name: FUN_0056380f


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_0056380f(float *param_1,float *param_2)

{
  float *pfVar1;
  float fVar2;
  int iVar3;
  float local_10;
  float local_c;
  float local_8;
  
  fVar2 = param_2[2] * param_2[2] + param_2[1] * param_2[1] + *param_2 * *param_2;
  iVar3 = FUN_00560f73(fVar2,0x3f800000);
  if (iVar3 == 0) {
    if (fVar2 <= (float)PTR_DAT_005a6810) {
      *param_1 = 0.0;
      param_1[1] = 0.0;
      param_1[2] = 0.0;
      return;
    }
    fVar2 = _DAT_005a67d8 / SQRT(fVar2);
    local_10 = fVar2 * *param_2;
    local_c = fVar2 * param_2[1];
    pfVar1 = param_2 + 2;
    param_2 = &local_10;
    local_8 = fVar2 * *pfVar1;
  }
  else if (param_1 == param_2) {
    return;
  }
  *param_1 = *param_2;
  param_1[1] = param_2[1];
  param_1[2] = param_2[2];
  return;
}


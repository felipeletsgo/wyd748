// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00598194 | Name: FUN_00598194


float10 FUN_00598194(undefined4 param_1,double param_2,undefined4 param_3)

{
  undefined4 *puVar1;
  float10 fVar2;
  
  if (DAT_005cc790 == 0) {
    fVar2 = (float10)FUN_0059874a(1,param_1,param_2,0,param_2,param_3);
    return fVar2;
  }
  puVar1 = (undefined4 *)FUN_00594c1d();
  *puVar1 = 0x21;
  FUN_005989ad();
  return (float10)param_2;
}


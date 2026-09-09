// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 005981e8 | Name: FUN_005981e8


float10 FUN_005981e8(undefined4 param_1,undefined4 param_2,undefined8 param_3,double param_4,
                    undefined4 param_5)

{
  int iVar1;
  float10 fVar2;
  
  iVar1 = FUN_00598533(param_1,&param_4,param_5);
  if (iVar1 == 0) {
    FUN_00598280();
  }
  iVar1 = FUN_0059881f(param_1);
  if ((DAT_005cc790 == 0) && (iVar1 != 0)) {
    fVar2 = (float10)FUN_0059874a(iVar1,param_2,param_3,0,param_4,param_5);
    return fVar2;
  }
  FUN_005987d2(iVar1);
  FUN_005989ad();
  return (float10)param_4;
}


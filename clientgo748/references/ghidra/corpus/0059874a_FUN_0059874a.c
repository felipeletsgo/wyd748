// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0059874a | Name: FUN_0059874a


float10 FUN_0059874a(undefined4 param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4,
                    undefined4 param_5,undefined4 param_6,undefined4 param_7,undefined4 param_8,
                    undefined4 param_9)

{
  int iVar1;
  undefined4 local_24;
  int local_20;
  undefined4 local_1c;
  undefined4 local_18;
  undefined4 local_14;
  undefined4 local_10;
  undefined4 local_c;
  undefined4 uStack_8;
  
  local_20 = FUN_005987fa(param_2);
  if (local_20 != 0) {
    local_1c = param_3;
    local_18 = param_4;
    local_14 = param_5;
    local_10 = param_6;
    local_24 = param_1;
    local_c = param_7;
    uStack_8 = param_8;
    FUN_005989ad(param_9,0xffff);
    iVar1 = FUN_005840ee(&local_24);
    if (iVar1 == 0) {
      FUN_005987d2(param_1);
    }
    return (float10)(double)CONCAT44(uStack_8,local_c);
  }
  FUN_005989ad(param_9,0xffff);
  FUN_005987d2(param_1);
  return (float10)(double)CONCAT44(param_8,param_7);
}


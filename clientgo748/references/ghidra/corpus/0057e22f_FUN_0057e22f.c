// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0057e22f | Name: FUN_0057e22f


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_0057e22f(undefined4 param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4,
                 undefined4 param_5,uint param_6)

{
  ulonglong uVar1;
  undefined8 uVar2;
  undefined1 local_20 [16];
  undefined1 local_10 [16];
  
  FUN_0057e06b(local_20,param_2,param_5,param_6);
  FUN_0057e06b(local_10,param_3,param_4,param_6);
  uVar1 = (ulonglong)param_6;
  uVar2 = PackedFloatingMUL(uVar1,uVar1);
  uVar2 = PackedFloatingSUB(uVar1,uVar2);
  uVar2 = PackedFloatingMUL(uVar2,_DAT_005a86a8);
  FUN_0057e06b(param_1,local_20,local_10,(int)uVar2);
  FastExitMediaState();
  return;
}


// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0054a097 | Name: FUN_0054a097


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_0054a097(float param_1,undefined4 param_2,float param_3)

{
  undefined4 *puVar1;
  undefined4 uVar2;
  
  FUN_0053e47d(param_1,param_2,param_3);
  uVar2 = 0;
  puVar1 = (undefined4 *)FUN_00430f20(param_1 + _DAT_005a365c,param_3 - _DAT_005a365c);
  FUN_0049da58(*puVar1,puVar1[1],uVar2);
  return;
}


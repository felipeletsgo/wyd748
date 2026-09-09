// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004b7430 | Name: FUN_004b7430


undefined4 FUN_004b7430(undefined4 param_1,undefined4 param_2,undefined4 param_3)

{
  undefined4 uVar1;
  undefined4 uVar2;
  undefined4 uVar3;
  uint uVar4;
  undefined1 local_24 [20];
  void *local_10;
  undefined1 *puStack_c;
  uint local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_005a0620;
  local_10 = ExceptionList;
  uVar4 = 0;
  ExceptionList = &local_10;
  uVar1 = FUN_004b62e0(param_2,local_24);
  local_8 = 1;
  uVar3 = uVar1;
  uVar2 = FUN_004b6370(param_3);
  FUN_004b62a0(uVar2);
  local_8 = local_8 & 0xffffff00;
  FUN_004b6310(uVar1,uVar3,uVar4 | 1);
  ExceptionList = local_10;
  return param_1;
}


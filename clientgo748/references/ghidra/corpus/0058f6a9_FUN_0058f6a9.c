// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0058f6a9 | Name: FUN_0058f6a9


undefined4 FUN_0058f6a9(undefined4 param_1,undefined4 param_2)

{
  undefined4 uVar1;
  undefined4 uVar2;
  
  FUN_00591bb3(param_1);
  uVar1 = FUN_005948bb(param_1);
  uVar2 = FUN_00594029(param_1,param_2,&stack0x0000000c);
  FUN_00594948(uVar1,param_1);
  FUN_00591c05(param_1);
  return uVar2;
}


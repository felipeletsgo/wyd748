// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00591822 | Name: FUN_00591822


undefined4 FUN_00591822(undefined4 param_1)

{
  undefined4 uVar1;
  undefined4 uVar2;
  
  FUN_00591be2(1,&DAT_005c9998);
  uVar1 = FUN_005948bb(&DAT_005c9998);
  uVar2 = FUN_00594029(&DAT_005c9998,param_1,&stack0x00000008);
  FUN_00594948(uVar1,&DAT_005c9998);
  FUN_00591c34(1,&DAT_005c9998);
  return uVar2;
}


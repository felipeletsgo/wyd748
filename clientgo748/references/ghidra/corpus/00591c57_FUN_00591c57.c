// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00591c57 | Name: FUN_00591c57


undefined4 FUN_00591c57(int param_1)

{
  undefined4 uVar1;
  
  if (param_1 == 0) {
    uVar1 = FUN_00591d19(0);
    return uVar1;
  }
  FUN_00591bb3(param_1);
  uVar1 = FUN_00591c86(param_1);
  FUN_00591c05(param_1);
  return uVar1;
}


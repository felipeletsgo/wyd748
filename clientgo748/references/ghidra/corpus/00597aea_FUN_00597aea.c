// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00597aea | Name: FUN_00597aea


uint FUN_00597aea(uint param_1)

{
  uint uVar1;
  
  if (DAT_005cc31c < 2) {
    uVar1 = (byte)PTR_DAT_005cc110[param_1 * 2] & 4;
  }
  else {
    uVar1 = FUN_005980ea(param_1,4);
  }
  if (uVar1 == 0) {
    param_1 = (param_1 & 0xffffffdf) - 7;
  }
  return param_1;
}


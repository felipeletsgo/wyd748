// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00430c26 | Name: FUN_00430c26


undefined4 FUN_00430c26(undefined4 param_1,uint param_2)

{
  undefined4 uVar1;
  
  if ((param_2 & 0x40) == 0) {
    uVar1 = 0;
  }
  else {
    uVar1 = 0x80004005;
  }
  return uVar1;
}


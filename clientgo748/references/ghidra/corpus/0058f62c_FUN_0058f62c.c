// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0058f62c | Name: FUN_0058f62c


undefined4 FUN_0058f62c(int param_1)

{
  undefined4 uVar1;
  
  uVar1 = 0xffffffff;
  if ((*(byte *)(param_1 + 0xc) & 0x40) == 0) {
    FUN_00591bb3(param_1);
    uVar1 = __fclose_lk(param_1);
    FUN_00591c05(param_1);
  }
  else {
    *(undefined4 *)(param_1 + 0xc) = 0;
  }
  return uVar1;
}


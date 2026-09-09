// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0059161d | Name: FUN_0059161d


uint FUN_0059161d(uint param_1,uint param_2)

{
  uint uVar1;
  bool in_ZF;
  short in_FPUControlWord;
  
  if (in_ZF) {
    if (((param_2 & 0xfffff) != 0) || (param_1 != 0)) {
      uVar1 = FUN_005954cc();
      goto LAB_0059169b;
    }
    param_1 = param_2 & 0x80000000;
    param_2 = 0;
joined_r0x0059168c:
    if (param_1 == 0) {
LAB_0059163e:
      if (DAT_013b7fc0 != 0) {
        return param_2;
      }
      uVar1 = __math_exit();
      return uVar1;
    }
  }
  else {
    if (in_FPUControlWord != 0x27f) {
      param_2 = FUN_005954b5();
    }
    if ((param_2 & 0x80000000) == 0) goto LAB_0059163e;
    if (((param_2 & 0x7ff00000) == 0) && ((param_2 & 0xfffff) == 0)) goto joined_r0x0059168c;
  }
  uVar1 = 1;
LAB_0059169b:
  if (DAT_013b7fc0 != 0) {
    return uVar1;
  }
  uVar1 = __startOneArgErrorHandling();
  return uVar1;
}


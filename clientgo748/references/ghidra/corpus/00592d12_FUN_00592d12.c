// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00592d12 | Name: FUN_00592d12


void FUN_00592d12(undefined4 param_1,undefined4 param_2,int param_3)

{
  uint in_EAX;
  uint extraout_ECX;
  uint uVar1;
  short in_FPUControlWord;
  
  uVar1 = in_EAX;
  if (in_FPUControlWord != 0x27f) {
    in_EAX = FUN_005954b5();
    uVar1 = extraout_ECX;
  }
  if ((uVar1 & 0x7ff00000) != 0x7ff00000) {
                    /* WARNING: Subroutine does not return */
    __fload_withFB();
  }
  if ((in_EAX & 0xfffff) == 0 && param_3 == 0) {
                    /* WARNING: Subroutine does not return */
    __fload_withFB();
  }
                    /* WARNING: Subroutine does not return */
  __fload_withFB();
}


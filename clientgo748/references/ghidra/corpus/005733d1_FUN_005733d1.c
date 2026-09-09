// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 005733d1 | Name: FUN_005733d1


void FUN_005733d1(int *param_1,undefined4 param_2)

{
  if ((code *)param_1[0x10] != (code *)0x0) {
    (*(code *)param_1[0x10])(param_1,param_2);
  }
                    /* WARNING: Subroutine does not return */
  _longjmp(param_1,1);
}


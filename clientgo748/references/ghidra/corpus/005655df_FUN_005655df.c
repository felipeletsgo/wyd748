// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 005655df | Name: FUN_005655df


void FUN_005655df(int *param_1)

{
  (**(code **)(*param_1 + 8))(param_1);
                    /* WARNING: Subroutine does not return */
  _longjmp((int *)(*param_1 + 0x84),1);
}


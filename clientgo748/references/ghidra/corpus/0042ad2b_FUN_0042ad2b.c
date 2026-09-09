// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0042ad2b | Name: FUN_0042ad2b


int __thiscall FUN_0042ad2b(int param_1,undefined4 param_2,undefined4 param_3)

{
  int iVar1;
  int *piVar2;
  int local_8;
  
  if (*(int *)(DAT_005ccf98 + 0x21008) == 1) {
    iVar1 = 0;
  }
  else if (*(int *)(param_1 + 4) == 0) {
    iVar1 = -0x7ffbfe10;
  }
  else {
    piVar2 = (int *)FUN_0042abe2();
    if (piVar2 == (int *)0x0) {
      iVar1 = -0x7fffbffb;
    }
    else {
      iVar1 = FUN_0042ab3f(piVar2,&local_8);
      if (-1 < iVar1) {
        if (local_8 != 0) {
          iVar1 = FUN_0042a97c(piVar2,0);
          if (iVar1 < 0) {
            return iVar1;
          }
          FUN_0042af62();
        }
        iVar1 = (**(code **)(*piVar2 + 0x30))(piVar2,0,param_2,param_3);
      }
    }
  }
  return iVar1;
}


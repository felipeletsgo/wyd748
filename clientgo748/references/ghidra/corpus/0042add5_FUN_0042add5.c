// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0042add5 | Name: FUN_0042add5


int __thiscall FUN_0042add5(int param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4)

{
  int iVar1;
  int *piVar2;
  int *local_c;
  int local_8;
  
  if (*(int *)(param_1 + 4) == 0) {
    iVar1 = -0x7ffbfe10;
  }
  else {
    local_c = (int *)0x0;
    piVar2 = (int *)FUN_0042abe2();
    if (piVar2 == (int *)0x0) {
      iVar1 = -0x7fffbffb;
    }
    else {
      iVar1 = FUN_0042ab3f(piVar2,&local_8);
      if (-1 < iVar1) {
        (**(code **)*piVar2)(piVar2,&DAT_005a9e9c,&local_c);
        if (local_c == (int *)0x0) {
          iVar1 = -0x7fffbffb;
        }
        else {
          (**(code **)(**(int **)(DAT_005ccf98 + 0x2100c) + 0x28))
                    (*(undefined4 *)(DAT_005ccf98 + 0x2100c),DAT_005ccf98 + 0x21010,0);
          (**(code **)(*local_c + 0x30))(local_c,param_4,0);
          if (local_8 != 0) {
            iVar1 = FUN_0042a97c(piVar2,0);
            if (iVar1 < 0) {
              return iVar1;
            }
            FUN_0042af62();
          }
          if (local_c != (int *)0x0) {
            (**(code **)(*local_c + 8))(local_c);
            local_c = (int *)0x0;
          }
          iVar1 = (**(code **)(*piVar2 + 0x30))(piVar2,0,param_2,param_3);
        }
      }
    }
  }
  return iVar1;
}


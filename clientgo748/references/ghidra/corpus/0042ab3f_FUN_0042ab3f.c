// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0042ab3f | Name: FUN_0042ab3f


int __thiscall FUN_0042ab3f(undefined4 param_1,int *param_2,undefined4 *param_3)

{
  int iVar1;
  uint local_8;
  
  if (param_2 == (int *)0x0) {
    iVar1 = -0x7ffbfe10;
  }
  else {
    if (param_3 != (undefined4 *)0x0) {
      *param_3 = 0;
    }
    iVar1 = (**(code **)(*param_2 + 0x24))(param_2,&local_8,param_1);
    if (-1 < iVar1) {
      if ((local_8 & 2) == 0) {
        iVar1 = 1;
      }
      else {
        do {
          iVar1 = (**(code **)(*param_2 + 0x50))(param_2);
          if (iVar1 == -0x7787ff6a) {
            Sleep(10);
          }
          iVar1 = (**(code **)(*param_2 + 0x50))(param_2);
        } while (iVar1 != 0);
        if (param_3 != (undefined4 *)0x0) {
          *param_3 = 1;
        }
        iVar1 = 0;
      }
    }
  }
  return iVar1;
}


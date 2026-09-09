// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0040cdd7 | Name: FUN_0040cdd7


int * __thiscall FUN_0040cdd7(int param_1,int param_2)

{
  int *piVar1;
  int iVar2;
  int *local_8;
  
  piVar1 = *(int **)(param_1 + 0x28);
  local_8 = piVar1;
  do {
    if (local_8 == (int *)0x0) {
      return (int *)0x0;
    }
    if ((char)local_8[5] == '\0') {
      iVar2 = (**(code **)(*local_8 + 0x48))();
      if (iVar2 == param_2) {
        return local_8;
      }
      if (local_8[4] == 0) goto LAB_0040ce2d;
      local_8 = (int *)local_8[4];
    }
    else {
LAB_0040ce2d:
      while (local_8[3] == 0) {
        local_8 = (int *)local_8[1];
        if ((local_8 == piVar1) || (local_8 == (int *)0x0)) goto LAB_0040ce58;
      }
      local_8 = (int *)local_8[3];
    }
LAB_0040ce58:
    if (local_8 == piVar1) {
      return (int *)0x0;
    }
  } while( true );
}


// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0040c945 | Name: FUN_0040c945


undefined4 __thiscall FUN_0040c945(int param_1,undefined4 param_2)

{
  int *piVar1;
  int iVar2;
  int *local_8;
  
  piVar1 = *(int **)(param_1 + 0x28);
  local_8 = piVar1;
  do {
    if (local_8 == (int *)0x0) {
      return 0;
    }
    if (((char)local_8[5] == '\0') && (local_8[10] == 1)) {
      iVar2 = (**(code **)(*local_8 + 0x10))(param_2);
      if (iVar2 == 1) {
        return 1;
      }
      if (local_8[4] == 0) goto LAB_0040c9aa;
      local_8 = (int *)local_8[4];
    }
    else {
LAB_0040c9aa:
      while (local_8[3] == 0) {
        local_8 = (int *)local_8[1];
        if ((local_8 == piVar1) || (local_8 == (int *)0x0)) goto LAB_0040c9d5;
      }
      local_8 = (int *)local_8[3];
    }
LAB_0040c9d5:
    if (local_8 == piVar1) {
      return 0;
    }
  } while( true );
}


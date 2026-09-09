// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004b2aba | Name: FUN_004b2aba


void __thiscall FUN_004b2aba(int param_1,undefined4 param_2,undefined4 param_3)

{
  int *piVar1;
  int iVar2;
  int *local_8;
  
  piVar1 = *(int **)(param_1 + 0x1b07c);
  local_8 = piVar1;
  if (piVar1 == (int *)0x0) {
    return;
  }
  do {
    if ((char)local_8[5] == '\0') {
      iVar2 = (**(code **)(*local_8 + 0x18))(param_2,param_3);
      if (iVar2 == 1) {
        return;
      }
      if (local_8[4] == 0) goto LAB_004b2b16;
      local_8 = (int *)local_8[4];
    }
    else {
LAB_004b2b16:
      while (local_8[3] == 0) {
        local_8 = (int *)local_8[1];
        if ((local_8 == piVar1) || (local_8 == (int *)0x0)) goto LAB_004b2b41;
      }
      local_8 = (int *)local_8[3];
    }
LAB_004b2b41:
    if (local_8 == piVar1) {
      return;
    }
    if (local_8 == (int *)0x0) {
      return;
    }
  } while( true );
}


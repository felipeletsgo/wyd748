// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004b29b9 | Name: FUN_004b29b9


void __thiscall FUN_004b29b9(undefined4 param_1,undefined1 param_2,undefined4 param_3)

{
  int *piVar1;
  int iVar2;
  int *local_8;
  
  piVar1 = DAT_0067cf38;
  local_8 = DAT_0067cf38;
  if (DAT_0067cf38 == (int *)0x0) {
    return;
  }
  do {
    if ((char)local_8[5] == '\0') {
      iVar2 = (**(code **)(*local_8 + 0x14))
                        (CONCAT31((int3)((uint)local_8 >> 8),param_2),param_3,param_1);
      if (iVar2 == 1) {
        return;
      }
      if (local_8[4] == 0) goto LAB_004b2a11;
      local_8 = (int *)local_8[4];
    }
    else {
LAB_004b2a11:
      while (local_8[3] == 0) {
        local_8 = (int *)local_8[1];
        if ((local_8 == piVar1) || (local_8 == (int *)0x0)) goto LAB_004b2a3c;
      }
      local_8 = (int *)local_8[3];
    }
LAB_004b2a3c:
    if (local_8 == piVar1) {
      return;
    }
    if (local_8 == (int *)0x0) {
      return;
    }
  } while( true );
}


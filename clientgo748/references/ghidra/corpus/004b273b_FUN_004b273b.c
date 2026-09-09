// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004b273b | Name: FUN_004b273b


void __thiscall
FUN_004b273b(undefined4 param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4,
            undefined4 param_5)

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
      iVar2 = (**(code **)(*local_8 + 8))(param_2,param_3,param_4,param_5,param_1);
      if (iVar2 == 1) {
        return;
      }
      if (local_8[4] == 0) goto LAB_004b279b;
      local_8 = (int *)local_8[4];
    }
    else {
LAB_004b279b:
      while (local_8[3] == 0) {
        local_8 = (int *)local_8[1];
        if ((local_8 == piVar1) || (local_8 == (int *)0x0)) goto LAB_004b27c6;
      }
      local_8 = (int *)local_8[3];
    }
LAB_004b27c6:
    if (local_8 == piVar1) {
      return;
    }
    if (local_8 == (int *)0x0) {
      return;
    }
  } while( true );
}


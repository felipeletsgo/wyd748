// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004b3205 | Name: FUN_004b3205


void __fastcall FUN_004b3205(undefined4 param_1)

{
  int *piVar1;
  int iVar2;
  int *local_8;
  
  piVar1 = DAT_0067cf38;
  local_8 = DAT_0067cf38;
  if (DAT_0067cf38 != (int *)0x0) {
    do {
      if (local_8 == (int *)0x0) {
        return;
      }
      if ((((char)local_8[5] == '\0') &&
          (iVar2 = (**(code **)(*local_8 + 0x24))(param_1), iVar2 != 0)) && (local_8[4] != 0)) {
        local_8 = (int *)local_8[4];
      }
      else {
        while (local_8[3] == 0) {
          local_8 = (int *)local_8[1];
          if ((local_8 == piVar1) || (local_8 == (int *)0x0)) goto LAB_004b3290;
        }
        local_8 = (int *)local_8[3];
      }
LAB_004b3290:
    } while (local_8 != piVar1);
  }
  return;
}


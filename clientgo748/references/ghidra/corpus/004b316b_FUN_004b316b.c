// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004b316b | Name: FUN_004b316b


void __thiscall FUN_004b316b(undefined4 param_1,int *param_2)

{
  int iVar1;
  int *local_8;
  
  local_8 = param_2;
  if (param_2 != (int *)0x0) {
    do {
      if (local_8 == (int *)0x0) {
        return;
      }
      if ((((char)local_8[5] == '\0') &&
          (iVar1 = (**(code **)(*local_8 + 0x24))(param_1), iVar1 != 0)) && (local_8[4] != 0)) {
        local_8 = (int *)local_8[4];
      }
      else {
        while (local_8[3] == 0) {
          local_8 = (int *)local_8[1];
          if ((local_8 == param_2) || (local_8 == (int *)0x0)) goto LAB_004b31f1;
        }
        local_8 = (int *)local_8[3];
      }
LAB_004b31f1:
    } while (local_8 != param_2);
  }
  return;
}


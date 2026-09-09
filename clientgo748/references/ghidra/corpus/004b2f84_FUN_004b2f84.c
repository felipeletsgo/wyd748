// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004b2f84 | Name: FUN_004b2f84


void __fastcall FUN_004b2f84(undefined4 param_1)

{
  int *piVar1;
  int *local_8;
  
  piVar1 = DAT_0067cf38;
  local_8 = DAT_0067cf38;
  do {
    if (local_8 == (int *)0x0) {
      return;
    }
    (**(code **)(*local_8 + 0x2c))(param_1);
    if (local_8[4] == 0) {
      while (local_8[3] == 0) {
        local_8 = (int *)local_8[1];
        if ((local_8 == piVar1) || (local_8 == (int *)0x0)) goto LAB_004b2fed;
      }
      local_8 = (int *)local_8[3];
    }
    else {
      local_8 = (int *)local_8[4];
    }
LAB_004b2fed:
    if (local_8 == piVar1) {
      return;
    }
  } while( true );
}


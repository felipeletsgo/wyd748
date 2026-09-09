// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00428618 | Name: FUN_00428618


undefined1 __thiscall FUN_00428618(undefined4 *param_1,int param_2)

{
  int iVar1;
  undefined4 uStack_2c;
  int **ppiStack_28;
  undefined1 *local_24;
  undefined1 *local_20;
  undefined4 *local_1c;
  undefined1 local_18;
  int *local_14;
  undefined1 *local_10;
  int local_c;
  int local_8;
  
  local_18 = 0;
  if (*(char *)(param_1 + 5) == '\0') {
    local_18 = 0;
  }
  else {
    ppiStack_28 = &local_14;
    uStack_2c = *param_1;
    local_1c = param_1;
    iVar1 = (**(code **)(*(int *)*param_1 + 0x14))();
    if (iVar1 == 0) {
      local_8 = 0;
      local_c = 0;
      while( true ) {
        iVar1 = (**(code **)(*local_14 + 0x10))(local_14,1);
        if (iVar1 != 0) break;
        local_8 = local_8 + 1;
      }
      FUN_0058f730();
      local_20 = (undefined1 *)&uStack_2c;
      local_10 = (undefined1 *)&uStack_2c;
      (**(code **)(*local_14 + 0x14))(local_14);
      do {
        local_24 = local_10 + local_c * 4;
        local_c = local_c + 1;
        iVar1 = (**(code **)(*local_14 + 0xc))(local_14,1,local_24,0);
      } while (iVar1 == 0);
      if (local_14 != (int *)0x0) {
        (**(code **)(*local_14 + 8))(local_14);
        local_14 = (int *)0x0;
      }
      for (local_c = 0; local_c < local_8; local_c = local_c + 1) {
        if (param_2 == *(int *)(local_10 + local_c * 4)) {
          local_18 = 1;
        }
        do {
          iVar1 = (**(code **)(**(int **)(local_10 + local_c * 4) + 8))
                            (*(undefined4 *)(local_10 + local_c * 4));
        } while (iVar1 != 0);
      }
    }
    else {
      local_18 = 0;
    }
  }
  return local_18;
}


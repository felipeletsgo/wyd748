// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00428412 | Name: FUN_00428412


uint __fastcall FUN_00428412(undefined4 *param_1)

{
  uint uVar1;
  int iVar2;
  undefined4 uStack_28;
  int **ppiStack_24;
  undefined1 *local_20;
  undefined1 *local_1c;
  undefined4 *local_18;
  int *local_14;
  undefined1 *local_10;
  int local_c;
  int local_8;
  
  if (*(char *)(param_1 + 5) == '\0') {
    uVar1 = (uint)param_1 & 0xffffff00;
  }
  else {
    local_18 = param_1;
    if (param_1[1] != 0) {
      ppiStack_24 = (int **)param_1[1];
      uStack_28 = 0x428449;
      (**(code **)(*(int *)param_1[1] + 0x24))();
    }
    ppiStack_24 = &local_14;
    uStack_28 = *local_18;
    uVar1 = (**(code **)(*(int *)*local_18 + 0x14))();
    if (uVar1 == 0) {
      local_8 = 0;
      local_c = 0;
      while( true ) {
        iVar2 = (**(code **)(*local_14 + 0x10))(local_14,1);
        if (iVar2 != 0) break;
        local_8 = local_8 + 1;
      }
      FUN_0058f730();
      local_1c = (undefined1 *)&uStack_28;
      local_10 = (undefined1 *)&uStack_28;
      (**(code **)(*local_14 + 0x14))(local_14);
      do {
        local_20 = local_10 + local_c * 4;
        local_c = local_c + 1;
        iVar2 = (**(code **)(*local_14 + 0xc))(local_14,1,local_20,0);
      } while (iVar2 == 0);
      if (local_14 != (int *)0x0) {
        (**(code **)(*local_14 + 8))(local_14);
        local_14 = (int *)0x0;
      }
      for (local_c = 0; local_c < local_8; local_c = local_c + 1) {
        (**(code **)(*(int *)*local_18 + 0x10))(*local_18,*(undefined4 *)(local_10 + local_c * 4));
        do {
          iVar2 = (**(code **)(**(int **)(local_10 + local_c * 4) + 8))
                            (*(undefined4 *)(local_10 + local_c * 4));
        } while (iVar2 != 0);
      }
      uVar1 = CONCAT31((int3)((uint)local_c >> 8),1);
    }
    else {
      uVar1 = uVar1 & 0xffffff00;
    }
  }
  return uVar1;
}


// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00427f1c | Name: FUN_00427f1c


undefined4 __fastcall FUN_00427f1c(int *param_1)

{
  bool bVar1;
  undefined4 uVar2;
  int iVar3;
  uint local_20;
  uint local_1c;
  int *local_14;
  
  local_14 = param_1 + param_1[0xa8e0] * 0x10e3 +
                       param_1[param_1[0xa8e0] * 0x10e3 + 0x10e3] * 0x32a + 0x111;
  bVar1 = false;
  if (local_14[0x37] == 0) {
    for (local_20 = 0; local_20 < (uint)param_1[0xa8df]; local_20 = local_20 + 1) {
      for (local_1c = 0; local_1c < (uint)param_1[local_20 * 0x10e3 + 0x110];
          local_1c = local_1c + 1) {
        if ((param_1 + local_20 * 0x10e3 + local_1c * 0x32a + 0x111)[0x37] != 0) {
          param_1[0xa8e0] = local_20;
          param_1[local_20 * 0x10e3 + 0x10e3] = local_1c;
          bVar1 = true;
          local_14 = param_1 + local_20 * 0x10e3 + local_1c * 0x32a + 0x111;
          break;
        }
      }
      if (bVar1) break;
    }
  }
  else {
    bVar1 = true;
  }
  if (bVar1) {
    local_14[0x328] = 1;
    param_1[0xa8e1] = 1;
    param_1[0xa8e3] = 0;
    (**(code **)(*param_1 + 0x14))();
    (**(code **)(*param_1 + 0x18))();
    iVar3 = (**(code **)(*(int *)param_1[0xa8f9] + 8))(param_1[0xa8f9]);
    if (iVar3 == 0) {
      iVar3 = FUN_00427119();
      if (iVar3 < 0) {
        uVar2 = FUN_00425d36(iVar3,1);
      }
      else {
        param_1[0xa8e3] = 1;
        uVar2 = 0;
      }
    }
    else {
      uVar2 = FUN_00425d36(0x8200000a,1);
    }
  }
  else {
    uVar2 = 0x80004005;
  }
  return uVar2;
}


// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00586549 | Name: FUN_00586549


void FUN_00586549(int *param_1)

{
  undefined4 *puVar1;
  int iVar2;
  int iVar3;
  bool bVar4;
  int iVar5;
  int *piVar6;
  int iVar7;
  bool bVar8;
  bool bVar9;
  undefined4 *local_14;
  int *local_10;
  int local_8;
  
  iVar2 = param_1[0x65];
  bVar8 = param_1[0x5a] != 0;
  bVar4 = false;
  if (bVar8) {
    if ((param_1[0x5b] < param_1[0x5a]) || (0x3f < param_1[0x5b])) {
      bVar4 = true;
    }
    bVar9 = param_1[0x48] == 1;
  }
  else {
    bVar9 = param_1[0x5b] == 0;
  }
  if (!bVar9) {
    bVar4 = true;
  }
  if ((param_1[0x5c] != 0) && (param_1[0x5d] != param_1[0x5c] + -1)) {
    bVar4 = true;
  }
  if (0xd < param_1[0x5d]) {
    bVar4 = true;
  }
  if (bVar4) {
    *(undefined4 *)(*param_1 + 0x14) = 0xe;
    *(int *)(*param_1 + 0x18) = param_1[0x5a];
    *(int *)(*param_1 + 0x1c) = param_1[0x5b];
    *(int *)(*param_1 + 0x20) = param_1[0x5c];
    *(int *)(*param_1 + 0x24) = param_1[0x5d];
    (**(code **)*param_1)(param_1);
  }
  local_8 = 0;
  if (0 < param_1[0x48]) {
    local_10 = param_1 + 0x49;
    do {
      iVar3 = *(int *)(*local_10 + 4);
      piVar6 = (int *)(iVar3 * 0x100 + param_1[0x23]);
      if ((bVar8) && (*piVar6 < 0)) {
        *(undefined4 *)(*param_1 + 0x14) = 0x6f;
        *(int *)(*param_1 + 0x18) = iVar3;
        *(undefined4 *)(*param_1 + 0x1c) = 0;
        (**(code **)(*param_1 + 4))(param_1,0xffffffff);
      }
      for (iVar7 = param_1[0x5a]; iVar7 <= param_1[0x5b]; iVar7 = iVar7 + 1) {
        iVar5 = piVar6[iVar7];
        if (iVar5 < 0) {
          iVar5 = 0;
        }
        if (param_1[0x5c] != iVar5) {
          *(undefined4 *)(*param_1 + 0x14) = 0x6f;
          *(int *)(*param_1 + 0x18) = iVar3;
          *(int *)(*param_1 + 0x1c) = iVar7;
          (**(code **)(*param_1 + 4))(param_1,0xffffffff);
        }
        piVar6[iVar7] = param_1[0x5d];
      }
      local_8 = local_8 + 1;
      local_10 = local_10 + 1;
    } while (local_8 < param_1[0x48]);
  }
  if (param_1[0x5c] == 0) {
    if (bVar8) {
      *(code **)(iVar2 + 4) = FUN_00585ed0;
    }
    else {
      *(code **)(iVar2 + 4) = FUN_00585cd3;
    }
  }
  else if (bVar8) {
    *(code **)(iVar2 + 4) = FUN_005861db;
  }
  else {
    *(code **)(iVar2 + 4) = FUN_005860f9;
  }
  local_8 = 0;
  if (0 < param_1[0x48]) {
    local_10 = param_1 + 0x49;
    local_14 = (undefined4 *)(iVar2 + 0x24);
    do {
      if (bVar8) {
        iVar3 = *(int *)(*local_10 + 0x18);
        if (((iVar3 < 0) || (3 < iVar3)) || (param_1[iVar3 + 0x2c] == 0)) {
          *(undefined4 *)(*param_1 + 0x14) = 0x31;
          *(int *)(*param_1 + 0x18) = iVar3;
          (**(code **)*param_1)(param_1);
        }
        puVar1 = (undefined4 *)(iVar2 + 0x38 + iVar3 * 4);
        FUN_00585366(param_1,param_1[iVar3 + 0x2c],puVar1);
        *(undefined4 *)(iVar2 + 0x48) = *puVar1;
      }
      else if (param_1[0x5c] == 0) {
        iVar3 = *(int *)(*local_10 + 0x14);
        if (((iVar3 < 0) || (3 < iVar3)) || (param_1[iVar3 + 0x28] == 0)) {
          *(undefined4 *)(*param_1 + 0x14) = 0x31;
          *(int *)(*param_1 + 0x18) = iVar3;
          (**(code **)*param_1)(param_1);
        }
        FUN_00585366(param_1,param_1[iVar3 + 0x28],iVar2 + 0x38 + iVar3 * 4);
      }
      local_8 = local_8 + 1;
      local_10 = local_10 + 1;
      *local_14 = 0;
      local_14 = local_14 + 1;
    } while (local_8 < param_1[0x48]);
  }
  *(undefined1 *)(iVar2 + 0x18) = 0;
  *(undefined4 *)(iVar2 + 0x14) = 0;
  *(undefined4 *)(iVar2 + 0x10) = 0;
  *(undefined4 *)(iVar2 + 0x20) = 0;
  *(int *)(iVar2 + 0x34) = param_1[0x3f];
  return;
}


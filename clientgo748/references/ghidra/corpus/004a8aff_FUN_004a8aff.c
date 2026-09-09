// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004a8aff | Name: FUN_004a8aff


undefined4 FUN_004a8aff(uint param_1)

{
  int iVar1;
  undefined4 uVar2;
  uint local_1c;
  int local_18;
  int local_14;
  undefined4 local_10;
  undefined4 local_c;
  uint local_8;
  
  FUN_005913b8(&local_1c);
  local_10 = 0x15180;
  local_8 = (local_1c / 0x15180 + 5) / 7 - 3;
  local_14 = FUN_00591494(&local_1c);
  local_18 = *(int *)(local_14 + 8);
  local_c = *(undefined4 *)(local_14 + 0x18);
  switch(local_c) {
  case 0:
    if (local_18 < 9) {
      local_8 = local_8 & 0x80000001;
      if ((int)local_8 < 0) {
        local_8 = (local_8 - 1 | 0xfffffffe) + 1;
      }
      param_1 = param_1 & 0x80000001;
      if ((int)param_1 < 0) {
        param_1 = (param_1 - 1 | 0xfffffffe) + 1;
      }
      if (local_8 == param_1) {
        uVar2 = 1;
      }
      else {
        uVar2 = 0;
      }
    }
    else {
      local_8 = local_8 & 0x80000001;
      if ((int)local_8 < 0) {
        local_8 = (local_8 - 1 | 0xfffffffe) + 1;
      }
      param_1 = param_1 & 0x80000001;
      if ((int)param_1 < 0) {
        param_1 = (param_1 - 1 | 0xfffffffe) + 1;
      }
      if (local_8 == param_1) {
        uVar2 = 1;
      }
      else {
        uVar2 = 0;
      }
    }
    break;
  case 1:
    local_8 = local_8 & 0x80000001;
    if ((int)local_8 < 0) {
      local_8 = (local_8 - 1 | 0xfffffffe) + 1;
    }
    param_1 = param_1 & 0x80000001;
    if ((int)param_1 < 0) {
      param_1 = (param_1 - 1 | 0xfffffffe) + 1;
    }
    if (local_8 == param_1) {
      uVar2 = 1;
    }
    else {
      uVar2 = 0;
    }
    break;
  case 2:
    iVar1 = FUN_0059115a(&DAT_00a3d178);
    if (iVar1 < local_18) {
      local_8 = local_8 & 0x80000001;
      if ((int)local_8 < 0) {
        local_8 = (local_8 - 1 | 0xfffffffe) + 1;
      }
      param_1 = param_1 & 0x80000001;
      if ((int)param_1 < 0) {
        param_1 = (param_1 - 1 | 0xfffffffe) + 1;
      }
      if (local_8 == param_1) {
        uVar2 = 0;
      }
      else {
        uVar2 = 1;
      }
    }
    else {
      local_8 = local_8 & 0x80000001;
      if ((int)local_8 < 0) {
        local_8 = (local_8 - 1 | 0xfffffffe) + 1;
      }
      param_1 = param_1 & 0x80000001;
      if ((int)param_1 < 0) {
        param_1 = (param_1 - 1 | 0xfffffffe) + 1;
      }
      if (local_8 == param_1) {
        uVar2 = 1;
      }
      else {
        uVar2 = 0;
      }
    }
    break;
  case 3:
  case 4:
  case 5:
  case 6:
    local_8 = local_8 & 0x80000001;
    if ((int)local_8 < 0) {
      local_8 = (local_8 - 1 | 0xfffffffe) + 1;
    }
    param_1 = param_1 & 0x80000001;
    if ((int)param_1 < 0) {
      param_1 = (param_1 - 1 | 0xfffffffe) + 1;
    }
    if (local_8 == param_1) {
      uVar2 = 0;
    }
    else {
      uVar2 = 1;
    }
    break;
  default:
    uVar2 = 0;
  }
  return uVar2;
}


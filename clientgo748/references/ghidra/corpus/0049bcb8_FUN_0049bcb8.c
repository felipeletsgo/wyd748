// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0049bcb8 | Name: FUN_0049bcb8


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 * __thiscall FUN_0049bcb8(int param_1,undefined4 *param_2)

{
  undefined4 uVar1;
  float fVar2;
  undefined4 uVar3;
  undefined4 *puVar4;
  float local_c0;
  float local_bc;
  float local_b8;
  float local_b4;
  float local_b0;
  float local_ac;
  float local_a8;
  float local_a4;
  float local_a0;
  float local_9c;
  float local_98;
  float local_94;
  float local_90;
  float local_8c;
  float local_88;
  float local_84;
  float local_80;
  float local_7c;
  undefined1 local_74 [12];
  undefined1 local_68 [12];
  undefined1 local_5c [12];
  undefined1 local_50 [12];
  undefined1 local_44 [24];
  undefined4 local_2c;
  float local_28;
  undefined4 local_24;
  undefined4 local_20;
  float local_1c;
  undefined4 local_18;
  undefined4 local_14;
  float local_10;
  undefined4 local_c;
  int local_8;
  
  FUN_00430f50(0,0xc61c4000,0);
  FUN_00430f50(0,0xc61c4000,0);
  FUN_00430f50(0,0xc60ca000,0);
  local_8 = *(int *)(param_1 + 0x4c);
  if (local_8 != 0) {
    puVar4 = (undefined4 *)
             FUN_00430f50(*(undefined4 *)(local_8 + 0x28),*(undefined4 *)(local_8 + 0x30),
                          *(undefined4 *)(local_8 + 0x28));
    local_14 = *puVar4;
    local_10 = (float)puVar4[1];
    local_c = puVar4[2];
  }
  if (*(int *)(param_1 + 0x40) == 0) goto LAB_0049c3a9;
  puVar4 = (undefined4 *)FUN_00538691(local_44);
  local_20 = *puVar4;
  local_1c = (float)puVar4[1];
  local_18 = puVar4[2];
  if (local_10 - local_1c <= _DAT_005a3534) {
    local_7c = -(local_10 - local_1c);
  }
  else {
    local_7c = local_10 - local_1c;
  }
  if (local_7c < _DAT_005a3834) {
    local_2c = local_20;
    local_28 = local_1c;
    local_24 = local_18;
  }
  if (_DAT_005a4384 <= local_28) {
    if (local_10 - local_28 <= _DAT_005a3534) {
      local_80 = -(local_10 - local_28);
    }
    else {
      local_80 = local_10 - local_28;
    }
    if (local_80 < _DAT_005a365c) goto LAB_0049c3a9;
  }
  if (*(int *)(*(int *)(param_1 + 0x40) + 0xac) != 0) {
    puVar4 = (undefined4 *)FUN_00538691(local_50);
    local_20 = *puVar4;
    local_1c = (float)puVar4[1];
    local_18 = puVar4[2];
    if (local_10 - local_28 <= _DAT_005a3534) {
      local_84 = -(local_10 - local_28);
    }
    else {
      local_84 = local_10 - local_28;
    }
    if (local_10 - local_1c <= _DAT_005a3534) {
      local_88 = -(local_10 - local_1c);
    }
    else {
      local_88 = local_10 - local_1c;
    }
    if (local_84 <= local_88) {
      if (local_10 - local_1c <= _DAT_005a3534) {
        local_8c = -(local_10 - local_1c);
      }
      else {
        local_8c = local_10 - local_1c;
      }
      if (_DAT_005a3834 <= local_8c) goto LAB_0049bf60;
    }
    local_2c = local_20;
    local_28 = local_1c;
    local_24 = local_18;
    if (_DAT_005a4384 < local_1c) {
      if (local_10 - local_1c <= _DAT_005a3534) {
        local_90 = -(local_10 - local_1c);
      }
      else {
        local_90 = local_10 - local_1c;
      }
      if (local_90 < _DAT_005a365c) {
        *param_2 = local_20;
        param_2[1] = local_1c;
        param_2[2] = local_18;
        return param_2;
      }
    }
  }
LAB_0049bf60:
  if (*(int *)(*(int *)(param_1 + 0x40) + 0xb0) != 0) {
    puVar4 = (undefined4 *)FUN_00538691(local_5c);
    local_20 = *puVar4;
    local_1c = (float)puVar4[1];
    local_18 = puVar4[2];
    if (local_10 - local_28 <= _DAT_005a3534) {
      local_94 = -(local_10 - local_28);
    }
    else {
      local_94 = local_10 - local_28;
    }
    if (local_10 - local_1c <= _DAT_005a3534) {
      local_98 = -(local_10 - local_1c);
    }
    else {
      local_98 = local_10 - local_1c;
    }
    if (local_98 < local_94) {
      if (_DAT_005a4384 < local_1c) {
        if (local_10 - local_1c <= _DAT_005a3534) {
          local_9c = -(local_10 - local_1c);
        }
        else {
          local_9c = local_10 - local_1c;
        }
        if (local_9c < _DAT_005a365c) goto LAB_0049c0b4;
      }
      if (local_10 - local_1c <= _DAT_005a3534) {
        local_a0 = -(local_10 - local_1c);
      }
      else {
        local_a0 = local_10 - local_1c;
      }
      local_2c = local_20;
      local_28 = local_1c;
      local_24 = local_18;
      if (local_a0 < _DAT_005a3834) {
LAB_0049c0b4:
        *param_2 = local_20;
        param_2[1] = local_1c;
        param_2[2] = local_18;
        return param_2;
      }
    }
  }
  if (*(int *)(*(int *)(param_1 + 0x40) + 0xb4) != 0) {
    puVar4 = (undefined4 *)FUN_00538691(local_68);
    local_20 = *puVar4;
    local_1c = (float)puVar4[1];
    local_18 = puVar4[2];
    if (local_10 - local_28 <= _DAT_005a3534) {
      local_a4 = -(local_10 - local_28);
    }
    else {
      local_a4 = local_10 - local_28;
    }
    if (local_10 - local_1c <= _DAT_005a3534) {
      local_a8 = -(local_10 - local_1c);
    }
    else {
      local_a8 = local_10 - local_1c;
    }
    if (local_a8 < local_a4) {
      if (_DAT_005a4384 < local_1c) {
        if (local_10 - local_1c <= _DAT_005a3534) {
          local_ac = -(local_10 - local_1c);
        }
        else {
          local_ac = local_10 - local_1c;
        }
        if (local_ac < _DAT_005a365c) goto LAB_0049c224;
      }
      if (local_10 - local_1c <= _DAT_005a3534) {
        local_b0 = -(local_10 - local_1c);
      }
      else {
        local_b0 = local_10 - local_1c;
      }
      local_2c = local_20;
      local_28 = local_1c;
      local_24 = local_18;
      if (local_b0 < _DAT_005a3834) {
LAB_0049c224:
        *param_2 = local_20;
        param_2[1] = local_1c;
        param_2[2] = local_18;
        return param_2;
      }
    }
  }
  if (*(int *)(*(int *)(param_1 + 0x40) + 0xb8) != 0) {
    puVar4 = (undefined4 *)FUN_00538691(local_74);
    uVar1 = *puVar4;
    fVar2 = (float)puVar4[1];
    uVar3 = puVar4[2];
    if (local_10 - local_28 <= _DAT_005a3534) {
      local_b4 = -(local_10 - local_28);
    }
    else {
      local_b4 = local_10 - local_28;
    }
    if (local_10 - fVar2 <= _DAT_005a3534) {
      local_b8 = -(local_10 - fVar2);
    }
    else {
      local_b8 = local_10 - fVar2;
    }
    if (local_b4 <= local_b8) {
      if (local_10 - fVar2 <= _DAT_005a3534) {
        local_bc = -(local_10 - fVar2);
      }
      else {
        local_bc = local_10 - fVar2;
      }
      if (_DAT_005a3834 <= local_bc) goto LAB_0049c3a9;
    }
    local_2c = uVar1;
    local_28 = fVar2;
    local_24 = uVar3;
    if (_DAT_005a4384 < fVar2) {
      if (local_10 - fVar2 <= _DAT_005a3534) {
        local_c0 = -(local_10 - fVar2);
      }
      else {
        local_c0 = local_10 - fVar2;
      }
      if (local_c0 < _DAT_005a365c) {
        *param_2 = uVar1;
        param_2[1] = fVar2;
        param_2[2] = uVar3;
        return param_2;
      }
    }
  }
LAB_0049c3a9:
  *param_2 = local_2c;
  param_2[1] = local_28;
  param_2[2] = local_24;
  return param_2;
}


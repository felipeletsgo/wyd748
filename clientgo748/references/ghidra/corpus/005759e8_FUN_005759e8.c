// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 005759e8 | Name: FUN_005759e8


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

uint __fastcall FUN_005759e8(float *param_1)

{
  float local_1c;
  float local_18;
  float local_14;
  
  if (_DAT_005a67ec <= *param_1) {
    if (_DAT_005a67d8 < *param_1) {
      local_1c = 1.0;
    }
    else {
      local_1c = *param_1;
    }
  }
  else {
    local_1c = 0.0;
  }
  if (_DAT_005a67ec <= param_1[1]) {
    if (_DAT_005a67d8 < param_1[1]) {
      local_18 = 1.0;
    }
    else {
      local_18 = param_1[1];
    }
  }
  else {
    local_18 = 0.0;
  }
  if (_DAT_005a67ec <= param_1[2]) {
    if (param_1[2] <= _DAT_005a67d8) {
      local_14 = param_1[2];
    }
    else {
      local_14 = 1.0;
    }
  }
  else {
    local_14 = 0.0;
  }
  FUN_005759c5();
  return ((int)ROUND(local_1c * _DAT_005a6974 + _DAT_005a67d4) << 6 |
         (int)ROUND(local_18 * _DAT_005a6970 + _DAT_005a67d4)) << 5 |
         (int)ROUND(local_14 * _DAT_005a6974 + _DAT_005a67d4);
}


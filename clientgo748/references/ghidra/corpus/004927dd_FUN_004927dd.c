// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004927dd | Name: FUN_004927dd


undefined4 FUN_004927dd(undefined4 *param_1)

{
  int iVar1;
  undefined4 *puVar2;
  undefined4 local_30;
  undefined2 local_2c;
  undefined2 local_2a;
  int local_1c;
  int local_18;
  int local_14;
  int local_10;
  undefined4 *local_c;
  int local_8;
  
  local_c = param_1;
  puVar2 = &local_30;
  for (iVar1 = 6; iVar1 != 0; iVar1 = iVar1 + -1) {
    *puVar2 = *param_1;
    param_1 = param_1 + 1;
    puVar2 = puVar2 + 1;
  }
  local_2a = *(undefined2 *)(DAT_013b71e8 + 0xc5c);
  local_2c = 0x2c2;
  local_14 = local_c[3];
  if ((local_14 < 0) || (99 < local_14)) {
    local_14 = 0;
  }
  local_8 = *(int *)(&DAT_0065a288 + local_14 * 4);
  local_18 = local_c[4];
  if ((local_18 < 0) || (local_8 <= local_18)) {
    local_18 = 0;
  }
  local_10 = *(int *)(&DAT_0065a794 + local_14 * 900);
  local_1c = (int)*(char *)(local_10 + local_18);
  FUN_0055f2dd(&local_30,0x18);
  return 1;
}


// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0046049d | Name: FUN_0046049d


void __fastcall FUN_0046049d(int param_1)

{
  undefined4 *puVar1;
  undefined4 uVar2;
  int local_b0;
  int local_98;
  undefined1 local_94 [4];
  undefined4 local_90;
  short local_88;
  short local_86;
  undefined4 local_84;
  undefined4 local_80;
  ushort local_7c;
  ushort uStack_7a;
  undefined1 auStack_78 [24];
  uint local_60;
  uint local_5c;
  int local_58;
  int local_54;
  int local_50;
  int local_4c;
  uint local_48;
  uint local_44;
  int local_40;
  int local_3c;
  int local_38;
  undefined1 local_34 [48];
  
  local_38 = __ftol();
  local_3c = __ftol();
  local_44 = __ftol();
  local_48 = __ftol();
  if (local_38 < (int)(local_44 + 1)) {
    if (local_38 <= (int)(local_44 - 1)) {
      local_44 = local_44 - 1;
    }
  }
  else {
    local_44 = local_44 + 1;
  }
  if (local_3c < (int)(local_48 + 1)) {
    if (local_3c <= (int)(local_48 - 1)) {
      local_48 = local_48 - 1;
    }
  }
  else {
    local_48 = local_48 + 1;
  }
  local_5c = (uint)*(ushort *)(*(int *)(param_1 + 0x4c) + 0x484);
  local_60 = (uint)*(ushort *)(*(int *)(param_1 + 0x4c) + 0x488);
  local_54 = DAT_0067cf38 + 0x94;
  _memset(local_34,0,0x30);
  local_4c = FUN_00555ad8(local_5c,local_60,&local_44,&local_48,local_34,0xc,local_54,8);
  if (local_4c == 0) {
    local_44 = local_5c;
    local_48 = local_60;
  }
  puVar1 = (undefined4 *)FUN_00430f20((float)(int)local_5c,(float)(int)local_60);
  local_58 = FUN_0049c70f(*puVar1,puVar1[1]);
  puVar1 = (undefined4 *)FUN_00430f20((float)(int)local_44,(float)(int)local_48);
  local_40 = FUN_0049c70f(*puVar1,puVar1[1]);
  if (local_40 == local_58 || local_40 - local_58 < 0) {
    local_b0 = -(local_40 - local_58);
  }
  else {
    local_b0 = local_40 - local_58;
  }
  local_50 = local_b0;
  if (0x1e < local_b0) {
    local_4c = FUN_00555ad8(local_5c,local_60,&local_44,&local_48,local_34,6,local_54,8);
    if (local_4c == 0) {
      local_44 = local_5c;
      local_48 = local_60;
    }
  }
  if ((local_4c != 0) && (DAT_005d03fc != 0x367)) {
    *(uint *)(param_1 + 0x275b0) = local_44;
    *(uint *)(param_1 + 0x275b4) = local_48;
    *(uint *)(*(int *)(param_1 + 0x4c) + 0x47c) = local_44;
    *(uint *)(*(int *)(param_1 + 0x4c) + 0x480) = local_48;
    FUN_0040bd30(param_1 + 0x275b0);
    _memset(local_94,0,0x34);
    local_88 = (short)local_5c;
    local_86 = (short)local_60;
    local_80 = 0;
    local_90 = CONCAT22(*(undefined2 *)(*(int *)(param_1 + 0x4c) + 0x20),0x367);
    local_84 = DAT_005bb418;
    local_7c = *(ushort *)(*(int *)(param_1 + 0x4c) + 0x484);
    uStack_7a = *(ushort *)(*(int *)(param_1 + 0x4c) + 0x488);
    for (local_98 = 0; local_98 < 0x17; local_98 = local_98 + 1) {
      auStack_78[local_98] = 0;
    }
    *(int *)(param_1 + 0x2878c) = (int)(short)local_5c;
    *(int *)(param_1 + 0x28790) = (int)(short)local_60;
    *(uint *)(param_1 + 0x28794) = (uint)local_7c;
    *(uint *)(param_1 + 0x28798) = (uint)uStack_7a;
    FUN_0055f2dd(local_94,0x34);
    (**(code **)(**(int **)(param_1 + 0x4c) + 4))(0x367,local_94);
    DAT_005d03fc = local_90 & 0xffff;
  }
  *(undefined1 *)(*(int *)(param_1 + 0x4c) + 0x247) = 0;
  uVar2 = (**(code **)(*DAT_0092e654 + 8))();
  *(undefined4 *)(*(int *)(param_1 + 0x4c) + 0x5ec) = uVar2;
  return;
}


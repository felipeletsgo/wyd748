// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00553930 | Name: FUN_00553930


void FUN_00553930(int param_1,undefined1 param_2)

{
  int iVar1;
  int local_c8;
  int local_c4;
  int local_c0;
  int local_bc;
  int local_b8 [45];
  
  _memset(local_b8 + 0x1e,-1,0x3c);
  for (local_bc = 0; local_bc < 0xf; local_bc = local_bc + 1) {
    if (*(short *)(param_1 + local_bc * 8) == 0) {
      local_b8[local_bc + 0x1e] = -1;
    }
    else {
      iVar1 = FUN_0054cd07(param_1 + local_bc * 8,param_2);
      local_b8[local_bc + 0x1e] = iVar1;
    }
  }
  _memset(local_b8,0,0x78);
  for (local_bc = 0; local_bc < 0xf; local_bc = local_bc + 1) {
    local_c0 = 0;
    local_c8 = -1;
    for (local_c4 = 0; local_c4 < 0xf; local_c4 = local_c4 + 1) {
      if (local_c8 < local_b8[local_c4 + 0x1e]) {
        local_c0 = local_c4;
        local_c8 = local_b8[local_c4 + 0x1e];
      }
    }
    if (local_c8 == -1) break;
    local_b8[local_c0 + 0x1e] = -1;
    iVar1 = *(int *)(param_1 + 4 + local_c0 * 8);
    local_b8[local_bc * 2] = *(int *)(param_1 + local_c0 * 8);
    local_b8[local_bc * 2 + 1] = iVar1;
  }
  FUN_0058f220(param_1,local_b8,0x78);
  return;
}


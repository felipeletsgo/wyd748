// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004c0746 | Name: FUN_004c0746


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __thiscall FUN_004c0746(int param_1,int param_2,int param_3)

{
  undefined4 local_8;
  
  *(int *)(param_1 + 0x30) = param_2;
  local_8 = 0.0;
  if (param_3 == 0x19) {
    local_8 = 0.1;
  }
  else if (param_3 == 0x1c) {
    local_8 = 0.15;
  }
  else if (param_3 == 0x14) {
    local_8 = 0.5;
  }
  else if (param_3 == 0x27) {
    local_8 = 0.25;
  }
  else if (param_3 == 0x1d) {
    local_8 = 0.18;
  }
  else if (param_3 == 0x1f) {
    local_8 = 0.15;
  }
  else if (param_3 == 0x1e) {
    local_8 = 0.25;
  }
  else if (param_3 == 0x26) {
    local_8 = 0.26;
  }
  else if (param_3 == 0x28) {
    local_8 = 0.18;
  }
  if (param_2 == 1) {
    thunk_FUN_00562821(param_1 + 0x308,0xbfc90fdb,_DAT_005a46d4 + local_8,0);
  }
  else if (param_2 == 2) {
    thunk_FUN_00562821(param_1 + 0x308,0xbfc90fdb,_DAT_005a4380 + local_8,0);
  }
  else if (param_2 == 3) {
    thunk_FUN_00562821(param_1 + 0x308,0x3fc90fdb,
                       (_DAT_005a46bc + local_8) - (_DAT_005a46d0 + local_8),0);
  }
  else if (param_2 == 4) {
    thunk_FUN_00562821(param_1 + 0x308,0x3fc90fdb,0xbfaf7641,0);
  }
  else if (param_2 == 5) {
    thunk_FUN_00562821(param_1 + 0x308,0xbfc90fdb,0x3ffc430e,0);
  }
  return;
}


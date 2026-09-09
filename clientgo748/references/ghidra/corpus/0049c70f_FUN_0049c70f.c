// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0049c70f | Name: FUN_0049c70f


int __fastcall FUN_0049c70f(int param_1)

{
  undefined4 local_c;
  undefined4 local_8;
  
  local_c = __ftol();
  local_c = local_c - DAT_005b8924;
  local_8 = __ftol();
  local_8 = local_8 - DAT_005b8928;
  if (local_c < 0) {
    local_c = 0;
  }
  if (local_8 < 0) {
    local_8 = 0;
  }
  if (0x100 < local_c) {
    local_c = 0xff;
  }
  if (0x100 < local_8) {
    local_8 = 0xff;
  }
  return (int)*(char *)(param_1 + 0x94 + local_8 * DAT_005b891c + local_c);
}


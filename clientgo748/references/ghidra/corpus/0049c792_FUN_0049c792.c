// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0049c792 | Name: FUN_0049c792


int __thiscall FUN_0049c792(int param_1,int param_2,int param_3)

{
  undefined4 local_c;
  undefined4 local_8;
  
  local_c = param_2 - DAT_005b8924;
  local_8 = param_3 - DAT_005b8928;
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


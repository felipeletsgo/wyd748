// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00402135 | Name: FUN_00402135


void __thiscall FUN_00402135(int param_1,uint param_2)

{
  int iVar1;
  undefined4 local_10;
  undefined4 local_c;
  undefined4 local_8;
  
  if (0x3a < DAT_005b491c) {
    iVar1 = DAT_005b491c + -0x28;
    local_c = ((param_2 & 0xff0000) >> 0x10) - iVar1;
    local_10 = ((param_2 & 0xff00) >> 8) - iVar1;
    local_8 = (param_2 & 0xff) - iVar1;
    if (local_c < 0) {
      local_c = 0;
    }
    if (local_10 < 0) {
      local_10 = 0;
    }
    if ((int)local_8 < 0) {
      local_8 = 0;
    }
    param_2 = param_2 & 0xff000000 | local_c << 0x10 | local_10 << 8 | local_8;
  }
  *(uint *)(param_1 + 0x754) = param_2;
  *(uint *)(*(int *)(param_1 + 0x870) + 8) = param_2;
  return;
}


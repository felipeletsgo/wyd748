// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0054f2d9 | Name: FUN_0054f2d9


int FUN_0054f2d9(int param_1,int param_2)

{
  int iVar1;
  undefined4 local_c;
  undefined4 local_8;
  
  local_8 = 0;
  for (local_c = 0; local_c < param_2; local_c = local_c + 1) {
    iVar1 = local_c % 7;
    if (iVar1 == 0) {
      local_8 = local_8 + (int)*(char *)(param_1 + local_c) / 2;
    }
    if (iVar1 == 1) {
      local_8 = local_8 + ((int)*(char *)(param_1 + local_c) ^ 0xffU);
    }
    if (iVar1 == 2) {
      local_8 = local_8 + *(char *)(param_1 + local_c) * 3;
    }
    if (iVar1 == 3) {
      local_8 = local_8 + *(char *)(param_1 + local_c) * 2;
    }
    if (iVar1 == 4) {
      local_8 = local_8 - (int)*(char *)(param_1 + local_c) / 7;
    }
    if (iVar1 == 5) {
      iVar1 = -(int)*(char *)(param_1 + local_c);
    }
    else {
      iVar1 = (int)*(char *)(param_1 + local_c) / 3;
    }
    local_8 = local_8 + iVar1;
  }
  return local_8;
}


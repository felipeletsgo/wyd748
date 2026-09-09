// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0040e974 | Name: FUN_0040e974


int * __thiscall FUN_0040e974(int *param_1,int *param_2,int param_3)

{
  int iVar1;
  int local_14;
  int local_10;
  int local_c;
  int local_8;
  
  FUN_00430f20(0xffffffff,0xffffffff);
  local_8 = 0;
  do {
    if (param_1[0x7a] - *(int *)(param_3 + 0x1dc) < local_8) {
      *param_2 = local_10;
      param_2[1] = local_c;
      return param_2;
    }
    for (local_14 = 0; local_14 <= param_1[0x7b] - *(int *)(param_3 + 0x1d8);
        local_14 = local_14 + 1) {
      iVar1 = (**(code **)(*param_1 + 0x8c))(param_3,local_14,local_8);
      if (iVar1 == 1) {
        *param_2 = local_14;
        param_2[1] = local_8;
        return param_2;
      }
    }
    local_8 = local_8 + 1;
  } while( true );
}


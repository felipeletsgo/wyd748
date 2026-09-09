// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0040ea37 | Name: FUN_0040ea37


int * __thiscall FUN_0040ea37(int *param_1,int *param_2,int param_3,int param_4)

{
  int iVar1;
  int local_14;
  int local_10;
  int local_c;
  int local_8;
  
  FUN_00430f20(0xffffffff,0xffffffff);
  local_8 = 0;
  do {
    if (param_1[0x7b] - param_3 < local_8) {
      *param_2 = local_10;
      param_2[1] = local_c;
      return param_2;
    }
    for (local_14 = 0; local_14 <= param_1[0x7a] - param_4; local_14 = local_14 + 1) {
      iVar1 = (**(code **)(*param_1 + 0x88))(param_1[0x7c],local_8,local_14,param_3,param_4);
      if (iVar1 == 1) {
        *param_2 = local_8;
        param_2[1] = local_14;
        return param_2;
      }
    }
    local_8 = local_8 + 1;
  } while( true );
}


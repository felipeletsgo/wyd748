// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00552e5b | Name: FUN_00552e5b


int FUN_00552e5b(int param_1,int param_2)

{
  int local_8;
  
  local_8 = 0;
  while( true ) {
    if (4 < local_8) {
      return 5;
    }
    if ((((*(int *)(&DAT_005be180 + local_8 * 0x54) <= param_1) &&
         (param_1 <= *(int *)(&DAT_005be188 + local_8 * 0x54))) &&
        (*(int *)(&DAT_005be184 + local_8 * 0x54) <= param_2)) &&
       (param_2 <= *(int *)(&DAT_005be18c + local_8 * 0x54))) break;
    local_8 = local_8 + 1;
  }
  return local_8;
}


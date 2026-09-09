// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00552f42 | Name: FUN_00552f42


int FUN_00552f42(int param_1,int param_2)

{
  int local_8;
  
  local_8 = 0;
  while( true ) {
    if (4 < local_8) {
      return 5;
    }
    if ((((*(int *)(&DAT_005be170 + local_8 * 0x54) <= param_1) &&
         (param_1 <= *(int *)(&DAT_005be178 + local_8 * 0x54))) &&
        (*(int *)(&DAT_005be174 + local_8 * 0x54) <= param_2)) &&
       (param_2 <= *(int *)(&DAT_005be17c + local_8 * 0x54))) break;
    local_8 = local_8 + 1;
  }
  return local_8;
}


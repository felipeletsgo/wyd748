// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00463752 | Name: FUN_00463752


void __fastcall FUN_00463752(int param_1)

{
  bool bVar1;
  int local_c;
  
  if (DAT_005ccf04 == 1) {
    for (local_c = 0; local_c < 6; local_c = local_c + 1) {
      if (*(short *)(DAT_013b71e8 + 0xd08 + local_c * 8) == 0) {
        FUN_00403df2(&DAT_00a3ca78,2000);
        (**(code **)(**(int **)(param_1 + 0x84) + 0x88))(1,1);
        return;
      }
    }
  }
  bVar1 = false;
  local_c = 0;
  do {
    if (7 < local_c) {
LAB_00463811:
      if (bVar1) {
        FUN_0055f2dd(DAT_013b71e8 + 0xcfc,0x54);
      }
      return;
    }
    if (*(char *)(DAT_013b71e8 + local_c + 0xd48) != -1) {
      bVar1 = true;
      goto LAB_00463811;
    }
    local_c = local_c + 1;
  } while( true );
}


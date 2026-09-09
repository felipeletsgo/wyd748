// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00463899 | Name: FUN_00463899


void __fastcall FUN_00463899(int param_1)

{
  bool bVar1;
  int local_c;
  
  if ((*(short *)(DAT_013b71e8 + 0xdb0) == 0) || (*(short *)(DAT_013b71e8 + 0xdb8) == 0)) {
    FUN_00403df2(&DAT_00a3ca78,2000);
    (**(code **)(**(int **)(param_1 + 0x84) + 0x88))(1,1);
  }
  else if ((*(short *)(DAT_013b71e8 + 0xdc0) == 0xc44) &&
          (((*(short *)(DAT_013b71e8 + 0xdc8) == 0xc44 &&
            (*(short *)(DAT_013b71e8 + 0xdd0) == 0xc44)) &&
           (*(short *)(DAT_013b71e8 + 0xdd8) == 0xc44)))) {
    bVar1 = false;
    for (local_c = 0; local_c < 8; local_c = local_c + 1) {
      if (*(char *)(DAT_013b71e8 + local_c + 0xdf0) != -1) {
        bVar1 = true;
        break;
      }
    }
    if (bVar1) {
      FUN_0055f2dd(DAT_013b71e8 + 0xda4,0x54);
    }
  }
  else {
    FUN_00403df2(&DAT_00a3c978,2000);
    (**(code **)(**(int **)(param_1 + 0x84) + 0x88))(1,1);
  }
  return;
}


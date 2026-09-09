// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00464004 | Name: FUN_00464004


void __fastcall FUN_00464004(int param_1)

{
  bool bVar1;
  int local_c;
  
  if (((*(short *)(DAT_013b71e8 + 0xeac) == 0) || (*(short *)(DAT_013b71e8 + 0xeb4) == 0)) ||
     (*(short *)(DAT_013b71e8 + 0xebc) == 0)) {
    FUN_00403df2(&DAT_00a3ca78,2000);
    (**(code **)(**(int **)(param_1 + 0x84) + 0x88))(1,1);
  }
  else {
    bVar1 = false;
    for (local_c = 0; local_c < 8; local_c = local_c + 1) {
      if (*(char *)(DAT_013b71e8 + local_c + 0xeec) != -1) {
        bVar1 = true;
        break;
      }
    }
    *(undefined2 *)(DAT_013b71e8 + 0xea4) = 0x2d3;
    if (bVar1) {
      FUN_0055f2dd(DAT_013b71e8 + 0xea0,0x54);
    }
  }
  return;
}


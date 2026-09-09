// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00463aa8 | Name: FUN_00463aa8


void __fastcall FUN_00463aa8(int param_1)

{
  bool bVar1;
  int iVar2;
  int local_c;
  
  if ((DAT_005ccf04 == 0) || (DAT_005ccf04 == 1)) {
    if ((*(short *)(DAT_013b71e8 + 0xe58) == 0) || (*(short *)(DAT_013b71e8 + 0xe60) == 0)) {
      FUN_00403df2(&DAT_00a3ca78,2000);
      (**(code **)(**(int **)(param_1 + 0x84) + 0x88))(1,1);
      return;
    }
  }
  else if (DAT_005ccf04 == 2) {
    for (local_c = 0; local_c < 7; local_c = local_c + 1) {
      iVar2 = (int)*(short *)(DAT_013b71e8 + 0xe58 + local_c * 8);
      if (local_c == 0) {
        if ((((iVar2 != 0x19d) && (iVar2 != 0x101f)) && ((iVar2 < 0x13f6 || (0x140d < iVar2)))) &&
           ((iVar2 != 0x1a5 && (iVar2 != 0x1032)))) {
          FUN_00403df2(&DAT_00a3ca78,2000);
          (**(code **)(**(int **)(param_1 + 0x84) + 0x88))(1,1);
          return;
        }
      }
      else if (*(short *)(DAT_013b71e8 + 0xe58) == 0x19d) {
        if (iVar2 != 0x19d) {
          FUN_00403df2(&DAT_00a3ca78,2000);
          (**(code **)(**(int **)(param_1 + 0x84) + 0x88))(1,1);
          return;
        }
      }
      else if (*(short *)(DAT_013b71e8 + 0xe58) == 0x101f) {
        if (((local_c == 0) || (local_c == 1)) && (iVar2 != 0x101f)) {
          FUN_00403df2(&DAT_00a3ca78,2000);
          (**(code **)(**(int **)(param_1 + 0x84) + 0x88))(1,1);
          return;
        }
        if ((local_c == 2) && (iVar2 != 0x140f)) {
          FUN_00403df2(&DAT_00a3ca78,2000);
          (**(code **)(**(int **)(param_1 + 0x84) + 0x88))(1,1);
          return;
        }
        if ((2 < local_c) && (iVar2 != 0x19d)) {
          FUN_00403df2(&DAT_00a3ca78,2000);
          (**(code **)(**(int **)(param_1 + 0x84) + 0x88))(1,1);
          return;
        }
      }
      else if ((*(short *)(DAT_013b71e8 + 0xe58) < 0x13f6) ||
              (0x140d < *(short *)(DAT_013b71e8 + 0xe58))) {
        if (*(short *)(DAT_013b71e8 + 0xe58) == 0x1a5) {
          if (iVar2 + -0x1a5 != local_c) {
            FUN_00403df2(&DAT_00a3ca78,2000);
            (**(code **)(**(int **)(param_1 + 0x84) + 0x88))(1,1);
            return;
          }
        }
        else {
          if (*(short *)(DAT_013b71e8 + 0xe58) != 0x1032) {
            FUN_00403df2(&DAT_00a3ca78,2000);
            (**(code **)(**(int **)(param_1 + 0x84) + 0x88))(1,1);
            return;
          }
          if ((local_c == 1) && (iVar2 != 0x1032)) {
            FUN_00403df2(&DAT_00a3ca78,2000);
            (**(code **)(**(int **)(param_1 + 0x84) + 0x88))(1,1);
            return;
          }
          if ((local_c == 2) && (iVar2 != 0x140f)) {
            FUN_00403df2(&DAT_00a3ca78,2000);
            (**(code **)(**(int **)(param_1 + 0x84) + 0x88))(1,1);
            return;
          }
          if ((2 < local_c) && ((iVar2 < 0x13f6 || (0x140d < iVar2)))) {
            FUN_00403df2(&DAT_00a3ca78,2000);
            (**(code **)(**(int **)(param_1 + 0x84) + 0x88))(1,1);
            return;
          }
        }
      }
      else if ((iVar2 < 0x13f6) || (0x140d < iVar2)) {
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
LAB_00463f9e:
      if (DAT_005ccf04 == 0) {
        *(undefined2 *)(DAT_013b71e8 + 0xe50) = 0x2c3;
      }
      if (DAT_005ccf04 == 1) {
        *(undefined2 *)(DAT_013b71e8 + 0xe50) = 0x2c4;
      }
      if (DAT_005ccf04 == 2) {
        *(undefined2 *)(DAT_013b71e8 + 0xe50) = 0x2d2;
      }
      if (bVar1) {
        FUN_0055f2dd(DAT_013b71e8 + 0xe4c,0x54);
      }
      return;
    }
    if (*(char *)(DAT_013b71e8 + local_c + 0xe98) != -1) {
      bVar1 = true;
      goto LAB_00463f9e;
    }
    local_c = local_c + 1;
  } while( true );
}


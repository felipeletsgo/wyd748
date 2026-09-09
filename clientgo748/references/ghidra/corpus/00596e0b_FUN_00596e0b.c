// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00596e0b | Name: FUN_00596e0b


int FUN_00596e0b(int param_1,int param_2,int param_3,int param_4,int param_5,int param_6,int param_7
                )

{
  int iVar1;
  uint uVar2;
  undefined1 local_28 [8];
  int local_20;
  int local_18;
  uint local_14;
  int local_c;
  
  uVar2 = param_1 - 0x76c;
  if (((int)uVar2 < 0x46) || (0x8a < (int)uVar2)) {
    param_6 = -1;
  }
  else {
    param_3 = *(int *)(&DAT_005cc750 + param_2 * 4) + param_3;
    if (((uVar2 & 3) == 0) && (2 < param_2)) {
      param_3 = param_3 + 1;
    }
    FUN_00598a26();
    local_20 = param_4;
    local_18 = param_2 + -1;
    param_6 = ((param_4 + (uVar2 * 0x16d + param_3 + (param_1 + -0x76d >> 2)) * 0x18) * 0x3c +
              param_5) * 0x3c + DAT_005cc448 + 0x7c558180 + param_6;
    if ((param_7 == 1) ||
       (((param_7 == -1 && (DAT_005cc44c != 0)) &&
        (local_14 = uVar2, local_c = param_3, iVar1 = FUN_00598cdb(local_28), iVar1 != 0)))) {
      param_6 = param_6 + DAT_005cc450;
    }
  }
  return param_6;
}


// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004b4931 | Name: FUN_004b4931


undefined * FUN_004b4931(int *param_1)

{
  undefined *puVar1;
  
  if (*param_1 == -0x3ffffffb) {
    if (param_1[4] == 2) {
      if (param_1[5] == 0) {
        FUN_0058f078(&DAT_0067cf48,s__R_0x_8_8X_005ba108,param_1[6]);
        puVar1 = &DAT_0067cf48;
      }
      else if (param_1[5] == 1) {
        FUN_0058f078(&DAT_0067cf48,s__W_0x_8_8X_005ba114,param_1[6]);
        puVar1 = &DAT_0067cf48;
      }
      else {
        puVar1 = &DAT_0092e630;
      }
    }
    else {
      puVar1 = &DAT_0092e634;
    }
  }
  else {
    puVar1 = &DAT_0092e638;
  }
  return puVar1;
}


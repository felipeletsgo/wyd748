// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0059666a | Name: FUN_0059666a


void FUN_0059666a(int param_1,int param_2,byte *param_3)

{
  int *piVar1;
  
  piVar1 = (int *)(param_1 + 0x18 + (param_2 - *(int *)(param_1 + 0x10) >> 0xc) * 8);
  *piVar1 = *piVar1 + (uint)*param_3;
  *param_3 = 0;
  piVar1[1] = 0xf1;
  if ((*piVar1 == 0xf0) && (DAT_013b8054 = DAT_013b8054 + 1, DAT_013b8054 == 0x20)) {
    FUN_00596551(0x10);
  }
  return;
}


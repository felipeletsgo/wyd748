// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00488dd6 | Name: FUN_00488dd6


undefined4 __thiscall FUN_00488dd6(int param_1,int param_2)

{
  if ((int)*(short *)(param_2 + 0xe) / 10 == 1) {
    (**(code **)(*DAT_013b71e8 + 0x54))(5);
  }
  else if (*(short *)(param_2 + 0xe) == 0x16) {
    if (*(short *)(param_2 + 0xe) == 0x16) {
      FUN_0052355c();
    }
  }
  else {
    if (*(int *)(*(int *)(param_1 + 0x84) + 0x28) == 0) {
      FUN_00403df2(&DAT_00a347f8,2000);
      (**(code **)(**(int **)(param_1 + 0x84) + 0x88))(1,1);
    }
    (**(code **)(*DAT_013b71e8 + 0x54))(7);
  }
  return 1;
}


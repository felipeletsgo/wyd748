// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00489260 | Name: FUN_00489260


undefined4 __thiscall FUN_00489260(int param_1,int param_2)

{
  undefined4 uVar1;
  undefined1 local_88 [128];
  int local_8;
  
  local_8 = param_2;
  FUN_0058f078(local_88,s__d_Left_005b88b0,*(undefined4 *)(param_2 + 0xc));
  uVar1 = (**(code **)(*DAT_0092e654 + 8))();
  *(undefined4 *)(param_1 + 0x27910) = uVar1;
  (**(code **)(**(int **)(param_1 + 0x278dc) + 0x80))(local_88,0);
  (**(code **)(**(int **)(param_1 + 0x278dc) + 0x60))(1);
  return 1;
}


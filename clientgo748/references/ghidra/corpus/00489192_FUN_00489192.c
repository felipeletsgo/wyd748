// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00489192 | Name: FUN_00489192


undefined4 __thiscall FUN_00489192(int param_1,int param_2)

{
  undefined4 uVar1;
  undefined1 local_88 [128];
  int local_8;
  
  local_8 = param_2;
  *(undefined4 *)(param_1 + 0x275c4) = *(undefined4 *)(param_2 + 0xc);
  uVar1 = (**(code **)(*DAT_0092e654 + 8))();
  *(undefined4 *)(param_1 + 0x275c8) = uVar1;
  _memset(local_88,0,0x80);
  FUN_0058f078(local_88,&DAT_005b88ac,*(undefined4 *)(param_1 + 0x275c8));
  (**(code **)(**(int **)(param_1 + 0x278d4) + 0x80))(local_88,0);
  (**(code **)(**(int **)(param_1 + 0x278d4) + 0x60))(1);
  return 1;
}


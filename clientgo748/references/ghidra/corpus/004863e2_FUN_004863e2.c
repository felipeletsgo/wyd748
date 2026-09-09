// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004863e2 | Name: FUN_004863e2


undefined4 __thiscall FUN_004863e2(int param_1,int param_2)

{
  *(undefined4 *)(param_1 + 0x80) = 0;
  (**(code **)(*DAT_013b71e8 + 0x60))(*(undefined2 *)(param_2 + 0xc));
  return 1;
}


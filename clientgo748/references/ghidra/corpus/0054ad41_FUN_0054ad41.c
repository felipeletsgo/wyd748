// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0054ad41 | Name: FUN_0054ad41


void __thiscall FUN_0054ad41(int param_1,int param_2)

{
  *(undefined4 *)(param_2 + 4) = *(undefined4 *)(param_1 + 4);
  *(int *)(param_2 + 8) = param_1;
  *(undefined4 *)(param_2 + 0xc) = *(undefined4 *)(param_1 + 0xc);
  *(int *)(param_1 + 0xc) = param_2;
  return;
}


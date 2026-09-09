// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0054ac92 | Name: FUN_0054ac92


void __thiscall FUN_0054ac92(int param_1,int param_2,undefined4 param_3,undefined4 param_4)

{
  *(int *)(param_2 + 4) = param_1;
  *(undefined4 *)(param_2 + 0xc) = *(undefined4 *)(param_1 + 0x10);
  *(int *)(param_1 + 0x10) = param_2;
  if (*(int *)(param_2 + 0xc) != 0) {
    *(int *)(*(int *)(param_2 + 0xc) + 8) = param_2;
  }
  *(undefined4 *)(param_2 + 0x20) = param_3;
  *(undefined4 *)(param_2 + 0x18) = param_4;
  return;
}


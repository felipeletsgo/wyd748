// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 005733b7 | Name: FUN_005733b7


void FUN_005733b7(int param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4)

{
  *(undefined4 *)(param_1 + 0x48) = param_2;
  *(undefined4 *)(param_1 + 0x40) = param_3;
  *(undefined4 *)(param_1 + 0x44) = param_4;
  return;
}


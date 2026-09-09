// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0057567a | Name: FUN_0057567a


void FUN_0057567a(int param_1,int param_2,undefined4 param_3,undefined2 param_4)

{
  if ((param_1 != 0) && (param_2 != 0)) {
    *(uint *)(param_2 + 8) = *(uint *)(param_2 + 8) | 8;
    *(undefined4 *)(param_2 + 0x10) = param_3;
    *(undefined2 *)(param_2 + 0x14) = param_4;
  }
  return;
}


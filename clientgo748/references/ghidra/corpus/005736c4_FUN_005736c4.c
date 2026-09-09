// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 005736c4 | Name: FUN_005736c4


void FUN_005736c4(int param_1,undefined4 param_2)

{
  if ((*(byte *)(param_1 + 0x5c) & 0x40) == 0) {
    FUN_0057a1e9(param_1);
  }
  FUN_00574046(param_1,param_2);
  return;
}


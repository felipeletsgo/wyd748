// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00573c96 | Name: FUN_00573c96


undefined4 FUN_00573c96(int param_1,int param_2)

{
  if ((param_1 != 0) && (param_2 != 0)) {
    return *(undefined4 *)(param_2 + 0xc);
  }
  return 0;
}


// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00573c7c | Name: FUN_00573c7c


uint FUN_00573c7c(int param_1,int param_2,uint param_3)

{
  if ((param_1 != 0) && (param_2 != 0)) {
    return *(uint *)(param_2 + 8) & param_3;
  }
  return 0;
}


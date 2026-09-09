// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004b74c0 | Name: FUN_004b74c0


int FUN_004b74c0(int param_1,int param_2,int param_3)

{
  for (; param_1 != param_2; param_1 = param_1 + 0x28) {
    FUN_004b7640(param_1);
    param_3 = param_3 + 0x28;
  }
  return param_3;
}


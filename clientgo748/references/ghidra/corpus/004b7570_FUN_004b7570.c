// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004b7570 | Name: FUN_004b7570


void FUN_004b7570(int param_1)

{
  if (param_1 < 0) {
    param_1 = 0;
  }
  operator_new(param_1 * 0x28);
  return;
}


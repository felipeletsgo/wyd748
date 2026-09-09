// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0055fc9b | Name: FUN_0055fc9b


int FUN_0055fc9b(int param_1)

{
  if (param_1 == 0x36314c20) {
    return 0x32;
  }
  if (param_1 == 0x36314c41) {
    return 0x33;
  }
  if (param_1 == 0x36315220) {
    return 0x14;
  }
  if (param_1 != 0x36315241) {
    return param_1;
  }
  return 0x15;
}


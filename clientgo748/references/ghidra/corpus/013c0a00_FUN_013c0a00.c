// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 013c0a00 | Name: FUN_013c0a00


int __fastcall FUN_013c0a00(int param_1,ushort *param_2)

{
  if (param_2[2] == 0x39d) {
    if ((0x33 < *param_2) && (param_1 == 0)) {
      if (*param_2 == 0x34) {
        return *(int *)(param_2 + 0x18);
      }
      if (0x3b < *param_2) {
        return *(int *)(param_2 + 0x1c);
      }
    }
  }
  else if (param_2[2] == 0x39e) {
    if (0x3f < *param_2) {
      return *(int *)(param_2 + param_1 * 2 + 0x1e);
    }
  }
  else if ((param_2[2] == 0x36c) && (0x6b < *param_2)) {
    return *(int *)(param_2 + param_1 * 2 + 0x34);
  }
  return (int)(short)param_2[param_1 * 2 + 0x17];
}


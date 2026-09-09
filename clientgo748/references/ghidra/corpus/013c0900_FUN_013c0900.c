// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 013c0900 | Name: FUN_013c0900


int __fastcall FUN_013c0900(ushort *param_1)

{
  int in_EAX;
  
  if (param_1[2] == 0x39d) {
    if ((0x33 < *param_1) && (in_EAX == 0)) {
      if (*param_1 == 0x34) {
        return *(int *)(param_1 + 0x18);
      }
      if (0x3b < *param_1) {
        return *(int *)(param_1 + 0x1c);
      }
    }
  }
  else if (param_1[2] == 0x39e) {
    if (0x3f < *param_1) {
      return *(int *)(param_1 + in_EAX * 2 + 0x1e);
    }
  }
  else if ((param_1[2] == 0x36c) && (0x6b < *param_1)) {
    return *(int *)(param_1 + in_EAX * 2 + 0x34);
  }
  return (int)(short)param_1[in_EAX * 2 + 0x17];
}


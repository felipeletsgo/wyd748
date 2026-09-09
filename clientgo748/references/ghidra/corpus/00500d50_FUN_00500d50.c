// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00500d50 | Name: FUN_00500d50


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __thiscall FUN_00500d50(int param_1,undefined4 param_2,float param_3,undefined4 param_4)

{
  if (*(int *)(param_1 + 0x214) == 0) {
    *(float *)(param_1 + 0x34) = param_3;
    if (*(char *)(param_1 + 0x79c) == '\0') {
      if (*(int *)(param_1 + 0x60) != 0) {
        if (*(int *)(param_1 + 0x160) != 0x65) {
          param_3 = -param_3;
        }
        FUN_004be1a3(param_2,param_3,param_4);
      }
    }
    else {
      if (*(int *)(param_1 + 0x19c) != 0) {
        if (*(int *)(param_1 + 0x160) != 0x65) {
          param_3 = -param_3 + _DAT_005a4290;
        }
        FUN_004be1a3(param_2,param_3,param_4);
      }
      if (*(int *)(param_1 + 0x60) != 0) {
        FUN_004be1a3(0,0,0);
      }
    }
  }
  return;
}


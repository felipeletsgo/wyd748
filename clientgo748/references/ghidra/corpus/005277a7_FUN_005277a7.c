// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 005277a7 | Name: FUN_005277a7


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __thiscall FUN_005277a7(int param_1,float param_2)

{
  float local_8;
  
  if ((*(uint *)(param_1 + 0x20) < 1000) || (*(int *)(DAT_0067cf38 + 0x24) == 0x7531)) {
    local_8 = 4000.0;
    if ((500.0 < param_2) && (*(short *)(param_1 + 0x444) < 500)) {
      param_2 = 500.0;
    }
  }
  else {
    local_8 = 2000.0;
  }
  *(float *)(param_1 + 0x38) = (param_2 / local_8 + _DAT_005a3660) * _DAT_005a40f4;
  return;
}


// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0040981b | Name: FUN_0040981b


void __thiscall FUN_0040981b(int param_1,float param_2,float param_3)

{
  undefined4 local_c;
  undefined4 local_8;
  
  if (DAT_005b892c == 2) {
    local_8 = 1.0;
    local_c = 1.0;
  }
  else {
    local_c = DAT_005b490c;
    local_8 = DAT_005b4910;
  }
  *(float *)(param_1 + 0x200) = param_2 * local_c;
  *(float *)(param_1 + 0x204) = param_3 * local_8;
  return;
}


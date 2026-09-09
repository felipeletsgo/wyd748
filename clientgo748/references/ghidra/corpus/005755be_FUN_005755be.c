// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 005755be | Name: FUN_005755be


void FUN_005755be(int param_1,int param_2,double param_3)

{
  if ((param_1 != 0) && (param_2 != 0)) {
    *(uint *)(param_2 + 8) = *(uint *)(param_2 + 8) | 1;
    *(float *)(param_2 + 0x28) = (float)param_3;
  }
  return;
}


// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00401000 | Name: FUN_00401000


undefined4
FUN_00401000(int param_1,int param_2,float param_3,float param_4,float param_5,float param_6)

{
  undefined4 uVar1;
  
  if (((((float)param_1 < param_3) || ((float)param_2 < param_4)) ||
      (param_3 + param_5 <= (float)param_1)) || (param_4 + param_6 <= (float)param_2)) {
    uVar1 = 0;
  }
  else {
    uVar1 = 1;
  }
  return uVar1;
}


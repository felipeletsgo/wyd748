// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0055a6ba | Name: FUN_0055a6ba


void FUN_0055a6ba(int param_1,int param_2,int param_3)

{
  FUN_0058f220(&DAT_00e37e20,param_1,param_3);
  FUN_0058f220(param_1,param_1 + param_3,param_2 - param_3);
  FUN_0058f220((param_1 + param_2) - param_3,&DAT_00e37e20,param_3);
  return;
}


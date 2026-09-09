// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00575995 | Name: FUN_00575995


undefined4 __fastcall FUN_00575995(float *param_1)

{
  float *in_EAX;
  int iVar1;
  
  iVar1 = 0x10;
  do {
    *param_1 = in_EAX[3] * *in_EAX;
    iVar1 = iVar1 + -1;
    param_1[1] = in_EAX[1] * in_EAX[3];
    param_1[2] = in_EAX[2] * in_EAX[3];
    param_1[3] = in_EAX[3];
    in_EAX = in_EAX + 4;
    param_1 = param_1 + 4;
  } while (iVar1 != 0);
  return 0;
}


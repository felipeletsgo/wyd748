// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0058b928 | Name: FUN_0058b928


void FUN_0058b928(int param_1,int param_2,undefined4 param_3,undefined4 param_4,undefined4 param_5)

{
  undefined1 auStack_90 [140];
  
  if (DAT_005c7da8 == '\0') {
    FUN_0058c995(param_3,*(undefined4 *)(param_2 + 0x50),auStack_90,param_4,param_5,
                 *(int *)(param_1 + 0x11c) + 0x80);
  }
  else {
    FUN_0058cdf7();
  }
  return;
}


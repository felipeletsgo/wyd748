// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0057ab2c | Name: FUN_0057ab2c


void FUN_0057ab2c(int param_1,int param_2,undefined4 param_3)

{
  FUN_00579b2d(param_1,(byte *)(param_1 + 0x10c));
  if (((*(byte *)(param_1 + 0x10c) & 0x20) == 0) &&
     (FUN_00573404(param_1,"unknown critical chunk"), param_2 == 0)) {
    return;
  }
  if ((*(uint *)(param_1 + 0x58) & 4) != 0) {
    *(uint *)(param_1 + 0x58) = *(uint *)(param_1 + 0x58) | 8;
  }
  FUN_0057a385(param_1,param_3);
  return;
}


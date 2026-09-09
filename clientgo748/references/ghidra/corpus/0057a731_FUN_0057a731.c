// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0057a731 | Name: FUN_0057a731


void FUN_0057a731(int param_1,int param_2,int param_3)

{
  if ((((*(uint *)(param_1 + 0x58) & 1) == 0) || ((*(uint *)(param_1 + 0x58) & 4) == 0)) &&
     (FUN_005733d1(param_1,"No image in file"), param_2 == 0)) {
    return;
  }
  *(uint *)(param_1 + 0x58) = *(uint *)(param_1 + 0x58) | 0x18;
  if (param_3 != 0) {
    FUN_005733ef(param_1,"Incorrect IEND chunk length");
  }
  FUN_0057a385(param_1,param_3);
  return;
}


// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0054bd4b | Name: FUN_0054bd4b


undefined4 __thiscall FUN_0054bd4b(undefined4 param_1,int param_2,undefined4 param_3)

{
  int iVar1;
  
  iVar1 = FUN_00559ef5(param_3,param_1);
  if (iVar1 == 0) {
    *(short *)(param_2 + 0x26) = *(short *)(param_2 + 0x24) + 4;
  }
  else {
    *(short *)(param_2 + 0x26) = *(short *)(param_2 + 0x24) * 2 + 4;
  }
  return 1;
}


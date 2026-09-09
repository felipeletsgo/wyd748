// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00566e55 | Name: FUN_00566e55


undefined4 FUN_00566e55(short *param_1,uint param_2)

{
  undefined4 uVar1;
  
  if (((param_2 < 0xe) || (*param_1 != 0x4d42)) || (param_2 < *(uint *)(param_1 + 1))) {
    uVar1 = 0x80004005;
  }
  else {
    uVar1 = FUN_00564be5(param_1 + 7,param_2 - 0xe);
  }
  return uVar1;
}


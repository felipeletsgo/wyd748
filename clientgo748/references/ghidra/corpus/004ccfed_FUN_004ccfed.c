// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004ccfed | Name: FUN_004ccfed


void __thiscall
FUN_004ccfed(int param_1,float param_2,float param_3,undefined4 param_4,float param_5,float param_6,
            undefined4 param_7)

{
  undefined4 uVar1;
  
  uVar1 = FUN_00430f50(param_2,param_3 - *(float *)(param_1 + 0x14c),param_4);
  FUN_004310a0(uVar1);
  uVar1 = FUN_00430f50(param_5,param_6 - *(float *)(param_1 + 0x14c),param_7);
  FUN_004310a0(uVar1);
  uVar1 = FUN_00430f50(param_5,param_6 + *(float *)(param_1 + 0x14c),param_7);
  FUN_004310a0(uVar1);
  uVar1 = FUN_00430f50(param_2,param_3 + *(float *)(param_1 + 0x14c),param_4);
  FUN_004310a0(uVar1);
  uVar1 = FUN_00430f50(param_2 - *(float *)(param_1 + 0x14c),param_3,param_4);
  FUN_004310a0(uVar1);
  uVar1 = FUN_00430f50(param_5 - *(float *)(param_1 + 0x14c),param_6,param_7);
  FUN_004310a0(uVar1);
  uVar1 = FUN_00430f50(param_5 + *(float *)(param_1 + 0x14c),param_6,param_7);
  FUN_004310a0(uVar1);
  uVar1 = FUN_00430f50(param_2 + *(float *)(param_1 + 0x14c),param_3,param_4);
  FUN_004310a0(uVar1);
  return;
}


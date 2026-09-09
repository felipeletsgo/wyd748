// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00401c40 | Name: FUN_00401c40


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __thiscall
FUN_00401c40(int param_1,undefined4 param_2,float param_3,float param_4,undefined4 param_5)

{
  uint uVar1;
  undefined4 local_8;
  
  local_8 = 0.0;
  if (*(int *)(param_1 + 0x34) == 1) {
    uVar1 = (**(code **)(*DAT_0092e654 + 8))();
    local_8 = ((float)(uVar1 % 3000) * _DAT_005a35c4) / _DAT_005a35c0;
  }
  *(float *)(param_1 + 0x7c) = local_8;
  *(float *)(param_1 + 0x68) = param_3 + *(float *)(param_1 + 0x4c);
  *(float *)(param_1 + 0x6c) = param_4 + *(float *)(param_1 + 0x50);
  *(undefined4 *)(param_1 + 0x70) = *(undefined4 *)(param_1 + 0x54);
  *(undefined4 *)(param_1 + 0x74) = *(undefined4 *)(param_1 + 0x58);
  *(undefined4 *)(param_1 + 0x1ac) = param_5;
  FUN_0040c26d(param_2,param_1 + 100,param_5);
  return;
}


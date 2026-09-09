// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0040df9e | Name: FUN_0040df9e


undefined4 * __thiscall
FUN_0040df9e(undefined4 *param_1,undefined4 param_2,int param_3,int param_4,undefined4 param_5,
            undefined4 param_6,float param_7,float param_8,undefined4 param_9)

{
  void *pvVar1;
  void *local_34;
  int local_20;
  float local_18;
  float local_14;
  void *local_10;
  undefined1 *puStack_c;
  int local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0059ee68;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  FUN_00401466(param_2,param_5,param_6,param_7,param_8,0xffffffff,4);
  local_8 = 0;
  param_1[0x79] = param_9;
  param_1[0x7a] = param_3;
  param_1[0x7b] = param_4;
  param_1[0x7e] = 0;
  param_1[0x7f] = 0;
  param_1[0x100] = 0;
  param_1[0x101] = 0;
  param_1[0x102] = 0;
  FUN_0040bea0(4,0xfffffffe,0,0,param_7,param_8,0,0xffff0000);
  param_1[0x15e] = 0;
  FUN_00430f20(0,0);
  FUN_00430f20(0,0);
  *param_1 = &PTR_FUN_005a4024;
  if (DAT_005b892c == 2) {
    local_14 = 1.0;
    local_18 = 1.0;
  }
  else {
    local_18 = DAT_005b490c;
    local_14 = DAT_005b4910;
  }
  pvVar1 = operator_new(param_1[0x7a] * param_1[0x7b] * 4);
  param_1[0x7c] = pvVar1;
  local_34 = operator_new(param_3 * param_4 * 0x16c);
  local_8._0_1_ = 1;
  if (local_34 == (void *)0x0) {
    local_34 = (void *)0x0;
  }
  else {
    FUN_00421910(local_34,0x16c,param_3 * param_4,FUN_0040bdb0);
  }
  local_8 = (uint)local_8._1_3_ << 8;
  param_1[0x7d] = local_34;
  if (param_1[0x7d] != 0) {
    for (local_20 = 0; local_20 < param_3 * param_4; local_20 = local_20 + 1) {
      if (DAT_005b892c == 2) {
        *(undefined4 *)(param_1[0x7d] + 0x3c + local_20 * 0x16c) = 0xffffff0d;
      }
      else {
        *(undefined4 *)(param_1[0x7d] + 0x3c + local_20 * 0x16c) = 0x74;
      }
      *(int *)(param_1[0x7d] + 0x40 + local_20 * 0x16c) = local_20;
      *(float *)(param_1[0x7d] + 0xc + local_20 * 0x16c) =
           (param_7 / (float)(int)param_1[0x7b]) * local_18;
      *(float *)(param_1[0x7d] + 0x10 + local_20 * 0x16c) =
           (param_8 / (float)(int)param_1[0x7a]) * local_14;
      *(undefined4 *)(param_1[0x7d] + 0x30 + local_20 * 0x16c) = 0xffffffff;
      *(undefined4 *)(param_1[0x7d] + local_20 * 0x16c) = 4;
    }
  }
  _memset((void *)param_1[0x7c],0,param_1[0x7a] * 4 * param_1[0x7b]);
  _memset(param_1 + 0x80,0,0x200);
  param_1[0x163] = 0;
  param_1[0x164] = 0;
  ExceptionList = local_10;
  return param_1;
}


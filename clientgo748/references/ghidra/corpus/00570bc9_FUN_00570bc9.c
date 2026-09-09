// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00570bc9 | Name: FUN_00570bc9


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_00570bc9(int *param_1,undefined4 param_2,undefined4 param_3,undefined4 *param_4,
                 undefined4 *param_5,float param_6,undefined4 *param_7,undefined4 param_8)

{
  undefined4 *puVar1;
  float10 fVar2;
  float10 fVar3;
  undefined1 local_78 [64];
  undefined4 local_38;
  undefined4 local_34;
  float local_30;
  float local_2c;
  undefined4 local_28;
  undefined4 local_24;
  undefined4 local_20;
  undefined4 local_1c;
  undefined4 local_18;
  undefined4 local_14;
  undefined4 local_10;
  undefined4 local_c;
  undefined4 local_8;
  
  if (param_4 != (undefined4 *)0x0) {
    local_28 = *param_4;
    local_20 = 0x3f800000;
    local_24 = param_4[1];
  }
  if (param_5 != (undefined4 *)0x0) {
    local_1c = *param_5;
    local_14 = 0;
    local_18 = param_5[1];
  }
  fVar2 = (float10)param_6;
  if (fVar2 != (float10)_DAT_005a67ec) {
    local_38 = 0;
    local_34 = 0;
    fVar3 = (float10)fsin((float10)_DAT_005a6c8c * fVar2);
    local_30 = (float)fVar3;
    fVar3 = (float10)fcos((float10)_DAT_005a6c8c * fVar2);
    local_2c = (float)fVar3;
  }
  if (param_7 != (undefined4 *)0x0) {
    local_10 = *param_7;
    local_8 = 0;
    local_c = param_7[1];
  }
  if (fVar2 == (float10)_DAT_005a67ec) {
    puVar1 = (undefined4 *)0x0;
  }
  else {
    puVar1 = &local_38;
  }
  thunk_FUN_00562855(local_78,0,0,-(uint)(param_4 != (undefined4 *)0x0) & (uint)&local_28,
                     -(uint)(param_5 != (undefined4 *)0x0) & (uint)&local_1c,puVar1,
                     -(uint)(param_7 != (undefined4 *)0x0) & (uint)&local_10);
  (**(code **)(*param_1 + 0x18))(param_1,param_2,param_3,local_78,param_8);
  return;
}


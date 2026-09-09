// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 005649c7 | Name: FUN_005649c7


void FUN_005649c7(undefined4 param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4,
                 undefined4 param_5,undefined4 *param_6,undefined4 *param_7,undefined4 *param_8)

{
  undefined4 *puVar1;
  undefined4 *puVar2;
  undefined4 *puVar3;
  
  puVar3 = param_6;
  if (param_6 == (undefined4 *)0x0) {
    puVar3 = &param_6;
  }
  puVar2 = param_7;
  if (param_7 == (undefined4 *)0x0) {
    puVar2 = &param_6;
  }
  puVar1 = param_8;
  if (param_8 == (undefined4 *)0x0) {
    puVar1 = &param_6;
  }
  FUN_005647e0(param_1,param_2,param_3,param_4,param_5,puVar3,puVar2,puVar1);
  return;
}


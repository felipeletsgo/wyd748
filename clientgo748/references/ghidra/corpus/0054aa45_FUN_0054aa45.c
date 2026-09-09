// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0054aa45 | Name: FUN_0054aa45


void __fastcall FUN_0054aa45(undefined4 *param_1)

{
  undefined4 *puVar1;
  undefined4 *local_8;
  
  *param_1 = &PTR_FUN_005a5cfc;
  puVar1 = (undefined4 *)param_1[4];
  while (local_8 = puVar1, local_8 != (undefined4 *)0x0) {
    puVar1 = (undefined4 *)local_8[3];
    if ((local_8 != (undefined4 *)0x0) && (local_8 != (undefined4 *)0x0)) {
      (**(code **)*local_8)(1);
    }
  }
  if (param_1[2] != 0) {
    *(undefined4 *)(param_1[2] + 0xc) = param_1[3];
  }
  if (param_1[3] != 0) {
    *(undefined4 *)(param_1[3] + 8) = param_1[2];
  }
  if (param_1[1] != 0) {
    if ((*(undefined4 **)(param_1[1] + 0x10) == param_1) &&
       (*(undefined4 *)(param_1[1] + 0x10) = param_1[3], param_1[3] != 0)) {
      *(undefined4 *)(param_1[3] + 4) = param_1[1];
    }
    param_1[1] = 0;
  }
  return;
}


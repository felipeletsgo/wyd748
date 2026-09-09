// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0056f6d0 | Name: FUN_0056f6d0


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __fastcall FUN_0056f6d0(int param_1)

{
  undefined4 uVar1;
  
  uVar1 = _DAT_005a6978;
  if ((*(int *)(param_1 + 4) != 0x32545844) && (*(int *)(param_1 + 4) != 0x33545844)) {
    uVar1 = _DAT_005a696c;
  }
  *(undefined4 *)(param_1 + 0x106c) = uVar1;
  *(float *)(param_1 + 0x1070) = 1.0 / *(float *)(param_1 + 0x106c);
  *(float *)(param_1 + 0x1c) =
       (float)(int)ROUND(*(float *)(param_1 + 0x1c) * _DAT_005a6974 + _DAT_005a67d4) * _DAT_005a69a4
  ;
  *(float *)(param_1 + 0x20) =
       (float)(int)ROUND(*(float *)(param_1 + 0x20) * _DAT_005a6970 + _DAT_005a67d4) * _DAT_005a69a0
  ;
  *(float *)(param_1 + 0x24) =
       (float)(int)ROUND(*(float *)(param_1 + 0x24) * _DAT_005a6974 + _DAT_005a67d4) * _DAT_005a69a4
  ;
  *(float *)(param_1 + 0x28) =
       (float)(int)ROUND(*(float *)(param_1 + 0x106c) * *(float *)(param_1 + 0x28) + _DAT_005a67d4)
       * *(float *)(param_1 + 0x1070);
  return;
}


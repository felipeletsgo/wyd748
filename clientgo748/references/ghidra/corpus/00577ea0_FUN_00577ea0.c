// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00577ea0 | Name: FUN_00577ea0


void FUN_00577ea0(undefined4 *param_1,undefined4 *param_2,uint param_3)

{
  int iVar1;
  
  for (iVar1 = (param_3 & 0x1ffffff) << 5; iVar1 != 0; iVar1 = iVar1 + -1) {
    *param_2 = *param_1;
    param_1 = param_1 + 1;
    param_2 = param_2 + 1;
  }
  for (iVar1 = 0; iVar1 != 0; iVar1 = iVar1 + -1) {
    *(undefined1 *)param_2 = *(undefined1 *)param_1;
    param_1 = (undefined4 *)((int)param_1 + 1);
    param_2 = (undefined4 *)((int)param_2 + 1);
  }
  return;
}


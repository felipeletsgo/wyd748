// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0042519a | Name: FUN_0042519a


void __fastcall FUN_0042519a(int param_1)

{
  int iVar1;
  
  iVar1 = *(int *)(param_1 + 0x10) - *(int *)(param_1 + 0x14);
  if ((0 < iVar1) && (iVar1 < 0x20001)) {
    FUN_0058f220(*(undefined4 *)(param_1 + 8),*(int *)(param_1 + 8) + *(int *)(param_1 + 0x14),iVar1
                );
    *(undefined4 *)(param_1 + 0x14) = 0;
    *(int *)(param_1 + 0x10) = *(int *)(param_1 + 0x10) - iVar1;
  }
  return;
}


// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00425200 | Name: FUN_00425200


void __fastcall FUN_00425200(int param_1)

{
  int iVar1;
  
  iVar1 = *(int *)(param_1 + 0xc) - *(int *)(param_1 + 0x18);
  if ((0 < iVar1) && (iVar1 < 0x20001)) {
    FUN_0058f220(*(undefined4 *)(param_1 + 4),*(int *)(param_1 + 4) + *(int *)(param_1 + 0x18),iVar1
                );
    *(undefined4 *)(param_1 + 0x18) = 0;
    *(int *)(param_1 + 0xc) = *(int *)(param_1 + 0xc) - iVar1;
  }
  return;
}


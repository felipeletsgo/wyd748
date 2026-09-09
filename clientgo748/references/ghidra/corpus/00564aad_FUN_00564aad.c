// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00564aad | Name: FUN_00564aad


void __fastcall FUN_00564aad(int param_1)

{
  int iVar1;
  
  if ((*(int *)(param_1 + 4) != 0) && (*(int *)(param_1 + 0x38) != 0)) {
    FUN_0058ea3a(*(int *)(param_1 + 4));
  }
  if ((*(int *)(param_1 + 8) != 0) && (*(int *)(param_1 + 0x3c) != 0)) {
    FUN_0058ea3a(*(int *)(param_1 + 8));
  }
  iVar1 = *(int *)(param_1 + 0x4c);
  if (iVar1 != 0) {
    FUN_00564aad();
    FUN_0058ea3a(iVar1);
  }
  iVar1 = *(int *)(param_1 + 0x50);
  if (iVar1 != 0) {
    FUN_00564aad();
    FUN_0058ea3a(iVar1);
  }
  return;
}


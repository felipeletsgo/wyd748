// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00405376 | Name: FUN_00405376


void __fastcall FUN_00405376(int param_1)

{
  int iVar1;
  undefined4 uVar2;
  undefined4 local_8;
  
  local_8 = *(int *)(param_1 + 0x234);
  if (local_8 < -2) {
    local_8 = -*(int *)(param_1 + 0x234);
  }
  iVar1 = FUN_004b8c45(local_8);
  if (iVar1 != 0) {
    uVar2 = __ftol();
    *(undefined4 *)(*(int *)(iVar1 + 4) + 0x10 + *(int *)(param_1 + 0x238) * 0x1c) = uVar2;
    uVar2 = __ftol();
    *(undefined4 *)(*(int *)(iVar1 + 4) + 8 + *(int *)(param_1 + 0x238) * 0x1c) = uVar2;
    uVar2 = __ftol();
    *(undefined4 *)(*(int *)(iVar1 + 4) + 0xc + *(int *)(param_1 + 0x238) * 0x1c) = uVar2;
    uVar2 = __ftol();
    *(undefined4 *)(*(int *)(iVar1 + 4) + 4 + *(int *)(param_1 + 0x238) * 0x1c) = uVar2;
  }
  return;
}


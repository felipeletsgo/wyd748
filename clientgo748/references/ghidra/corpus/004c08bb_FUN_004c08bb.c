// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004c08bb | Name: FUN_004c08bb


void __thiscall FUN_004c08bb(int param_1,undefined4 param_2)

{
  int iVar1;
  int iVar2;
  undefined4 local_8;
  
  if (*(int *)(param_1 + 0x90) != 0) {
    iVar1 = *(int *)(*(int *)(param_1 + 0x90) + 0xc);
    for (local_8 = 0; local_8 < iVar1; local_8 = local_8 + 1) {
      iVar2 = FUN_004c15fc(*(undefined4 *)(*(int *)(*(int *)(param_1 + 0x90) + 0xb4) + local_8 * 4))
      ;
      if (iVar2 != 0) {
        *(int *)(*(int *)(param_1 + 0x90) + 0x14 + local_8 * 4) = iVar2 + 0x48;
      }
    }
  }
  if (*(int *)(param_1 + 0x8c) != 0) {
    FUN_004c08bb(param_2);
  }
  if (*(int *)(param_1 + 0x88) != 0) {
    FUN_004c08bb(param_2);
  }
  return;
}


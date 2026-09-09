// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00584702 | Name: FUN_00584702


void __fastcall FUN_00584702(int param_1)

{
  int iVar1;
  undefined4 uVar2;
  
  iVar1 = *(int *)(param_1 + 0x184);
  if (*(int *)(param_1 + 0x120) < 2) {
    if (*(uint *)(param_1 + 0x80) < *(int *)(param_1 + 0x118) - 1U) {
      uVar2 = *(undefined4 *)(*(int *)(param_1 + 0x124) + 0xc);
    }
    else {
      uVar2 = *(undefined4 *)(*(int *)(param_1 + 0x124) + 0x48);
    }
    *(undefined4 *)(iVar1 + 0x1c) = uVar2;
  }
  else {
    *(undefined4 *)(iVar1 + 0x1c) = 1;
  }
  *(undefined4 *)(iVar1 + 0x14) = 0;
  *(undefined4 *)(iVar1 + 0x18) = 0;
  return;
}


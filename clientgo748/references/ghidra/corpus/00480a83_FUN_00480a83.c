// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00480a83 | Name: FUN_00480a83


void __fastcall FUN_00480a83(int param_1)

{
  int iVar1;
  int iVar2;
  undefined2 extraout_var;
  undefined2 extraout_var_00;
  uint local_1c [4];
  float local_c;
  int local_8;
  
  iVar1 = DAT_013b71e8;
  iVar2 = DAT_013b71e8 + 0x6ec;
  _memset(local_1c,0,0x10);
  FUN_0058f220(local_1c,*(int *)(param_1 + 0x4c) + 0x1f2,0x10);
  FUN_00524ded(iVar2);
  if (((local_1c[0] & 0xff) != 0) && (*(short *)(iVar1 + 0x748) != 0x20)) {
    FUN_0058f220(*(int *)(param_1 + 0x4c) + 0x1f2,local_1c,0x10);
  }
  local_c = (float)(int)*(short *)(*(int *)(param_1 + 0x4c) + 0x45a);
  FUN_005277a7(local_c);
  FUN_004faf13(CONCAT22(extraout_var,*(undefined2 *)(iVar1 + 0x748)));
  local_8 = FUN_0054cd07(iVar1 + 0x778,0x15);
  if (local_8 == 0x29) {
    *(undefined2 *)(*(int *)(param_1 + 0x4c) + 0x1ea) =
         *(undefined2 *)(*(int *)(param_1 + 0x4c) + 0x1ee);
    *(undefined2 *)(*(int *)(param_1 + 0x4c) + 0x1ec) =
         *(undefined2 *)(*(int *)(param_1 + 0x4c) + 0x1f0);
    *(undefined1 *)(*(int *)(param_1 + 0x4c) + 0x1f8) =
         *(undefined1 *)(*(int *)(param_1 + 0x4c) + 0x1f9);
    *(undefined1 *)(*(int *)(param_1 + 0x4c) + 0x200) =
         *(undefined1 *)(*(int *)(param_1 + 0x4c) + 0x201);
  }
  (**(code **)(**(int **)(param_1 + 0x4c) + 0x38))();
  FUN_0051bb41(CONCAT22((short)((uint)iVar2 >> 0x10),*(undefined2 *)(iVar1 + 0x778)),
               CONCAT22(extraout_var_00,*(undefined2 *)(iVar1 + 0x780)));
  (**(code **)(**(int **)(param_1 + 0x4c) + 0x40))
            (0,*(undefined4 *)(*(int *)(param_1 + 0x4c) + 0x34),0);
  FUN_0052433d();
  FUN_00480c25();
  return;
}


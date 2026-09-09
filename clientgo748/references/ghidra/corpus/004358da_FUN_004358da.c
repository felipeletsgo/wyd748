// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004358da | Name: FUN_004358da


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __fastcall FUN_004358da(undefined4 *param_1)

{
  int iVar1;
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  puStack_c = &LAB_0059f1cc;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  *param_1 = &PTR_FUN_005a4294;
  local_8 = 0;
  _DAT_005b9a44 = 0;
  _DAT_005b9a48 = 0;
  if (DAT_013b736c == 1) {
    (**(code **)(*DAT_013b71e0 + 0x14))();
    FUN_0055ee59(0);
  }
  if (param_1[0x9f32] != 0) {
    _memset((void *)(DAT_013b71e8 + 0x1b094),0,0x3390);
    for (local_14 = 0; local_14 < *(int *)(param_1[0x9f32] + 0x210); local_14 = local_14 + 1) {
      *(undefined4 *)(DAT_013b71e8 + 0x1b094 + local_14 * 0x84) =
           *(undefined4 *)(*(int *)(param_1[0x9f32] + 0x214 + local_14 * 4) + 0x754);
      FUN_0058f078(DAT_013b71e8 + 0x1b098 + local_14 * 0x84,&DAT_005b78b4,
                   *(int *)(param_1[0x9f32] + 0x214 + local_14 * 4) + 0x76a);
    }
  }
  *(undefined4 *)(DAT_005ccf9c + 0x2a5e4) = 0;
  if (DAT_005ccf98 != 0) {
    iVar1 = FUN_00429a6d(0x6b);
    if (iVar1 != 0) {
      FUN_00429a6d(0x6b);
      iVar1 = FUN_0042afd0();
      if (iVar1 != 0) {
        FUN_00429a6d(0x6b);
        FUN_0042aef6();
      }
    }
  }
  if (DAT_005ccf98 != 0) {
    iVar1 = FUN_00429a6d(6);
    if (iVar1 != 0) {
      FUN_00429a6d(6);
      iVar1 = FUN_0042afd0();
      if (iVar1 != 0) {
        FUN_00429a6d(6);
        FUN_0042aef6();
      }
    }
  }
  if ((((DAT_0067cf38 != 0) && (*(int *)(DAT_0067cf38 + 0x24) != 0x7534)) &&
      (*(int *)(DAT_0067cf38 + 0x24) != 30000)) &&
     ((*(int *)(DAT_0067cf38 + 0x24) != 0x7535 && (DAT_013b71e0[0x3b] != 0)))) {
    FUN_00429480();
  }
  local_8 = 0xffffffff;
  FUN_00494c00();
  ExceptionList = local_10;
  return;
}


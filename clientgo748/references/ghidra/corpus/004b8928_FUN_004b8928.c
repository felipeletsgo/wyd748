// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004b8928 | Name: FUN_004b8928


undefined4 __fastcall FUN_004b8928(int param_1)

{
  int iVar1;
  undefined4 uVar2;
  void *local_134;
  undefined1 local_124 [128];
  int local_a4;
  int local_a0;
  int local_9c;
  int local_98;
  undefined1 local_94 [128];
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_005a067e;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  local_9c = FUN_0058f716(s_UI_UITextureSetList_txt_005ba7ec,&DAT_005ba7e8);
  if (local_9c == 0) {
    uVar2 = 0;
  }
  else {
    while (iVar1 = FUN_00590a68(local_9c,s__s_SetIndex___d_ItemCount___d_005ba804,local_94,&local_98
                                ,&local_14), iVar1 != -1) {
      if (local_98 < 0x200) {
        *(int *)(param_1 + 0x328 + local_98 * 8) = local_14;
        local_a0 = local_14;
        local_134 = operator_new(local_14 * 0x1c);
        local_8 = 0;
        if (local_134 == (void *)0x0) {
          local_134 = (void *)0x0;
        }
        else {
          FUN_00421910(local_134,0x1c,local_a0,FUN_004bac10);
        }
        local_8 = 0xffffffff;
        *(void **)(param_1 + 0x32c + local_98 * 8) = local_134;
        for (local_a4 = 0; local_a4 < local_14; local_a4 = local_a4 + 1) {
          FUN_00590a68(local_9c,s__d__d__d__d__d__d__d_005ba850,
                       *(int *)(param_1 + 0x32c + local_98 * 8) + local_a4 * 0x1c,
                       *(int *)(param_1 + 0x32c + local_98 * 8) + 4 + local_a4 * 0x1c,
                       *(int *)(param_1 + 0x32c + local_98 * 8) + 8 + local_a4 * 0x1c,
                       *(int *)(param_1 + 0x32c + local_98 * 8) + 0xc + local_a4 * 0x1c,
                       *(int *)(param_1 + 0x32c + local_98 * 8) + 0x10 + local_a4 * 0x1c,
                       *(int *)(param_1 + 0x32c + local_98 * 8) + 0x14 + local_a4 * 0x1c,
                       *(int *)(param_1 + 0x32c + local_98 * 8) + 0x18 + local_a4 * 0x1c);
        }
      }
      else {
        FUN_0058f078(local_124,s_Cannot_read___s__in_UI_TextureSe_005ba828,local_94);
        FUN_00431159(local_124);
      }
    }
    FUN_0058f62c(local_9c);
    uVar2 = 1;
  }
  ExceptionList = local_10;
  return uVar2;
}


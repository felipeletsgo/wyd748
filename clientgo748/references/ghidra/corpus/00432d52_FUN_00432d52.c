// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00432d52 | Name: FUN_00432d52


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __fastcall FUN_00432d52(int param_1)

{
  void *pvVar1;
  int iVar2;
  undefined4 uVar3;
  undefined2 extraout_var;
  float10 fVar4;
  undefined4 local_80;
  short local_6c;
  short local_68 [4];
  undefined4 local_60;
  undefined1 local_5c;
  undefined1 local_5b;
  undefined1 local_5a;
  undefined1 local_59;
  undefined1 local_58;
  undefined1 local_57;
  undefined1 local_56;
  undefined1 local_55;
  undefined1 local_53;
  undefined1 local_52;
  undefined1 local_51;
  undefined1 local_50;
  undefined1 local_4f;
  undefined1 local_4e;
  undefined1 local_4d;
  int local_4c;
  undefined2 local_48;
  undefined2 local_46;
  undefined2 local_44;
  undefined2 local_42;
  undefined2 local_40;
  undefined2 local_3e;
  undefined2 local_3c;
  undefined2 local_3a;
  undefined2 local_38;
  undefined2 local_36;
  undefined2 local_34;
  undefined2 local_32;
  undefined2 local_30;
  undefined2 local_2e;
  undefined2 local_2c;
  undefined2 local_2a;
  undefined2 local_28;
  int *local_24;
  int local_20;
  int local_1c;
  int local_18;
  int local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0059f19b;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  FUN_00549bbf(0);
  if (*(int *)(param_1 + 0x26e30) != 0) {
    for (local_1c = 0; local_1c < 0x32; local_1c = local_1c + 1) {
      if (*(int *)(param_1 + 0x26e30 + local_1c * 4) != 0) {
        (**(code **)(*DAT_013b71e8 + 100))(*(undefined4 *)(param_1 + 0x26e30 + local_1c * 4));
        *(undefined4 *)(param_1 + 0x26e30 + local_1c * 4) = 0;
      }
    }
  }
  _memset((void *)(param_1 + 0x26ef8),0,0x960);
  local_18 = FUN_0058f716(s_UI_EndDemo_bin_005b4c88,&DAT_005b4c84);
  if (local_18 != 0) {
    local_24 = DAT_013b71e8 + 0x1bb;
    for (local_20 = 0; local_20 < 0x32; local_20 = local_20 + 1) {
      *(undefined4 *)(param_1 + 0x26f04 + local_20 * 0x30) = 0;
      local_4c = FUN_00590d68(param_1 + 0x26ef8 + local_20 * 0x30,1,0x30,local_18);
      if (local_4c == 0) break;
      local_28 = 0;
      _memset(&local_48,0,0x20);
      _memset(&local_5c,0,0x10);
      local_28 = *(undefined2 *)(param_1 + 0x26ef8 + local_20 * 0x30);
      local_48 = *(undefined2 *)
                  (&DAT_00d44990 + *(int *)(param_1 + 0x26ef8 + local_20 * 0x30) * 0x8c);
      local_46 = *(undefined2 *)
                  (&DAT_00d44992 + *(int *)(param_1 + 0x26ef8 + local_20 * 0x30) * 0x8c);
      local_44 = *(undefined2 *)
                  (&DAT_00d44990 + *(int *)(param_1 + 0x26efc + local_20 * 0x30) * 0x8c);
      local_42 = *(undefined2 *)
                  (&DAT_00d44992 + *(int *)(param_1 + 0x26efc + local_20 * 0x30) * 0x8c);
      local_40 = *(undefined2 *)
                  (&DAT_00d44990 + *(int *)(param_1 + 0x26f00 + local_20 * 0x30) * 0x8c);
      local_3e = *(undefined2 *)
                  (&DAT_00d44992 + *(int *)(param_1 + 0x26f00 + local_20 * 0x30) * 0x8c);
      local_3c = *(undefined2 *)
                  (&DAT_00d44990 + *(int *)(param_1 + 0x26f00 + local_20 * 0x30) * 0x8c);
      local_3a = *(undefined2 *)
                  (&DAT_00d44992 + *(int *)(param_1 + 0x26f00 + local_20 * 0x30) * 0x8c);
      local_38 = *(undefined2 *)
                  (&DAT_00d44990 + *(int *)(param_1 + 0x26f00 + local_20 * 0x30) * 0x8c);
      local_36 = *(undefined2 *)
                  (&DAT_00d44992 + *(int *)(param_1 + 0x26f00 + local_20 * 0x30) * 0x8c);
      local_34 = *(undefined2 *)
                  (&DAT_00d44990 + *(int *)(param_1 + 0x26f00 + local_20 * 0x30) * 0x8c);
      local_32 = *(undefined2 *)
                  (&DAT_00d44992 + *(int *)(param_1 + 0x26f00 + local_20 * 0x30) * 0x8c);
      local_30 = *(undefined2 *)
                  (&DAT_00d44990 + *(int *)(param_1 + 0x26f08 + local_20 * 0x30) * 0x8c);
      local_2e = *(undefined2 *)
                  (&DAT_00d44992 + *(int *)(param_1 + 0x26f08 + local_20 * 0x30) * 0x8c);
      local_2c = *(undefined2 *)
                  (&DAT_00d44990 + *(int *)(param_1 + 0x26f0c + local_20 * 0x30) * 0x8c);
      local_2a = *(undefined2 *)
                  (&DAT_00d44992 + *(int *)(param_1 + 0x26f0c + local_20 * 0x30) * 0x8c);
      local_5a = *(undefined1 *)(param_1 + 0x26f10 + local_20 * 0x30);
      local_52 = (&DAT_00d449da)[*(int *)(param_1 + 0x26f00 + local_20 * 0x30) * 0x8c];
      if (*(int *)(param_1 + 0x26f00 + local_20 * 0x30) ==
          *(int *)(param_1 + 0x26efc + local_20 * 0x30)) {
        local_5b = local_5a;
        local_53 = local_52;
      }
      local_59 = local_5a;
      local_58 = local_5a;
      local_57 = local_5a;
      local_56 = local_5a;
      local_55 = local_5a;
      local_51 = local_52;
      local_50 = local_52;
      local_4f = local_52;
      local_4e = local_52;
      local_4d = local_52;
      if (local_4c == -1) break;
      pvVar1 = operator_new(0x894);
      local_8 = 0;
      if (pvVar1 == (void *)0x0) {
        local_80 = 0;
      }
      else {
        local_80 = FUN_004f7ea6(param_1);
      }
      local_8 = 0xffffffff;
      *(undefined4 *)(param_1 + 0x26e30 + local_20 * 4) = local_80;
      *(undefined2 *)(*(int *)(param_1 + 0x26e30 + local_20 * 4) + 0x450) = 1;
      *(undefined4 *)(*(int *)(param_1 + 0x26e30 + local_20 * 4) + 0x20) = 10000;
      FUN_0058f078(*(int *)(param_1 + 0x26e30 + local_20 * 4) + 0x168,s_person_d_005b4c98,local_20);
      if (((0x937 < *(int *)(param_1 + 0x26f14 + local_20 * 0x30)) &&
          (*(int *)(param_1 + 0x26f14 + local_20 * 0x30) < 0x956)) ||
         ((0xb8f < *(int *)(param_1 + 0x26f14 + local_20 * 0x30) &&
          (*(int *)(param_1 + 0x26f14 + local_20 * 0x30) < 3000)))) {
        *(undefined1 *)(*(int *)(param_1 + 0x26e30 + local_20 * 4) + 0x79c) = 1;
        iVar2 = *(int *)(param_1 + 0x26f14 + local_20 * 0x30) + -0x7fd;
        _memset(local_68,0,8);
        local_6c = (short)iVar2;
        local_68[0] = local_6c;
        *(short *)(*(int *)(param_1 + 0x26e30 + local_20 * 4) + 0x1a0) = local_6c + -0x13b;
        local_60 = 0;
        uVar3 = FUN_0054cd07(local_68,0x12);
        uVar3 = FUN_004fab52(uVar3);
        *(undefined4 *)(*(int *)(param_1 + 0x26e30 + local_20 * 4) + 0x7a0) = uVar3;
        *(undefined2 *)(*(int *)(param_1 + 0x26e30 + local_20 * 4) + 0x1a2) =
             *(undefined2 *)(&DAT_00d44990 + iVar2 * 0x8c);
        *(undefined2 *)(*(int *)(param_1 + 0x26e30 + local_20 * 4) + 0x1a6) =
             *(undefined2 *)(*(int *)(param_1 + 0x26e30 + local_20 * 4) + 0x1a2);
        *(undefined2 *)(*(int *)(param_1 + 0x26e30 + local_20 * 4) + 0x1a4) =
             *(undefined2 *)(&DAT_00d44992 + iVar2 * 0x8c);
        *(undefined2 *)(*(int *)(param_1 + 0x26e30 + local_20 * 4) + 0x1a8) =
             *(undefined2 *)(*(int *)(param_1 + 0x26e30 + local_20 * 4) + 0x1a4);
        if ((iVar2 < 0x141) || (0x145 < iVar2)) {
          if ((iVar2 < 0x146) || (0x14a < iVar2)) {
            *(undefined2 *)(*(int *)(param_1 + 0x26e30 + local_20 * 4) + 0x1aa) = 0;
            *(undefined1 *)(*(int *)(param_1 + 0x26e30 + local_20 * 4) + 0x1c4) = 0;
            *(undefined1 *)(*(int *)(param_1 + 0x26e30 + local_20 * 4) + 0x1c2) =
                 (undefined1)local_60;
            *(undefined1 *)(*(int *)(param_1 + 0x26e30 + local_20 * 4) + 0x1c3) =
                 (undefined1)local_60;
          }
          else {
            *(short *)(*(int *)(param_1 + 0x26e30 + local_20 * 4) + 0x1aa) = local_6c + -0x145;
            *(undefined1 *)(*(int *)(param_1 + 0x26e30 + local_20 * 4) + 0x1c2) = 0;
            *(undefined1 *)(*(int *)(param_1 + 0x26e30 + local_20 * 4) + 0x1c3) = 0;
            *(undefined1 *)(*(int *)(param_1 + 0x26e30 + local_20 * 4) + 0x1c4) =
                 (undefined1)local_60;
          }
        }
        else {
          *(short *)(*(int *)(param_1 + 0x26e30 + local_20 * 4) + 0x1aa) = local_6c + -0x140;
          *(undefined1 *)(*(int *)(param_1 + 0x26e30 + local_20 * 4) + 0x1c2) = 0;
          *(undefined1 *)(*(int *)(param_1 + 0x26e30 + local_20 * 4) + 0x1c3) = 0;
          *(undefined1 *)(*(int *)(param_1 + 0x26e30 + local_20 * 4) + 0x1c4) = (undefined1)local_60
          ;
        }
        fVar4 = (float10)FUN_004faaf4(*(undefined4 *)
                                       (*(int *)(param_1 + 0x26e30 + local_20 * 4) + 0x7a0),
                                      *(undefined2 *)
                                       (*(int *)(param_1 + 0x26e30 + local_20 * 4) + 0x1a2));
        *(float *)(*(int *)(param_1 + 0x26e30 + local_20 * 4) + 0x7a4) = (float)fVar4;
      }
      FUN_0058f220(*(int *)(param_1 + 0x26e30 + local_20 * 4) + 0x1d2,&local_48,0x20);
      FUN_0058f220(*(int *)(param_1 + 0x26e30 + local_20 * 4) + 0x1f2,&local_5c,0x10);
      if (0 < *(int *)(param_1 + 0x26f04 + local_20 * 0x30)) {
        *(undefined1 *)(*(int *)(param_1 + 0x26e30 + local_20 * 4) + 0x79b) = 1;
        *(undefined2 *)(*(int *)(param_1 + 0x26e30 + local_20 * 4) + 0x7aa) =
             *(undefined2 *)(&DAT_00d44992 + *(int *)(param_1 + 0x26f04 + local_20 * 0x30) * 0x8c);
        *(undefined1 *)(*(int *)(param_1 + 0x26e30 + local_20 * 4) + 0x7ac) =
             *(undefined1 *)(param_1 + 0x26f10 + local_20 * 0x30);
        *(undefined *)(*(int *)(param_1 + 0x26e30 + local_20 * 4) + 0x7ad) =
             (&DAT_00d449da)[*(int *)(param_1 + 0x26f04 + local_20 * 0x30) * 0x8c];
      }
      *(undefined1 *)(*(int *)(param_1 + 0x26e30 + local_20 * 4) + 0x244) = 1;
      FUN_004faf13(local_28);
      (**(code **)(**(int **)(param_1 + 0x26e30 + local_20 * 4) + 0x38))();
      FUN_0051bb41(CONCAT22(extraout_var,*(undefined2 *)(param_1 + 0x26f0c + local_20 * 0x30)),
                   CONCAT22(extraout_var,*(undefined2 *)(param_1 + 0x26f08 + local_20 * 0x30)));
      (**(code **)(**(int **)(param_1 + 0x26e30 + local_20 * 4) + 0x40))
                (0,((float)*(int *)(param_1 + 0x26f20 + local_20 * 0x30) * _DAT_005a4290) /
                   _DAT_005a428c,0);
      (**(code **)(**(int **)(param_1 + 0x26e30 + local_20 * 4) + 0x3c))
                ((float)*(int *)(param_1 + 0x26f18 + local_20 * 0x30),0,
                 (float)*(int *)(param_1 + 0x26f1c + local_20 * 0x30));
      *(float *)(*(int *)(param_1 + 0x26e30 + local_20 * 4) + 0x220) =
           (float)*(int *)(param_1 + 0x26f24 + local_20 * 0x30);
      *(undefined1 *)(*(int *)(param_1 + 0x26e30 + local_20 * 4) + 0x248) = 1;
      FUN_0054ac09(*(undefined4 *)(param_1 + 0x26e30 + local_20 * 4));
    }
    FUN_0058f62c(local_18);
  }
  FUN_0043381a();
  for (local_14 = 0; local_14 < 0x32; local_14 = local_14 + 1) {
    if (*(int *)(param_1 + 0x26e30 + local_14 * 4) != 0) {
      *(undefined4 *)(*(int *)(param_1 + 0x26e30 + local_14 * 4) + 0x48) = 1;
      (**(code **)(**(int **)(param_1 + 0x26e30 + local_14 * 4) + 0x20))(0);
      (**(code **)(**(int **)(param_1 + 0x26e30 + local_14 * 4) + 0x24))();
    }
  }
  ExceptionList = local_10;
  return;
}


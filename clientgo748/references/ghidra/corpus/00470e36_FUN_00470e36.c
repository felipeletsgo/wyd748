// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00470e36 | Name: FUN_00470e36


void __fastcall FUN_00470e36(int param_1)

{
  int iVar1;
  int iVar2;
  int iVar3;
  undefined4 *puVar4;
  undefined4 uVar5;
  undefined4 uVar6;
  int local_58;
  short local_54;
  undefined1 local_44 [4];
  undefined2 local_40;
  undefined2 local_3e;
  undefined4 local_38;
  int local_34;
  undefined4 local_30;
  undefined4 local_2c;
  undefined2 local_28;
  undefined2 local_26;
  undefined2 local_24;
  undefined1 local_22 [18];
  int local_10;
  undefined4 local_c;
  undefined4 local_8;
  
  iVar3 = DAT_0067cf38;
  if ((((*(int *)(param_1 + 0x289a4) != 0) && (-1 < *(short *)(param_1 + 0x27130))) &&
      (-1 < *(short *)(param_1 + 0x27132))) &&
     (((local_10 = (**(code **)(*DAT_0092e654 + 8))(), *(int *)(iVar3 + 0x27478) == 0 ||
       (199 < (uint)(local_10 - *(int *)(iVar3 + 0x27478)))) &&
      ((*(int *)(param_1 + 0x2873c) != 0 &&
       (iVar1 = (**(code **)(**(int **)(param_1 + 0x2873c) + 0xb4))
                          ((int)*(short *)(param_1 + 0x27130),(int)*(short *)(param_1 + 0x27132)),
       iVar1 != 0)))))) {
    local_c = *(undefined4 *)(iVar3 + 0x4c);
    (**(code **)(**(int **)(param_1 + 0x2873c) + 0xc0))
              (*(undefined4 *)(*(int *)(iVar1 + 0x1e4) + 0x1e4),
               *(undefined4 *)(*(int *)(iVar1 + 0x1e4) + 0x400));
    local_54 = (**(code **)(**(int **)(param_1 + 0x2873c) + 0xc4))
                         (*(undefined4 *)(*(int *)(iVar1 + 0x1e4) + 0x1e4));
    if (local_54 == -1) {
      local_54 = (short)*(undefined4 *)(iVar1 + 0x1d0) + (short)*(undefined4 *)(iVar1 + 0x1d4) * 9;
    }
    iVar2 = FUN_0054cd07(*(undefined4 *)(iVar1 + 0x670),0x26);
    _memset(local_44,0,0x34);
    local_3e = *(undefined2 *)(DAT_013b71e8 + 0xc5c);
    local_40 = 0x3c9;
    local_38 = 1;
    local_34 = (int)local_54;
    if (iVar2 == 0xf) {
      local_30 = 0;
      local_2c = 0xe;
    }
    local_24 = 0;
    local_28 = __ftol();
    local_26 = __ftol();
    for (local_58 = 0; local_58 < 100; local_58 = local_58 + 1) {
      if (*(int *)(*(int *)(param_1 + 0x289a8 + local_58 * 4) + 0x1e8) != 0) {
        FUN_0054b2ac(local_22,local_58);
      }
    }
    FUN_0055f2dd(local_44,0x34);
    *(undefined2 *)(param_1 + 0x27130) = 0xffff;
    *(undefined2 *)(param_1 + 0x27132) = 0xffff;
    *(int *)(iVar3 + 0x27478) = local_10;
    iVar3 = FUN_0054e93b(*(undefined4 *)(iVar1 + 0x670));
    if (iVar3 < 2) {
      puVar4 = (undefined4 *)
               (**(code **)(**(int **)(param_1 + 0x2873c) + 0xa4))
                         (*(undefined4 *)(iVar1 + 0x1d0),*(undefined4 *)(iVar1 + 0x1d4));
      if ((*(int *)(DAT_005ccec0 + 0x1e8) != 0) &&
         (*(undefined4 **)(DAT_005ccec0 + 0x1e8) == puVar4)) {
        *(undefined4 *)(DAT_005ccec0 + 0x1e8) = 0;
      }
      if ((puVar4 != (undefined4 *)0x0) && (puVar4 != (undefined4 *)0x0)) {
        (**(code **)*puVar4)(1);
      }
    }
    else {
      FUN_0054e9ab(*(undefined4 *)(iVar1 + 0x670),iVar3 + -1);
      FUN_0058f078(iVar1 + 0x39a,&PTR_DAT_005b824c,iVar3 + -1);
      FUN_00421b8f(iVar1 + 0x39a,*(undefined4 *)(iVar1 + 900),0);
    }
    local_8 = 0x29;
    if ((10 < iVar2) && (iVar2 < 0xe)) {
      local_8 = 0x36;
    }
    if (((iVar2 != 0x13) && (DAT_005ccf98 != 0)) && (iVar1 = FUN_00429a6d(local_8), iVar1 != 0)) {
      uVar6 = 0;
      uVar5 = 0;
      FUN_00429a6d(local_8);
      FUN_0042ad2b(uVar5,uVar6);
    }
    if (*(int *)(DAT_0067cf38 + 0x24) == 30000) {
      FUN_004431e4(0);
    }
    if (iVar3 < 2) {
      _memset((void *)(DAT_013b71e8 + 0x7c8 + local_54 * 8),0,8);
    }
  }
  return;
}


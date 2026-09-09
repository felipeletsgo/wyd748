// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00465f85 | Name: FUN_00465f85


void __thiscall
FUN_00465f85(int param_1,int param_2,int param_3,int param_4,undefined4 param_5,undefined4 param_6)

{
  undefined4 *puVar1;
  int iVar2;
  undefined4 uVar3;
  undefined4 uVar4;
  int local_44;
  short local_3c;
  undefined1 local_30 [4];
  undefined2 local_2c;
  undefined2 local_2a;
  undefined4 local_24;
  int local_20;
  undefined4 local_1c;
  undefined4 local_18;
  undefined2 local_14;
  undefined2 local_12;
  undefined2 local_10;
  int local_c;
  undefined4 local_8;
  
  if (((param_2 != 0) && (*(int *)(param_1 + 0x2873c) != 0)) &&
     ((local_c = (**(code **)(*DAT_0092e654 + 8))(), *(int *)(param_1 + 0x27478) == 0 ||
      (199 < (uint)(local_c - *(int *)(param_1 + 0x27478)))))) {
    (**(code **)(**(int **)(param_1 + 0x2873c) + 0xc0))
              (*(undefined4 *)(*(int *)(param_2 + 0x1e4) + 0x1e4),
               *(undefined4 *)(*(int *)(param_2 + 0x1e4) + 0x400));
    local_3c = (**(code **)(**(int **)(param_1 + 0x2873c) + 0xc4))
                         (*(undefined4 *)(*(int *)(param_2 + 0x1e4) + 0x1e4));
    if (local_3c == -1) {
      local_3c = (short)*(undefined4 *)(param_2 + 0x1d0) +
                 (short)*(undefined4 *)(param_2 + 0x1d4) * 9;
    }
    _memset(local_30,0,0x24);
    local_2a = *(undefined2 *)(DAT_013b71e8 + 0xc5c);
    local_2c = 0x373;
    local_24 = 1;
    local_20 = (int)local_3c;
    if (param_3 == 0xf) {
      local_1c = 0;
      local_18 = 0xe;
    }
    local_10 = 0;
    local_14 = __ftol();
    local_12 = __ftol();
    FUN_0055f2dd(local_30,0x24);
    *(int *)(param_1 + 0x27478) = local_c;
    *(undefined4 *)(DAT_005d0504 + 0x43c) = 1;
    local_44 = FUN_0054e93b(*(undefined4 *)(param_2 + 0x670));
    if ((0x919 < param_4) && (param_4 < 0x956)) {
      local_44 = 0;
    }
    if (local_44 < 2) {
      puVar1 = (undefined4 *)(**(code **)(**(int **)(param_1 + 0x2873c) + 0xa4))(param_5,param_6);
      if ((*(int *)(DAT_005ccec0 + 0x1e8) != 0) &&
         (*(undefined4 **)(DAT_005ccec0 + 0x1e8) == puVar1)) {
        *(undefined4 *)(DAT_005ccec0 + 0x1e8) = 0;
      }
      if ((puVar1 != (undefined4 *)0x0) && (puVar1 != (undefined4 *)0x0)) {
        (**(code **)*puVar1)(1);
      }
    }
    else {
      FUN_0054e9ab(*(undefined4 *)(param_2 + 0x670),local_44 + -1);
      FUN_0058f078(param_2 + 0x39a,&PTR_DAT_005b7ddc,local_44 + -1);
      FUN_00421b8f(param_2 + 0x39a,*(undefined4 *)(param_2 + 900),0);
    }
    local_8 = 0x29;
    if ((10 < param_3) && (param_3 < 0xe)) {
      local_8 = 0x36;
    }
    if (((param_3 != 0x13) && (DAT_005ccf98 != 0)) && (iVar2 = FUN_00429a6d(local_8), iVar2 != 0)) {
      uVar4 = 0;
      uVar3 = 0;
      FUN_00429a6d(local_8);
      FUN_0042ad2b(uVar3,uVar4);
    }
    FUN_004431e4(0);
    if (local_44 < 2) {
      _memset((void *)(DAT_013b71e8 + 0x7c8 + local_3c * 8),0,8);
    }
  }
  return;
}


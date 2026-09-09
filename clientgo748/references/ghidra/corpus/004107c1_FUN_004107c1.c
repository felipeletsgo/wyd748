// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004107c1 | Name: FUN_004107c1


undefined4 __thiscall FUN_004107c1(int *param_1,int param_2)

{
  int iVar1;
  undefined4 *puVar2;
  int local_2c;
  int local_28;
  undefined1 local_24 [4];
  undefined2 local_20;
  undefined2 local_1e;
  char local_18 [20];
  
  if ((((param_1[0xb] != 0) && (param_2 == 0x2e)) && (param_1[0x100] == 5)) &&
     (*(int *)(*(int *)(DAT_0067cf38 + 0x28) + 300) == 0)) {
    local_2c = -1;
    for (local_28 = 0; local_28 < 10; local_28 = local_28 + 1) {
      iVar1 = (**(code **)(*param_1 + 0xb4))(local_28,0);
      if (((DAT_005b892c == 2) && (iVar1 != 0)) && (*(int *)(iVar1 + 0xa0) == 200)) {
        local_2c = local_28;
        puVar2 = (undefined4 *)(**(code **)(*param_1 + 0xa4))(local_28,0);
        if ((*(int *)(DAT_005ccec0 + 0x1e8) != 0) &&
           (*(undefined4 **)(DAT_005ccec0 + 0x1e8) == puVar2)) {
          *(undefined4 *)(DAT_005ccec0 + 0x1e8) = 0;
        }
        if ((puVar2 != (undefined4 *)0x0) && (puVar2 != (undefined4 *)0x0)) {
          (**(code **)*puVar2)(1);
        }
        break;
      }
      if ((iVar1 != 0) && (*(int *)(iVar1 + 0xa0) == 2)) {
        local_2c = local_28;
        puVar2 = (undefined4 *)(**(code **)(*param_1 + 0xa4))(local_28,0);
        if ((*(int *)(DAT_005ccec0 + 0x1e8) != 0) &&
           (*(undefined4 **)(DAT_005ccec0 + 0x1e8) == puVar2)) {
          *(undefined4 *)(DAT_005ccec0 + 0x1e8) = 0;
        }
        if ((puVar2 != (undefined4 *)0x0) && (puVar2 != (undefined4 *)0x0)) {
          (**(code **)*puVar2)(1);
        }
        break;
      }
    }
    iVar1 = FUN_0040c0f0();
    if (iVar1 == 1) {
      local_2c = local_2c + 10;
    }
    if (-1 < local_2c) {
      *(undefined1 *)(DAT_013b71e8 + local_2c + 0xef6) = 0xff;
    }
    _memset(local_24,0,0x20);
    local_1e = *(undefined2 *)(*(int *)(DAT_0067cf38 + 0x4c) + 0x20);
    local_20 = 0x378;
    FUN_0058f220(local_18,DAT_013b71e8 + 0xef6,0x14);
    for (local_28 = 0; local_28 < 0x14; local_28 = local_28 + 1) {
      if ((-1 < local_18[local_28]) && (local_18[local_28] < '`')) {
        local_18[local_28] = local_18[local_28] + *(char *)(DAT_013b71e8 + 0x700) * -0x18;
      }
    }
    FUN_0055f2dd(local_24,0x20);
    FUN_004431e4(0);
    FUN_004470b9();
  }
  return 0;
}


// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004d0651 | Name: FUN_004d0651


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __thiscall FUN_004d0651(int param_1,int param_2)

{
  float fVar1;
  float fVar2;
  undefined1 uVar3;
  int iVar4;
  int iVar5;
  float10 fVar6;
  float10 fVar7;
  undefined4 local_3c;
  undefined4 local_38;
  undefined4 local_30;
  undefined4 local_2c;
  undefined4 local_24;
  undefined4 local_20;
  undefined4 local_1c;
  undefined4 local_14;
  undefined4 local_10;
  
  if (*(int *)(param_1 + 0x24) == 6) {
    if (param_2 != 0) {
      for (local_3c = 0; local_3c < 100; local_3c = local_3c + 1) {
        uVar3 = FUN_0054b256(param_2,local_3c);
        *(undefined1 *)(param_1 + local_3c + 0x1e48) = uVar3;
      }
    }
    *(undefined4 *)(param_1 + 0x1e44) = 0xffffffff;
    local_30 = 0;
    for (local_20 = 0; local_20 < 100; local_20 = local_20 + 1) {
      if (*(char *)(param_1 + local_20 + 0x1e48) != '\0') {
        local_30 = local_30 + 1;
      }
    }
    if (local_30 == 0) {
      local_30 = 1;
      *(undefined1 *)(param_1 + 0x1e48) = 1;
    }
    local_1c = 0;
    fVar6 = (float10)FUN_00493bf0(0xbf490fdb);
    fVar7 = (float10)FUN_00423b00(0xbf490fdb);
    for (local_10 = 0; local_10 < 10; local_10 = local_10 + 1) {
      for (local_38 = 0; local_38 < 10; local_38 = local_38 + 1) {
        if (*(char *)(param_1 + 0x1e48 + local_10 * 10 + local_38) != '\0') {
          for (local_14 = 0; local_14 < (int)(100 / (longlong)local_30); local_14 = local_14 + 1) {
            if (local_14 == 0) {
              *(undefined4 *)(param_1 + 0x1998 + local_1c * 0xc) = 0xbfc00000;
              fVar1 = (float)(5 - local_10) * 0.3;
              fVar2 = (float)(4 - local_38) * 0.3;
              *(float *)(param_1 + 0x1994 + local_1c * 0xc) =
                   (float)fVar7 * fVar2 + (float)fVar6 * fVar1;
              *(float *)(param_1 + 0x199c + local_1c * 0xc) =
                   (float)fVar6 * fVar2 + -(float)fVar7 * fVar1;
            }
            else {
              iVar4 = _rand();
              iVar5 = _rand();
              *(float *)(param_1 + 0x1994 + local_1c * 0xc) =
                   ((((float)iVar4 - (float)iVar5) / _DAT_005a4960) / _DAT_005a365c) / 0.08 +
                   (float)(5 - local_10) * 0.3;
              iVar4 = _rand();
              iVar5 = _rand();
              *(float *)(param_1 + 0x1998 + local_1c * 0xc) =
                   ((((float)iVar4 - (float)iVar5) / _DAT_005a4960) / 0.08) * _DAT_005a40ec -
                   _DAT_005a430c;
              iVar4 = _rand();
              iVar5 = _rand();
              *(float *)(param_1 + 0x199c + local_1c * 0xc) =
                   ((((float)iVar4 - (float)iVar5) / _DAT_005a4960) / _DAT_005a365c) / 0.08 +
                   (float)(4 - local_38) * 0.3;
            }
            local_24 = local_10;
            local_2c = local_38;
            local_1c = local_1c + 1;
          }
        }
      }
    }
    for (; local_1c < 100; local_1c = local_1c + 1) {
      iVar4 = _rand();
      iVar5 = _rand();
      *(float *)(param_1 + 0x1994 + local_1c * 0xc) =
           (((float)iVar4 - (float)iVar5) / _DAT_005a4960) / 0.08 + (float)local_24 * 0.3;
      *(undefined4 *)(param_1 + 0x1998 + local_1c * 0xc) = 0xbfc00000;
      iVar4 = _rand();
      iVar5 = _rand();
      *(float *)(param_1 + 0x199c + local_1c * 0xc) =
           (((float)iVar4 - (float)iVar5) / _DAT_005a4960) / 0.08 + (float)local_2c * 0.3;
    }
    iVar4 = _rand();
    *(int *)(param_1 + 0x1e44) = iVar4 % 6;
  }
  return;
}


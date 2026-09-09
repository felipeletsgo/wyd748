// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00565621 | Name: FUN_00565621


undefined4 FUN_00565621(undefined4 param_1,undefined4 param_2)

{
  uint *puVar1;
  int iVar2;
  void *pvVar3;
  undefined1 *puVar4;
  uint uVar5;
  undefined4 *puVar6;
  undefined4 uVar7;
  undefined4 *puVar8;
  uint *puVar9;
  undefined4 *puVar10;
  code *local_278 [2];
  undefined *local_270;
  undefined1 local_1f4 [64];
  undefined4 local_1b4;
  undefined4 *local_1b0;
  undefined4 *local_1a0;
  undefined4 local_170;
  uint local_158;
  uint local_154;
  int local_14c;
  uint local_13c;
  int *local_c;
  undefined4 *local_8;
  
  local_1b4 = FUN_005732e4(local_278);
  local_278[0] = FUN_005655df;
  local_270 = &DAT_0058758f;
  iVar2 = __setjmp3(local_1f4,0);
  if (iVar2 == 0) {
    FUN_00571522(&local_1b4,0x3d,0x1a8);
    local_1a0 = (undefined4 *)(*(code *)*local_1b0)(&local_1b4,0,0x24);
    local_1a0[8] = param_2;
    local_1a0[2] = &DAT_0058758f;
    local_1a0[3] = &LAB_005655fb;
    local_1a0[4] = &LAB_00565610;
    local_1a0[5] = &LAB_0057303d;
    local_1a0[6] = &DAT_0058758f;
    local_1a0[1] = 0;
    *local_1a0 = 0;
    local_1a0[7] = param_1;
    FUN_0057189b(&local_1b4,1);
    iVar2 = FUN_00571112();
    if (iVar2 == 0) {
      local_170 = 1;
    }
    local_270 = (undefined *)0x5731ee;
    FUN_00571a2b(&local_1b4);
    if (local_14c == 1) {
      *local_c = 0x32;
      local_c[0xc] = local_158;
    }
    else {
      if (local_14c != 3) goto LAB_0056566c;
      *local_c = 0x16;
      local_c[0xc] = local_158 << 2;
    }
    local_c[0xd] = 0;
    local_c[3] = local_158;
    local_c[4] = local_154;
    local_c[5] = 1;
    if (local_c[0x10] != 0) {
      local_c[0xe] = 1;
      pvVar3 = operator_new(local_c[0xc] * local_154);
      local_c[1] = (int)pvVar3;
      if ((pvVar3 == (void *)0x0) ||
         (local_8 = (undefined4 *)(*(code *)local_1b0[2])(&local_1b4,1,local_158 * local_14c,1),
         local_8 == (undefined4 *)0x0)) {
        uVar7 = 0x8007000e;
        goto LAB_0056587b;
      }
      if (*local_c == 0x32) {
        puVar6 = (undefined4 *)local_c[1];
        if (local_13c < local_154) {
          do {
            FUN_005719a7(&local_1b4,local_8,1);
            puVar8 = (undefined4 *)*local_8;
            puVar10 = puVar6;
            for (uVar5 = local_158 >> 2; uVar5 != 0; uVar5 = uVar5 - 1) {
              *puVar10 = *puVar8;
              puVar8 = puVar8 + 1;
              puVar10 = puVar10 + 1;
            }
            for (uVar5 = local_158 & 3; uVar5 != 0; uVar5 = uVar5 - 1) {
              *(undefined1 *)puVar10 = *(undefined1 *)puVar8;
              puVar8 = (undefined4 *)((int)puVar8 + 1);
              puVar10 = (undefined4 *)((int)puVar10 + 1);
            }
            puVar6 = (undefined4 *)((int)puVar6 + local_158);
          } while (local_13c < local_154);
        }
      }
      else if (*local_c == 0x16) {
        puVar9 = (uint *)local_c[1];
        while (local_13c < local_154) {
          FUN_005719a7(&local_1b4,local_8,1);
          puVar4 = (undefined1 *)*local_8;
          puVar1 = puVar9 + local_158;
          for (; puVar9 < puVar1; puVar9 = puVar9 + 1) {
            *puVar9 = (uint)CONCAT21(CONCAT11(*puVar4,puVar4[1]),puVar4[2]);
            puVar4 = puVar4 + 3;
          }
        }
      }
      FUN_005717f4(&local_1b4);
    }
    uVar7 = 0;
  }
  else {
LAB_0056566c:
    uVar7 = 0x80004005;
  }
LAB_0056587b:
  thunk_FUN_00578e73(&local_1b4);
  return uVar7;
}


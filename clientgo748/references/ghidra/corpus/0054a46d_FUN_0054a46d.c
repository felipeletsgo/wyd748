// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0054a46d | Name: FUN_0054a46d


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __fastcall FUN_0054a46d(int *param_1)

{
  undefined4 uVar1;
  int iVar2;
  int iVar3;
  void *pvVar4;
  uint uVar5;
  float fVar6;
  float fVar7;
  int *local_60;
  undefined *local_3c [3];
  float local_30 [4];
  float local_20;
  float local_1c;
  float local_18;
  undefined4 local_14;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_005a2ae3;
  local_10 = ExceptionList;
  if ((((param_1[6] == -1) || (param_1[6] == DAT_005bdef0)) || (param_1[6] == DAT_005bdef4)) ||
     ((param_1[6] == DAT_005bdef8 || (param_1[6] == DAT_005bdefc)))) {
    ExceptionList = &local_10;
    iVar2 = (**(code **)(*DAT_0092e654 + 8))();
    if (param_1[0x18] != 0) {
      iVar3 = (**(code **)(*param_1 + 0x4c))();
      if (iVar3 == 1) {
        local_30[3] = (float)DAT_0067cf38;
        FUN_0049cb58(&local_20,param_1[10],param_1[0xb]);
        local_20 = *(float *)(DAT_005ccf9c + 0x2a6e4) * _DAT_005a414c + local_20 * _DAT_005a4154;
        local_1c = *(float *)(DAT_005ccf9c + 0x2a6e8) * _DAT_005a414c + local_1c * _DAT_005a4154;
        local_18 = *(float *)(DAT_005ccf9c + 0x2a6ec) * _DAT_005a414c + local_18 * _DAT_005a4154;
        *(undefined4 *)(param_1[0x18] + 0x298) = 0;
        *(undefined4 *)(param_1[0x18] + 0x29c) = 0;
        *(undefined4 *)(param_1[0x18] + 0x2a0) = 0;
        iVar3 = param_1[0x18];
        *(float *)(iVar3 + 0x288) = local_20;
        *(float *)(iVar3 + 0x28c) = local_1c;
        *(float *)(iVar3 + 0x290) = local_18;
        *(undefined4 *)(iVar3 + 0x294) = local_14;
        *(undefined4 *)(param_1[0x18] + 0x2a8) = 0x3f800000;
        *(undefined4 *)(param_1[0x18] + 0x2ac) = 0x3f800000;
        *(undefined4 *)(param_1[0x18] + 0x2b0) = 0x3f800000;
        *(float *)(param_1[0x18] + 0x2b8) =
             local_20 * _DAT_005a4154 + *(float *)(DAT_005ccf9c + 0x2a6f4) * _DAT_005a4154;
        *(float *)(param_1[0x18] + 700) =
             local_20 * _DAT_005a4154 + *(float *)(DAT_005ccf9c + 0x2a6f8) * _DAT_005a4154;
        *(float *)(param_1[0x18] + 0x2c0) =
             local_20 * _DAT_005a4154 + *(float *)(DAT_005ccf9c + 0x2a6fc) * _DAT_005a4154;
        if (param_1[0x17] == 0x57) {
          *(undefined4 *)(param_1[0x18] + 0x2b8) = 0x3f800000;
          *(undefined4 *)(param_1[0x18] + 700) = 0x3f666666;
          *(undefined4 *)(param_1[0x18] + 0x2c0) = 0x3f666666;
        }
        FUN_004bda22(iVar2);
        if (((0x16a < (uint)param_1[9]) && ((uint)param_1[9] < 0x170)) &&
           ((6 < *(uint *)(param_1[0x18] + 0x2e0) && (*(uint *)(param_1[0x18] + 0x2e0) < 0xc)))) {
          uVar5 = param_1[9] - 0x16bU >> 1;
          local_30[0] = 1.6;
          local_30[1] = 1.4;
          local_30[2] = 1.4;
          local_3c[0] = (undefined *)0x8822;
          local_3c[1] = &DAT_00884400;
          local_3c[2] = &DAT_00770000;
          if (1000 < (uint)(param_1[100] - iVar2)) {
            pvVar4 = operator_new(0x180);
            local_8 = 0;
            if (pvVar4 == (void *)0x0) {
              local_60 = (int *)0x0;
            }
            else {
              local_60 = (int *)FUN_004c8f70(0,0x5dc,0x3c23d70a,0x3dcccccd,0x3c23d70a,0x3a83126f,1,
                                             0x50);
            }
            local_8 = 0xffffffff;
            if (local_60 != (int *)0x0) {
              iVar3 = _rand();
              fVar7 = (float)(iVar3 % 10 + -5) * _DAT_005a439c + (float)param_1[0xb];
              fVar6 = (float)param_1[0xc] + local_30[uVar5];
              iVar3 = _rand();
              uVar1 = FUN_00430f50((float)(iVar3 % 10 + -5) * _DAT_005a439c + (float)param_1[10],
                                   fVar6,fVar7);
              uVar1 = FUN_004310a0(uVar1);
              FUN_004310a0(uVar1);
              local_60[0x1b] = 1;
              local_60[0x52] = 1;
              local_60[0x50] = 0x40000000;
              (**(code **)(*local_60 + 0x3c))(local_3c[uVar5]);
              FUN_0054ac09(local_60);
            }
            param_1[100] = iVar2;
          }
        }
      }
    }
    uVar1 = 1;
  }
  else {
    uVar1 = 0;
  }
  ExceptionList = local_10;
  return uVar1;
}


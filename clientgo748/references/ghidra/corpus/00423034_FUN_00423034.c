// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00423034 | Name: FUN_00423034


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __fastcall FUN_00423034(int param_1)

{
  float fVar1;
  float fVar2;
  float fVar3;
  float fVar4;
  float fVar5;
  undefined4 uVar6;
  int iVar7;
  float local_120;
  float local_11c;
  int local_114;
  char local_110 [256];
  float local_10;
  int local_c;
  int local_8;
  
  if ((*(int *)(param_1 + 0x28c) == 1) && (*(int *)(DAT_013b71e8 + 0x1e428) == 0)) {
    if (*(int *)(param_1 + 0xac) == 0) {
      (**(code **)(*DAT_005ccf9c + 0x40))(2);
      FUN_0042261f(*(undefined4 *)(param_1 + 0xb8),*(undefined4 *)(param_1 + 0xbc),1);
      (**(code **)(*DAT_005ccf9c + 0x40))(1);
    }
    else {
      local_c = FUN_004b8c45(*(undefined4 *)(param_1 + 0xb0));
      local_8 = 0;
      local_10 = (float)*(int *)(param_1 + 0x24) * _DAT_005a34a0 - _DAT_005a3660;
      if ((local_c != 0) && (*(int *)(local_c + 4) != 0)) {
        (**(code **)(*DAT_005ccf9c + 0x30))(0x1c,0);
        (**(code **)(*DAT_005ccf9c + 0x30))(7,0);
        uVar6 = FUN_0042260e();
        FUN_0058ee20(local_110,uVar6);
        for (local_114 = 0; local_114 < *(int *)(param_1 + 0x24); local_114 = local_114 + 1) {
          if ((*(int *)(param_1 + 0x30 + local_114 * 0xc) != -1) &&
             (local_8 = FUN_004b87be(*(undefined4 *)
                                      (*(int *)(local_c + 4) +
                                      *(int *)(param_1 + 0x30 + local_114 * 0xc) * 0x1c),20000),
             local_8 != 0)) {
            fVar1 = (float)*(int *)(*(int *)(local_c + 4) + 4 +
                                   *(int *)(param_1 + 0x30 + local_114 * 0xc) * 0x1c);
            fVar2 = (float)*(int *)(*(int *)(local_c + 4) + 8 +
                                   *(int *)(param_1 + 0x30 + local_114 * 0xc) * 0x1c);
            fVar3 = (float)*(int *)(*(int *)(local_c + 4) + 0xc +
                                   *(int *)(param_1 + 0x30 + local_114 * 0xc) * 0x1c);
            fVar4 = (float)*(int *)(*(int *)(local_c + 4) + 0x10 +
                                   *(int *)(param_1 + 0x30 + local_114 * 0xc) * 0x1c);
            if (local_10 == _DAT_005a3534) {
              local_120 = 0.0;
            }
            else {
              local_120 = ((float)local_114 - local_10) / local_10;
            }
            if ((*(int *)(param_1 + 0xac) == 5) || (*(int *)(param_1 + 0xac) == 6)) {
              local_11c = (_DAT_005a3f2c - *(float *)(param_1 + 0x298)) *
                          -local_120 * fVar3 * local_10 * _DAT_005a3660 +
                          (float)(*(int *)(*(int *)(local_c + 4) + 0x14 +
                                          *(int *)(param_1 + 0x30 + local_114 * 0xc) * 0x1c) +
                                 *(int *)(param_1 + 0x28 + local_114 * 0xc)) +
                          (float)*(int *)(param_1 + 0xb8);
            }
            else {
              local_11c = (_DAT_005a4144 - *(float *)(param_1 + 0x298)) *
                          -local_120 * fVar3 * local_10 * _DAT_005a40ec +
                          (float)(*(int *)(*(int *)(local_c + 4) + 0x14 +
                                          *(int *)(param_1 + 0x30 + local_114 * 0xc) * 0x1c) +
                                 *(int *)(param_1 + 0x28 + local_114 * 0xc)) +
                          (float)*(int *)(param_1 + 0xb8);
            }
            fVar5 = (float)(*(int *)(*(int *)(local_c + 4) + 0x18 +
                                    *(int *)(param_1 + 0x30 + local_114 * 0xc) * 0x1c) +
                           *(int *)(param_1 + 0x2c + local_114 * 0xc)) +
                    (float)*(int *)(param_1 + 0xbc);
            iVar7 = __ftol();
            if ((local_110[local_114] == '1') &&
               ((*(int *)(param_1 + 0xac) != 5 || (*(int *)(param_1 + 0xac) != 6)))) {
              FUN_004306ce(fVar1,fVar2,fVar3,fVar4,local_11c - _DAT_005a40f0 * DAT_005b4910,fVar5,
                           fVar3,fVar4,local_8,iVar7 << 0x18 | 0xffffff,1,0,
                           DAT_005b490c * *(float *)(param_1 + 0x298));
            }
            else {
              FUN_004306ce(fVar1,fVar2,fVar3,fVar4,local_11c,fVar5,fVar3,fVar4,local_8,
                           iVar7 << 0x18 | 0xffffff,1,0,DAT_005b490c * *(float *)(param_1 + 0x298));
            }
          }
        }
        (**(code **)(*DAT_005ccf9c + 0x30))(7,1);
      }
    }
  }
  return 1;
}


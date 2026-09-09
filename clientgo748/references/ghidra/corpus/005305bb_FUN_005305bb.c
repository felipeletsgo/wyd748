// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 005305bb | Name: FUN_005305bb


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __fastcall FUN_005305bb(int *param_1)

{
  float fVar1;
  float fVar2;
  float fVar3;
  undefined4 uVar4;
  int iVar5;
  uint uVar6;
  int iVar7;
  float10 fVar8;
  float10 fVar9;
  float10 fVar10;
  undefined4 uVar11;
  float local_64;
  undefined4 local_34;
  float local_30;
  
  if ((((param_1[6] == -1) || (param_1[6] == DAT_005bdef0)) || (param_1[6] == DAT_005bdef4)) ||
     ((param_1[6] == DAT_005bdef8 || (param_1[6] == DAT_005bdefc)))) {
    iVar5 = (**(code **)(*DAT_0092e654 + 8))();
    if (param_1[0x18] != 0) {
      fVar8 = (float10)FUN_00423b00(((float)((uint)(iVar5 - param_1[0x6c]) % 20000) / _DAT_005a5770)
                                    * _DAT_005a4148);
      if (fVar8 <= (float10)_DAT_005a3534) {
        fVar8 = (float10)FUN_00423b00(((float)((uint)(iVar5 - param_1[0x6c]) % 20000) /
                                      _DAT_005a5770) * _DAT_005a4148);
        fVar8 = -fVar8;
      }
      else {
        fVar8 = (float10)FUN_00423b00(((float)((uint)(iVar5 - param_1[0x6c]) % 20000) /
                                      _DAT_005a5770) * _DAT_005a4148);
      }
      local_64 = (float)fVar8;
      if (param_1[100] == 0) {
        fVar8 = (float10)FUN_00423b00(local_64 * _DAT_005a4148 * (float)param_1[0x66]);
        fVar9 = (float10)FUN_00423b00(local_64 * _DAT_005a3794 * _DAT_005a4148 *
                                      (float)param_1[0x66]);
        if (param_1[7] == 0) {
          param_1[0xc] = (int)((float)fVar9 * (float)param_1[0x67] + (float)param_1[0x6a]);
          param_1[10] = (int)(local_64 * (float)param_1[0x68] * _DAT_005a34a0 + (float)param_1[0x69]
                             );
          param_1[0xb] = (int)((float)fVar8 * (float)param_1[0x68] * _DAT_005a34a0 +
                              (float)param_1[0x6b]);
        }
        else {
          param_1[0xc] = (int)((float)fVar9 * (float)param_1[0x67] + *(float *)(param_1[7] + 0x30) +
                              _DAT_005a430c);
          param_1[10] = (int)(local_64 * (float)param_1[0x68] * _DAT_005a34a0 +
                             *(float *)(param_1[7] + 0x28));
          param_1[0xb] = (int)((float)fVar8 * (float)param_1[0x68] * _DAT_005a34a0 +
                              *(float *)(param_1[7] + 0x2c));
        }
      }
      else if (param_1[100] == 1) {
        fVar8 = (float10)FUN_00493bf0(local_64 * _DAT_005a4148 * (float)param_1[0x66]);
        fVar9 = (float10)FUN_00493bf0(local_64 * _DAT_005a3794 * _DAT_005a4148 *
                                      (float)param_1[0x66]);
        if (param_1[7] == 0) {
          param_1[0xc] = (int)((float)fVar9 * (float)param_1[0x67] + (float)param_1[0x6a]);
          param_1[10] = (int)((float)fVar8 * (float)param_1[0x68] * _DAT_005a34a0 +
                             (float)param_1[0x69]);
          param_1[0xb] = (int)(local_64 * (float)param_1[0x68] * _DAT_005a34a0 +
                              (float)param_1[0x6b]);
        }
        else {
          param_1[0xc] = (int)((float)fVar9 * (float)param_1[0x67] + *(float *)(param_1[7] + 0x30) +
                              _DAT_005a430c);
          param_1[10] = (int)((float)fVar8 * (float)param_1[0x68] * _DAT_005a34a0 +
                             *(float *)(param_1[7] + 0x28));
          param_1[0xb] = (int)(local_64 * (float)param_1[0x68] * _DAT_005a34a0 +
                              *(float *)(param_1[7] + 0x2c));
        }
      }
      else if (param_1[100] == 2) {
        fVar8 = (float10)FUN_00423b00((local_64 + local_64) * _DAT_005a4148);
        fVar9 = (float10)FUN_00493bf0((local_64 + local_64) * _DAT_005a4148);
        fVar10 = (float10)FUN_00493bf0(local_64 * _DAT_005a3794 * _DAT_005a4148 *
                                       (float)param_1[0x66]);
        if (param_1[7] == 0) {
          param_1[0xc] = (int)((float)fVar10 * (float)param_1[0x67] + (float)param_1[0x6a]);
          param_1[10] = (int)(local_64 * (float)fVar9 * (float)param_1[0x68] * _DAT_005a34a0 +
                             (float)param_1[0x69]);
          param_1[0xb] = (int)(local_64 * (float)fVar8 * (float)param_1[0x68] * _DAT_005a34a0 +
                              (float)param_1[0x6b]);
        }
        else {
          param_1[0xc] = (int)((float)fVar10 * (float)param_1[0x67] + *(float *)(param_1[7] + 0x30)
                              + _DAT_005a365c);
          param_1[10] = (int)(local_64 * (float)fVar9 * (float)param_1[0x68] * _DAT_005a34a0 +
                             *(float *)(param_1[7] + 0x28));
          param_1[0xb] = (int)(local_64 * (float)fVar8 * (float)param_1[0x68] * _DAT_005a34a0 +
                              *(float *)(param_1[7] + 0x2c));
        }
      }
      else if (param_1[100] == 3) {
        fVar2 = (float)((uint)(iVar5 - param_1[0x6c]) % 7000) / _DAT_005a4968;
        local_30 = 1.0;
        if (fVar2 < _DAT_005a45d4) {
          uVar6 = _rand();
          uVar6 = uVar6 & 0x80000001;
          if ((int)uVar6 < 0) {
            uVar6 = (uVar6 - 1 | 0xfffffffe) + 1;
          }
          if (uVar6 == 0) {
            local_30 = -1.0;
          }
        }
        fVar8 = (float10)FUN_00423b00((local_30 * fVar2 + local_30 * fVar2) * _DAT_005a4148);
        fVar9 = (float10)FUN_00493bf0((local_30 * fVar2 + local_30 * fVar2) * _DAT_005a4148);
        fVar1 = (float)fVar9;
        if (param_1[7] == 0) {
          param_1[0xc] = (int)(fVar1 * (float)param_1[0x67] * _DAT_005a34a0 + (float)param_1[0x6a]);
          param_1[10] = (int)(fVar1 * (float)param_1[0x68] * _DAT_005a34a0 + (float)param_1[0x69]);
          param_1[0xb] = (int)((float)fVar8 * (float)param_1[0x68] * _DAT_005a34a0 +
                              (float)param_1[0x6b]);
        }
        else {
          param_1[0xc] = (int)(fVar1 * (float)param_1[0x67] + *(float *)(param_1[7] + 0x30) +
                              _DAT_005a365c);
          param_1[10] = (int)(fVar2 * fVar1 * (float)param_1[0x68] * _DAT_005a34a0 +
                             *(float *)(param_1[7] + 0x28));
          param_1[0xb] = (int)(fVar2 * (float)fVar8 * (float)param_1[0x68] * _DAT_005a34a0 +
                              *(float *)(param_1[7] + 0x2c));
        }
      }
      (**(code **)(*param_1 + 0x44))(param_1[10],param_1[0xc],param_1[0xb]);
      iVar7 = (**(code **)(*param_1 + 0x4c))();
      if (iVar7 == 1) {
        fVar8 = (float10)FUN_004d0150(param_1 + 10);
        if ((((float)fVar8 < _DAT_005a365c) && (param_1[0x17] == 0x18)) &&
           ((DAT_005ccf98 != 0 && (iVar7 = FUN_00429a6d(5), iVar7 != 0)))) {
          FUN_00429a6d(5);
          iVar7 = FUN_0042afd0();
          if (iVar7 == 0) {
            uVar11 = 0;
            uVar4 = 0;
            FUN_00429a6d(5);
            FUN_0042ad2b(uVar4,uVar11);
          }
        }
        fVar2 = *(float *)(DAT_005ccf9c + 0x2a6e4) * _DAT_005a40ec + _DAT_005a4160;
        fVar1 = *(float *)(DAT_005ccf9c + 0x2a6e8) * _DAT_005a40ec + _DAT_005a4160;
        fVar3 = *(float *)(DAT_005ccf9c + 0x2a6ec) * _DAT_005a40ec + _DAT_005a4160;
        *(undefined4 *)(param_1[0x18] + 0x298) = 0;
        *(undefined4 *)(param_1[0x18] + 0x29c) = 0;
        *(undefined4 *)(param_1[0x18] + 0x2a0) = 0;
        iVar7 = param_1[0x18];
        *(float *)(iVar7 + 0x288) = fVar2;
        *(float *)(iVar7 + 0x28c) = fVar1;
        *(float *)(iVar7 + 0x290) = fVar3;
        *(undefined4 *)(iVar7 + 0x294) = local_34;
        *(undefined4 *)(param_1[0x18] + 0x2a8) = 0x3f800000;
        *(undefined4 *)(param_1[0x18] + 0x2ac) = 0x3f800000;
        *(undefined4 *)(param_1[0x18] + 0x2b0) = 0x3f800000;
        *(float *)(param_1[0x18] + 0x2b8) =
             fVar2 * _DAT_005a4154 + *(float *)(DAT_005ccf9c + 0x2a6f4) * _DAT_005a4154;
        *(float *)(param_1[0x18] + 700) =
             fVar2 * _DAT_005a4154 + *(float *)(DAT_005ccf9c + 0x2a6f8) * _DAT_005a4154;
        *(float *)(param_1[0x18] + 0x2c0) =
             fVar2 * _DAT_005a4154 + *(float *)(DAT_005ccf9c + 0x2a6fc) * _DAT_005a4154;
        iVar7 = __ftol();
        FUN_004bda22(iVar5 + iVar7);
      }
    }
    uVar4 = 1;
  }
  else {
    param_1[0x12] = 0;
    uVar4 = 0;
  }
  return uVar4;
}


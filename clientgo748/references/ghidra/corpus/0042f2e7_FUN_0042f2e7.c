// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0042f2e7 | Name: FUN_0042f2e7


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __thiscall FUN_0042f2e7(int *param_1,int *param_2)

{
  float fVar1;
  float fVar2;
  int iVar3;
  int iVar4;
  DWORD DVar5;
  float local_70;
  float local_6c;
  float local_68;
  float local_5c;
  float local_50;
  
  if (param_2[0xf] < 0) {
    if (param_2[0xf] < -2) {
      iVar3 = FUN_004b8c45(-param_2[0xf]);
      (**(code **)(*param_1 + 0x34))(0,0x11,2);
      (**(code **)(*param_1 + 0x34))(0,0x10,2);
      (**(code **)(*param_1 + 0x34))(0,0x12,0);
      if (((iVar3 != 0) && (*(int *)(iVar3 + 4) != 0)) &&
         (iVar4 = FUN_004b87be(*(undefined4 *)(*(int *)(iVar3 + 4) + param_2[0x10] * 0x1c),20000),
         iVar4 != 0)) {
        FUN_00430224((float)*(int *)(*(int *)(iVar3 + 4) + 4 + param_2[0x10] * 0x1c),
                     (float)*(int *)(*(int *)(iVar3 + 4) + 8 + param_2[0x10] * 0x1c),
                     (float)*(int *)(*(int *)(iVar3 + 4) + 0xc + param_2[0x10] * 0x1c),
                     (float)*(int *)(*(int *)(iVar3 + 4) + 0x10 + param_2[0x10] * 0x1c),
                     (float)*(int *)(*(int *)(iVar3 + 4) + 0x14 + param_2[0x10] * 0x1c) +
                     (float)param_2[1],
                     (float)*(int *)(*(int *)(iVar3 + 4) + 0x18 + param_2[0x10] * 0x1c) +
                     (float)param_2[2],param_2[3],param_2[4],iVar4,param_2[0xc],1,param_2[6],
                     0x3f800000);
      }
    }
    else {
      iVar3 = param_2[0x58];
      if ((iVar3 < 0) || (0x40 < iVar3)) {
        FUN_0042ffd4(param_2[1],param_2[2],param_2[3],param_2[4],param_2[0xc],param_2[0xf] == -2);
      }
      else if (*(int *)(DAT_0092e640 + 4 + iVar3 * 0xc) != 0) {
        local_50 = 1.0;
        local_70 = (float)param_2[1] - 2.0;
        local_5c = (float)param_2[2] - 2.0;
        local_68 = _DAT_005a4204 + 2.0;
        local_6c = _DAT_005a3658 + 2.0;
        if (DAT_005b892c != 2) {
          local_50 = DAT_005b490c;
          if (DAT_005b490c != 1.0) {
            local_68 = DAT_005b490c * _DAT_005a4200;
            local_6c = DAT_005b4910 * _DAT_005a41fc;
          }
          if (DAT_005b4910 == 0.8) {
            local_70 = local_70 - _DAT_005a3534;
            local_5c = local_5c + _DAT_005a3660;
          }
          else if (DAT_005b4910 == 1.0) {
            local_70 = local_70 + _DAT_005a3534;
            local_5c = local_5c + _DAT_005a3534;
          }
          else if (DAT_005b490c == 1.6) {
            local_70 = local_70 - _DAT_005a3660;
            local_5c = local_5c + _DAT_005a3534;
          }
          else if (DAT_005b490c == 2.0) {
            local_70 = local_70 - _DAT_005a3660;
            local_5c = local_5c + _DAT_005a3534;
          }
        }
        if (param_2[0x59] == 1) {
          FUN_0042ffd4(local_70,local_5c,local_68,local_6c,0xffffd700,1);
        }
        else if (param_2[0x59] == 2) {
          FUN_0042ffd4(local_70,local_5c,local_68,local_6c,0xffc0c0c0,1);
        }
        FUN_0042fae6(0,0,0x41800000,0x41400000,(float)param_2[1] - (local_50 - _DAT_005a415c),
                     param_2[2],*(undefined4 *)(DAT_0092e640 + 4 + iVar3 * 0xc),0x3f800000,
                     0x3f800000);
        DVar5 = timeGetTime();
        *(DWORD *)(DAT_0092e640 + 0xc + iVar3 * 0xc) = DVar5;
      }
    }
  }
  else {
    iVar3 = FUN_004b8c45(param_2[0xf]);
    if (((iVar3 != 0) && (*(int *)(iVar3 + 4) != 0)) &&
       (iVar4 = FUN_004b87be(*(undefined4 *)(*(int *)(iVar3 + 4) + param_2[0x10] * 0x1c),20000),
       iVar4 != 0)) {
      if (*param_2 == 2) {
        FUN_0042fae6((float)*(int *)(*(int *)(iVar3 + 4) + 4 + param_2[0x10] * 0x1c),
                     (float)*(int *)(*(int *)(iVar3 + 4) + 8 + param_2[0x10] * 0x1c),
                     (float)*(int *)(*(int *)(iVar3 + 4) + 0xc + param_2[0x10] * 0x1c),
                     (float)*(int *)(*(int *)(iVar3 + 4) + 0x10 + param_2[0x10] * 0x1c),
                     (float)*(int *)(*(int *)(iVar3 + 4) + 0x14 + param_2[0x10] * 0x1c) +
                     (float)param_2[1],
                     (float)*(int *)(*(int *)(iVar3 + 4) + 0x18 + param_2[0x10] * 0x1c) +
                     (float)param_2[2],iVar4,0x3f800000,0x3f800000);
      }
      else if (*param_2 == 4) {
        fVar1 = (float)param_2[3] /
                (float)*(int *)(*(int *)(iVar3 + 4) + 0xc + param_2[0x10] * 0x1c);
        fVar2 = (float)param_2[4] /
                (float)*(int *)(*(int *)(iVar3 + 4) + 0x10 + param_2[0x10] * 0x1c);
        if (param_2[0x5a] == 1) {
          FUN_0042fbb1((float)*(int *)(*(int *)(iVar3 + 4) + 4 + param_2[0x10] * 0x1c),
                       (float)*(int *)(*(int *)(iVar3 + 4) + 8 + param_2[0x10] * 0x1c) +
                       (float)param_2[9],
                       (float)*(int *)(*(int *)(iVar3 + 4) + 0xc + param_2[0x10] * 0x1c),
                       (float)param_2[0xb] - (float)param_2[9],
                       (float)*(int *)(*(int *)(iVar3 + 4) + 0x14 + param_2[0x10] * 0x1c) +
                       (float)param_2[1],
                       (float)*(int *)(*(int *)(iVar3 + 4) + 0x18 + param_2[0x10] * 0x1c) +
                       (float)param_2[2] + (float)param_2[9],(float)param_2[3] / _DAT_005a365c,
                       (float)param_2[4] / _DAT_005a365c,param_2[6],iVar4,fVar1,fVar2);
        }
        else {
          FUN_0042fbb1((float)*(int *)(*(int *)(iVar3 + 4) + 4 + param_2[0x10] * 0x1c),
                       (float)*(int *)(*(int *)(iVar3 + 4) + 8 + param_2[0x10] * 0x1c),
                       (float)*(int *)(*(int *)(iVar3 + 4) + 0xc + param_2[0x10] * 0x1c),
                       (float)*(int *)(*(int *)(iVar3 + 4) + 0x10 + param_2[0x10] * 0x1c),
                       (float)*(int *)(*(int *)(iVar3 + 4) + 0x14 + param_2[0x10] * 0x1c) +
                       (float)param_2[1],
                       (float)*(int *)(*(int *)(iVar3 + 4) + 0x18 + param_2[0x10] * 0x1c) +
                       (float)param_2[2],(float)param_2[3] / _DAT_005a365c,
                       (float)param_2[4] / _DAT_005a365c,param_2[6],iVar4,fVar1,fVar2);
        }
      }
      else if (*param_2 == 3) {
        FUN_0042fe6c((float)*(int *)(*(int *)(iVar3 + 4) + 0x14 + param_2[0x10] * 0x1c) +
                     (float)param_2[1],
                     (float)*(int *)(*(int *)(iVar3 + 4) + 0x18 + param_2[0x10] * 0x1c) +
                     (float)param_2[2],param_2[3],param_2[4],iVar4,param_2[0xc],
                     (float)param_2[3] /
                     (float)*(int *)(*(int *)(iVar3 + 4) + 0xc + param_2[0x10] * 0x1c),
                     (float)param_2[4] /
                     (float)*(int *)(*(int *)(iVar3 + 4) + 0x10 + param_2[0x10] * 0x1c));
      }
    }
  }
  return;
}


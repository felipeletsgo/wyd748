// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00598533 | Name: FUN_00598533


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

bool FUN_00598533(uint param_1,double *param_2,uint param_3)

{
  double dVar1;
  bool bVar2;
  uint uVar3;
  bool bVar4;
  float10 fVar5;
  undefined8 local_10;
  int local_8;
  
  uVar3 = param_1 & 0x1f;
  bVar2 = true;
  if (((param_1 & 8) != 0) && ((param_3 & 1) != 0)) {
    FUN_005989d0(1);
    uVar3 = param_1 & 0x17;
    goto LAB_00598728;
  }
  if (((param_1 & 4) != 0) && ((param_3 & 4) != 0)) {
    FUN_005989d0(4);
    uVar3 = param_1 & 0x1b;
    goto LAB_00598728;
  }
  if (((param_1 & 1) == 0) || ((param_3 & 8) == 0)) {
    if (((param_1 & 2) != 0) && ((param_3 & 0x10) != 0)) {
      bVar4 = (param_1 & 0x10) != 0;
      if (*param_2 != _DAT_005aa050) {
        fVar5 = (float10)FUN_005988cf(*param_2,&local_8);
        local_8 = local_8 + -0x600;
        if (local_8 < -0x432) {
          local_10 = 0.0;
          bVar4 = bVar2;
        }
        else {
          local_10 = (double)(ulonglong)
                             (SUB87((double)fVar5,0) & 0xfffffffffffff | 0x10000000000000);
          if (local_8 < -0x3fd) {
            local_8 = -0x3fd - local_8;
            do {
              if ((((ulonglong)local_10 & 1) != 0) && (!bVar4)) {
                bVar4 = bVar2;
              }
              uVar3 = (uint)local_10 >> 1;
              if (((ulonglong)local_10 & 0x100000000) != 0) {
                local_10._3_1_ = (byte)((ulonglong)local_10 >> 0x18) >> 1;
                local_10._0_3_ = (undefined3)uVar3;
                local_10._0_4_ = CONCAT13(local_10._3_1_,(undefined3)local_10) | 0x80000000;
                uVar3 = (uint)local_10;
              }
              local_10._0_4_ = uVar3;
              local_10 = (double)CONCAT44(local_10._4_4_ >> 1,(uint)local_10);
              local_8 = local_8 + -1;
            } while (local_8 != 0);
          }
          if ((double)fVar5 < _DAT_005aa050) {
            local_10 = -local_10;
          }
        }
        *param_2 = local_10;
        bVar2 = bVar4;
      }
      if (bVar2) {
        FUN_005989d0(0x10);
      }
      uVar3 = param_1 & 0x1d;
    }
    goto LAB_00598728;
  }
  FUN_005989d0(8);
  uVar3 = param_3 & 0xc00;
  dVar1 = _DAT_005cc408;
  if (uVar3 == 0) {
    if (*param_2 <= _DAT_005aa050) {
      dVar1 = -_DAT_005cc408;
    }
LAB_00598648:
    *param_2 = dVar1;
  }
  else {
    if (uVar3 == 0x400) {
      dVar1 = _DAT_005cc418;
      if (*param_2 <= _DAT_005aa050) {
        dVar1 = -_DAT_005cc408;
      }
      goto LAB_00598648;
    }
    if (uVar3 == 0x800) {
      if (*param_2 <= _DAT_005aa050) {
        dVar1 = -_DAT_005cc418;
      }
      goto LAB_00598648;
    }
    if (uVar3 == 0xc00) {
      dVar1 = _DAT_005cc418;
      if (*param_2 <= _DAT_005aa050) {
        dVar1 = -_DAT_005cc418;
      }
      goto LAB_00598648;
    }
  }
  uVar3 = param_1 & 0x1e;
LAB_00598728:
  if (((param_1 & 0x10) != 0) && ((param_3 & 0x20) != 0)) {
    FUN_005989d0(0x20);
    uVar3 = uVar3 & 0xffffffef;
  }
  return uVar3 == 0;
}


// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0053aebf | Name: FUN_0053aebf


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __fastcall FUN_0053aebf(int param_1)

{
  int iVar1;
  int *piVar2;
  undefined4 uVar3;
  int *piVar4;
  float local_a4;
  float local_a0;
  float local_9c;
  float local_98;
  undefined4 local_64;
  undefined4 local_60;
  undefined4 local_5c;
  undefined4 local_58;
  undefined4 local_54;
  undefined4 local_50;
  undefined4 local_4c;
  undefined4 local_48;
  undefined4 local_44;
  undefined4 local_40;
  undefined4 local_3c;
  undefined4 local_38;
  undefined4 local_34;
  undefined4 local_30;
  undefined4 local_2c;
  undefined4 local_28;
  int *local_20;
  int *local_1c;
  int local_18;
  int local_14;
  int *local_10;
  int local_c;
  int local_8;
  
  if (DAT_005d03f0 == 1) {
    return 0;
  }
  (**(code **)(*DAT_005ccf9c + 0x30))(0x1b,0);
  if (*(char *)(param_1 + 0x161) == '\v') {
    local_c = *(int *)(DAT_013b71e8 + 0x1b084);
    if (*(int *)(local_c + 0x54) == 0) {
      return 1;
    }
    local_8 = 0;
    if (*(float *)(*(int *)(local_c + 0x54) + 0x28) - *(float *)(param_1 + 0x28) <= _DAT_005a3534) {
      local_98 = -(*(float *)(*(int *)(local_c + 0x54) + 0x28) - *(float *)(param_1 + 0x28));
    }
    else {
      local_98 = *(float *)(*(int *)(local_c + 0x54) + 0x28) - *(float *)(param_1 + 0x28);
    }
    if (local_98 < _DAT_005a3794) {
      if (*(float *)(*(int *)(local_c + 0x54) + 0x2c) - *(float *)(param_1 + 0x2c) <= _DAT_005a3534)
      {
        local_9c = -(*(float *)(*(int *)(local_c + 0x54) + 0x2c) - *(float *)(param_1 + 0x2c));
      }
      else {
        local_9c = *(float *)(*(int *)(local_c + 0x54) + 0x2c) - *(float *)(param_1 + 0x2c);
      }
      if (local_9c < _DAT_005a3794) {
        local_8 = 1;
        (**(code **)(*DAT_005ccf9c + 0x30))(0x1b,1);
        (**(code **)(*DAT_005ccf9c + 0x30))(0x14,9);
      }
    }
    local_10 = (int *)FUN_004b14fd(*(undefined4 *)(param_1 + 0x24),0,180000);
    if (local_10 == (int *)0x0) {
      return 0;
    }
    (**(code **)(*local_10 + 8))
              (*(undefined4 *)(param_1 + 0x28),*(undefined4 *)(param_1 + 0x30),
               *(undefined4 *)(param_1 + 0x2c),*(undefined4 *)(param_1 + 0x34),0,0,0,0);
    if (local_8 == 1) {
      (**(code **)(*DAT_005ccf9c + 0x30))(0x14,6);
    }
  }
  else {
    FUN_0053fa66();
  }
  if (*(int *)(param_1 + 0x48) == 1) {
    if (*(char *)(param_1 + 0x161) == '\0') {
      local_14 = *(int *)(DAT_013b71e8 + 0x1b084);
      if (*(int *)(local_14 + 0x54) == 0) {
        return 1;
      }
      if (*(char *)(param_1 + 0x160) == '\0') {
        local_18 = 0;
        if (*(float *)(*(int *)(local_14 + 0x54) + 0x28) - *(float *)(param_1 + 0x28) <=
            _DAT_005a3534) {
          local_a0 = -(*(float *)(*(int *)(local_14 + 0x54) + 0x28) - *(float *)(param_1 + 0x28));
        }
        else {
          local_a0 = *(float *)(*(int *)(local_14 + 0x54) + 0x28) - *(float *)(param_1 + 0x28);
        }
        if (local_a0 < _DAT_005a3794) {
          if (*(float *)(*(int *)(local_14 + 0x54) + 0x2c) - *(float *)(param_1 + 0x2c) <=
              _DAT_005a3534) {
            local_a4 = -(*(float *)(*(int *)(local_14 + 0x54) + 0x2c) - *(float *)(param_1 + 0x2c));
          }
          else {
            local_a4 = *(float *)(*(int *)(local_14 + 0x54) + 0x2c) - *(float *)(param_1 + 0x2c);
          }
          if (local_a4 < _DAT_005a3794) {
            local_18 = 1;
            (**(code **)(*DAT_005ccf9c + 0x30))(0x1b,1);
            (**(code **)(*DAT_005ccf9c + 0x30))(0x14,9);
          }
        }
        local_1c = (int *)FUN_004b14fd(*(int *)(param_1 + 0x24) + 1,0,180000);
        if (local_1c == (int *)0x0) {
          return 0;
        }
        (**(code **)(*local_1c + 8))
                  (*(undefined4 *)(param_1 + 0x28),*(undefined4 *)(param_1 + 0x30),
                   *(undefined4 *)(param_1 + 0x2c),*(undefined4 *)(param_1 + 0x34),0,0,0,0);
        if (local_18 == 1) {
          (**(code **)(*DAT_005ccf9c + 0x30))(0x14,6);
        }
      }
    }
    else if (*(char *)(param_1 + 0x161) == '\x01') {
      local_20 = (int *)FUN_004b14fd(*(int *)(param_1 + 0x24) + 1,0,180000);
      if (local_20 == (int *)0x0) {
        return 0;
      }
      (**(code **)(*local_20 + 8))
                (*(undefined4 *)(param_1 + 0x28),*(float *)(param_1 + 0x30) + _DAT_005a5940,
                 *(undefined4 *)(param_1 + 0x2c),*(float *)(param_1 + 0x34) - _DAT_005a4380,
                 *(undefined4 *)(param_1 + 0x168),0x3fc90fdb,0,0);
    }
    else if ((*(char *)(param_1 + 0x161) == '\x02') || (*(char *)(param_1 + 0x161) == '\x03')) {
      (**(code **)(*DAT_005ccf9c + 0x30))(0x1b,1);
      _memset(&local_64,0,0x44);
      local_54 = 0x3f000000;
      local_50 = 0x3f000000;
      local_4c = 0x3f000000;
      local_64 = 0x3f000000;
      local_60 = 0x3f000000;
      local_5c = 0x3f000000;
      local_44 = 0x3f000000;
      local_40 = 0x3f000000;
      local_3c = 0x3f000000;
      local_38 = local_58;
      local_34 = 0;
      local_30 = 0;
      local_2c = 0;
      local_28 = 0;
      local_48 = 0;
      local_58 = 0x3f000000;
      (**(code **)(*(int *)DAT_005ccf9c[0xa8f9] + 0xa8))(DAT_005ccf9c[0xa8f9],&local_64);
      piVar2 = (int *)FUN_004b14fd((int)*(short *)(param_1 + 0x162),0,180000);
      if (piVar2 == (int *)0x0) {
        return 0;
      }
      if (((DAT_005ccfa0 == 0) || (DAT_005ccfa0 == 3)) || (DAT_005ccfa0 == 4)) {
        if (DAT_005ccf9c[0xa8e4] == 1) {
          (**(code **)(*DAT_005ccf9c + 0x30))(0x13,3);
          (**(code **)(*DAT_005ccf9c + 0x30))(0x14,7);
          (**(code **)(*DAT_005ccf9c + 0x34))(0,1,4);
          (**(code **)(*DAT_005ccf9c + 0x34))(1,1,7);
          (**(code **)(*DAT_005ccf9c + 0x30))(0x16,1);
        }
        else {
          (**(code **)(*DAT_005ccf9c + 0x30))(0x13,3);
          (**(code **)(*DAT_005ccf9c + 0x30))(0x14,7);
          (**(code **)(*DAT_005ccf9c + 0x34))(0,1,4);
          (**(code **)(*DAT_005ccf9c + 0x34))(1,1,5);
          (**(code **)(*DAT_005ccf9c + 0x30))(0x89,1);
          (**(code **)(*DAT_005ccf9c + 0x30))(0x16,1);
        }
        uVar3 = FUN_004b9a4b(2,180000);
        (**(code **)(*DAT_005ccf9c + 0x38))(1,uVar3);
        piVar2[0xa6] = 2;
        (**(code **)(*piVar2 + 8))
                  (*(undefined4 *)(param_1 + 0x28),*(undefined4 *)(param_1 + 0x30),
                   *(undefined4 *)(param_1 + 0x2c),*(undefined4 *)(param_1 + 0x34),0,0,4,0);
      }
      else {
        (**(code **)(*DAT_005ccf9c + 0x30))(0x89,1);
        (**(code **)(*DAT_005ccf9c + 0x30))(0x13,5);
        (**(code **)(*DAT_005ccf9c + 0x30))(0x14,6);
        (**(code **)(*DAT_005ccf9c + 0x34))(0,1,4);
        (**(code **)(*DAT_005ccf9c + 0x34))(1,1,1);
        (**(code **)(*DAT_005ccf9c + 0x30))(0x19,8);
        (**(code **)(*DAT_005ccf9c + 0x30))(0xf,0);
        (**(code **)(*DAT_005ccf9c + 0x30))(0xe,0);
        (**(code **)(*DAT_005ccf9c + 0x30))(0x16,1);
        uVar3 = FUN_004b9a4b(9,180000);
        (**(code **)(*DAT_005ccf9c + 0x38))(1,uVar3);
        if (((*(int *)(param_1 + 0x24) == 0x124) || (*(int *)(param_1 + 0x24) == 0x1ea)) ||
           ((*(int *)(param_1 + 0x24) == 0x5f6 ||
            ((*(int *)(param_1 + 0x24) == 0x681 || (*(int *)(param_1 + 0x24) == 0x7d5)))))) {
          piVar2[0xa6] = 1;
        }
        else {
          piVar2[0xa6] = 8;
        }
        (**(code **)(*piVar2 + 8))
                  (*(undefined4 *)(param_1 + 0x28),*(undefined4 *)(param_1 + 0x30),
                   *(undefined4 *)(param_1 + 0x2c),*(undefined4 *)(param_1 + 0x34),0,0,7,0);
      }
      (**(code **)(*DAT_005ccf9c + 0x34))(0,4,2);
      (**(code **)(*DAT_005ccf9c + 0x34))(1,4,2);
      (**(code **)(*DAT_005ccf9c + 0x30))(0x16,3);
      (**(code **)(*DAT_005ccf9c + 0x34))(0,1,4);
      (**(code **)(*DAT_005ccf9c + 0x34))(1,1,1);
      (**(code **)(*DAT_005ccf9c + 0x30))(0x13,2);
      (**(code **)(*DAT_005ccf9c + 0x30))(0x14,6);
    }
    else if (*(char *)(param_1 + 0x161) == '\x04') {
      piVar2 = (int *)FUN_004b14fd(*(int *)(param_1 + 0x24) + 1,0,180000);
      piVar4 = (int *)FUN_004b14fd(*(int *)(param_1 + 0x24) + 2,0,180000);
      if (piVar2 == (int *)0x0) {
        return 0;
      }
      if (piVar4 == (int *)0x0) {
        return 0;
      }
      (**(code **)(*piVar2 + 8))
                (*(undefined4 *)(param_1 + 0x28),*(float *)(param_1 + 0x30) + _DAT_005a593c,
                 *(undefined4 *)(param_1 + 0x2c),-*(float *)(param_1 + 0x168),0,0,0,0);
      (**(code **)(*piVar2 + 8))
                (*(undefined4 *)(param_1 + 0x28),*(float *)(param_1 + 0x30) + _DAT_005a5938,
                 *(undefined4 *)(param_1 + 0x2c),-*(float *)(param_1 + 0x168),0,0,0,0);
      (**(code **)(*piVar4 + 8))
                (*(undefined4 *)(param_1 + 0x28),*(float *)(param_1 + 0x30) + _DAT_005a365c,
                 *(undefined4 *)(param_1 + 0x2c),*(undefined4 *)(param_1 + 0x168),0,0,0,0);
    }
    else if (*(char *)(param_1 + 0x161) == '\x05') {
      piVar2 = (int *)FUN_004b14fd(*(int *)(param_1 + 0x24) + 1,0,180000);
      piVar4 = (int *)FUN_004b14fd(*(int *)(param_1 + 0x24) + 2,0,180000);
      if (piVar2 == (int *)0x0) {
        return 0;
      }
      if (piVar4 == (int *)0x0) {
        return 0;
      }
      (**(code **)(*piVar2 + 8))
                (*(float *)(param_1 + 0x28) + *(float *)(param_1 + 0x170),
                 *(float *)(param_1 + 0x30) + _DAT_005a4678,
                 *(float *)(param_1 + 0x2c) + *(float *)(param_1 + 0x174),
                 *(float *)(param_1 + 0x34) - _DAT_005a4380,*(undefined4 *)(param_1 + 0x168),
                 0x3fc90fdb,0,0);
      (**(code **)(*piVar2 + 8))
                (*(float *)(param_1 + 0x28) + *(float *)(param_1 + 0x178),
                 *(float *)(param_1 + 0x30) + _DAT_005a4678,
                 *(float *)(param_1 + 0x2c) + *(float *)(param_1 + 0x17c),
                 *(float *)(param_1 + 0x34) - _DAT_005a4380,-*(float *)(param_1 + 0x168),0x3fc90fdb,
                 0,0);
      (**(code **)(*piVar4 + 8))
                (*(undefined4 *)(param_1 + 0x28),*(undefined4 *)(param_1 + 0x30),
                 *(undefined4 *)(param_1 + 0x2c),*(undefined4 *)(param_1 + 0x34),0,0,0,0);
    }
    else if (*(char *)(param_1 + 0x161) == '\x06') {
      piVar2 = (int *)FUN_004b14fd(*(int *)(param_1 + 0x24) + 1,0,180000);
      if (piVar2 == (int *)0x0) {
        return 0;
      }
      (**(code **)(*piVar2 + 8))
                (*(undefined4 *)(param_1 + 0x28),*(undefined4 *)(param_1 + 0x30),
                 *(undefined4 *)(param_1 + 0x2c),*(undefined4 *)(param_1 + 0x34),0,0,6,0);
    }
    else if (*(char *)(param_1 + 0x161) == '\a') {
      piVar2 = (int *)FUN_004b14fd(*(int *)(param_1 + 0x24) + 0x14,0,180000);
      if (piVar2 == (int *)0x0) {
        return 0;
      }
      (**(code **)(*piVar2 + 8))
                (*(undefined4 *)(param_1 + 0x28),*(undefined4 *)(param_1 + 0x30),
                 *(undefined4 *)(param_1 + 0x2c),*(undefined4 *)(param_1 + 0x34),0,0,0x65,0);
    }
    else if (*(char *)(param_1 + 0x161) == '\b') {
      piVar2 = (int *)FUN_004b14fd(*(int *)(param_1 + 0x24) + 0x20,0,180000);
      if (piVar2 == (int *)0x0) {
        return 0;
      }
      (**(code **)(*piVar2 + 8))
                (*(undefined4 *)(param_1 + 0x28),*(undefined4 *)(param_1 + 0x30),
                 *(undefined4 *)(param_1 + 0x2c),*(undefined4 *)(param_1 + 0x34),0,0,100,0);
    }
    else if (*(char *)(param_1 + 0x161) == '\t') {
      piVar2 = (int *)FUN_004b14fd(*(int *)(param_1 + 0x24) + 0x3d,0,180000);
      if (piVar2 == (int *)0x0) {
        return 0;
      }
      (**(code **)(*piVar2 + 8))
                (*(undefined4 *)(param_1 + 0x28),*(undefined4 *)(param_1 + 0x30),
                 *(undefined4 *)(param_1 + 0x2c),*(undefined4 *)(param_1 + 0x34),0,0,6,0);
    }
    else if (*(char *)(param_1 + 0x161) == '\n') {
      piVar2 = (int *)FUN_004b14fd(*(undefined4 *)(param_1 + 0x24),0,180000);
      if (piVar2 == (int *)0x0) {
        return 0;
      }
      (**(code **)(*DAT_005ccf9c + 0x30))(0x1b,1);
      (**(code **)(*DAT_005ccf9c + 0x30))(0x14,2);
      (**(code **)(*DAT_005ccf9c + 0x34))(0,4,2);
      (**(code **)(*DAT_005ccf9c + 0x30))(0x89,1);
      (**(code **)(*DAT_005ccf9c + 0x30))(0x13,3);
      (**(code **)(*DAT_005ccf9c + 0x30))(0x14,4);
      (**(code **)(*DAT_005ccf9c + 0x34))(0,1,4);
      (**(code **)(*DAT_005ccf9c + 0x30))(0x19,8);
      (**(code **)(*DAT_005ccf9c + 0x30))(0xf,0);
      (**(code **)(*DAT_005ccf9c + 0x30))(0xe,0);
      (**(code **)(*DAT_005ccf9c + 0x30))(0x16,1);
      piVar2[1] = 1;
      iVar1 = piVar2[0xa6];
      piVar2[0xa6] = 0xea;
      (**(code **)(*piVar2 + 8))
                (*(undefined4 *)(param_1 + 0x28),*(undefined4 *)(param_1 + 0x30),
                 *(undefined4 *)(param_1 + 0x2c),*(undefined4 *)(param_1 + 0x34),0,0,100,0);
      piVar2[0xa6] = iVar1;
      piVar2[1] = 0;
      (**(code **)(*DAT_005ccf9c + 0x34))(0,4,2);
      (**(code **)(*DAT_005ccf9c + 0x30))(0x16,3);
      (**(code **)(*DAT_005ccf9c + 0x34))(0,1,4);
      (**(code **)(*DAT_005ccf9c + 0x30))(0x13,2);
      (**(code **)(*DAT_005ccf9c + 0x30))(0x14,6);
    }
  }
  (**(code **)(*DAT_005ccf9c + 0x30))(0x1b,1);
  return 1;
}


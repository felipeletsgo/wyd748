// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004d9bde | Name: FUN_004d9bde


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __fastcall FUN_004d9bde(int *param_1)

{
  int iVar1;
  undefined4 uVar2;
  uint uVar3;
  int local_19c;
  undefined1 local_198 [16];
  uint local_188;
  int local_180;
  uint local_17c;
  int local_178;
  int *local_174;
  undefined1 local_170 [64];
  undefined1 local_130 [64];
  float local_f0 [3];
  int local_e4;
  float local_e0;
  undefined4 local_dc;
  undefined4 local_d8;
  undefined4 local_d4;
  undefined4 local_d0;
  undefined4 local_cc;
  undefined4 local_c8;
  int local_c4 [9];
  undefined1 local_a0 [128];
  int local_20;
  float local_1c [4];
  undefined4 local_c;
  undefined4 local_8;
  
  if (((param_1[0x20] != 0) || (*(char *)((int)param_1 + 0x28f7) != '\0')) && (param_1[0x21] != 0))
  {
    iVar1 = *(int *)(DAT_013b71e8 + 0x1b084);
    local_1c[3] = *(float *)(iVar1 + 0x48);
    local_c = *(undefined4 *)(iVar1 + 0x4c);
    local_8 = *(undefined4 *)(iVar1 + 0x50);
    iVar1 = (**(code **)(*param_1 + 0x38))();
    if (iVar1 == 1) {
      (**(code **)(*(int *)DAT_005ccf9c[0xa8f9] + 0x94))
                (DAT_005ccf9c[0xa8f9],0x100,DAT_005ccf9c + 0xa99b);
      if (param_1[0x1b] == 1) {
        (**(code **)(*DAT_005ccf9c + 0x30))(0x14,2);
        (**(code **)(*DAT_005ccf9c + 0x34))(0,4,2);
      }
      (**(code **)(*DAT_005ccf9c + 0x30))(0x1b,1);
      (**(code **)(*DAT_005ccf9c + 0x30))(0x1c,0);
      (**(code **)(*DAT_005ccf9c + 0x30))(0x13,5);
      (**(code **)(*DAT_005ccf9c + 0x30))(0x19,8);
      (**(code **)(*DAT_005ccf9c + 0x30))(0xf,0);
      (**(code **)(*DAT_005ccf9c + 0x30))(0xe,0);
      (**(code **)(*DAT_005ccf9c + 0x30))(0x89,0);
      (**(code **)(*DAT_005ccf9c + 0x30))(0x16,1);
      (**(code **)(*(int *)DAT_005ccf9c[0xa8f9] + 0x130))(DAT_005ccf9c[0xa8f9],0x142);
      uVar2 = FUN_004b925c(param_1[0xa40],360000);
      (**(code **)(*DAT_005ccf9c + 0x38))(0,uVar2);
      if (param_1[0xa37] != 0) {
        (**(code **)(*(int *)DAT_005ccf9c[0xa8f9] + 0x120))
                  (DAT_005ccf9c[0xa8f9],5,0x1e,param_1 + 0x23,0x18);
      }
      if ((*(char *)((int)param_1 + 0x28f7) != '\0') && (DAT_0092e660 == 0)) {
        FUN_00430f10();
        FUN_00430f10();
        FUN_00430f10();
        FUN_00430f10();
        local_20 = (**(code **)(*DAT_0092e654 + 8))();
        if (*(char *)((int)param_1 + 0x28f7) == '\x02') {
          local_e0 = _DAT_005a3660 - (float)(uint)(local_20 - param_1[0x20]) / _DAT_005a4820;
        }
        else {
          local_e0 = _DAT_005a3660 - (float)(uint)(local_20 - param_1[0x20]) / _DAT_005a35c0;
        }
        if (local_e0 < _DAT_005a3534) {
          local_e0 = 0.0;
        }
        local_c4[0] = 10;
        local_c4[1] = 0x13;
        local_c4[2] = 0x14;
        local_c4[3] = 0x37;
        local_c4[4] = 0xc9;
        local_c4[5] = 0xc9;
        local_dc = 0x33;
        local_d8 = 0x33;
        local_d4 = 0;
        local_c4[6] = 0x22;
        local_c4[7] = 0x33;
        local_c4[8] = 0x33;
        local_d0 = 0x44;
        local_cc = 0x88;
        local_c8 = 0x88;
        local_f0[0] = 0.2;
        local_f0[1] = 0.3;
        local_f0[2] = 0.3;
        local_1c[0] = 1000.0;
        local_1c[1] = 100.0;
        local_1c[2] = -100.0;
        for (local_e4 = 0; local_e4 < 3; local_e4 = local_e4 + 1) {
          if (*(char *)((int)param_1 + 0x28f7) == '\x02') {
            thunk_FUN_0056230d(local_170,local_f0[local_e4] * _DAT_005a4154,
                               local_f0[local_e4] * _DAT_005a4154,
                               (float)param_1[0xa3a] * _DAT_005a4520);
          }
          else if (*(char *)((int)param_1 + 0x28f7) == '\x03') {
            thunk_FUN_0056230d(local_170,local_f0[local_e4],local_f0[local_e4],
                               (float)param_1[0xa3a] * _DAT_005a4bdc);
          }
          else if (*(char *)((int)param_1 + 0x28f7) == '\x04') {
            thunk_FUN_0056230d(local_170,local_f0[local_e4],local_f0[local_e4],
                               (float)param_1[0xa3a] * _DAT_005a4bd8);
          }
          else if (*(char *)((int)param_1 + 0x28f7) == '\x05') {
            thunk_FUN_0056230d(local_170,local_f0[local_e4],local_f0[local_e4],0x3f000000);
          }
          else {
            thunk_FUN_0056230d(local_170,local_f0[local_e4],local_f0[local_e4],
                               (float)param_1[0xa3a] * _DAT_005a430c);
          }
          uVar3 = (**(code **)(*DAT_0092e654 + 8))();
          thunk_FUN_00562566(local_a0,(float)(uVar3 % 0x1888) / local_1c[local_e4]);
          if (((*(char *)((int)param_1 + 0x28f7) == '\x02') ||
              (*(char *)((int)param_1 + 0x28f7) == '\x03')) ||
             (*(char *)((int)param_1 + 0x28f7) == '\x05')) {
            thunk_FUN_005624c9(local_130,0xbfc90fdb);
            thunk_FUN_00561a5b(local_170,local_170,local_130);
            thunk_FUN_00561a5b(local_170,local_170,param_1 + 0xb);
            thunk_FUN_00561a5b(local_a0,local_a0,local_170);
          }
          else if (*(char *)((int)param_1 + 0x28f7) == '\x04') {
            iVar1 = (**(code **)(*DAT_0092e654 + 8))();
            thunk_FUN_005624c9(local_130,(float)((uint)(iVar1 * 5) % 0x1888) / local_1c[2]);
            thunk_FUN_00561a5b(local_170,local_170,local_130);
            thunk_FUN_00561a5b(local_170,local_170,param_1 + 0xb);
            thunk_FUN_00561a5b(local_a0,local_a0,local_170);
          }
          else {
            thunk_FUN_00561a5b(local_170,local_170,param_1 + 0xb);
            thunk_FUN_00561a5b(local_a0,local_a0,local_170);
          }
          (**(code **)(*(int *)DAT_005ccf9c[0xa8f9] + 0x94))(DAT_005ccf9c[0xa8f9],0x100,local_a0);
          local_174 = (int *)FUN_004b14fd(local_c4[local_e4],1,180000);
          if (local_174 != (int *)0x0) {
            local_180 = __ftol();
            iVar1 = __ftol();
            local_17c = __ftol();
            if (1 < local_e4) {
              local_174[0xa6] = local_c4[local_e4 + 3];
            }
            (**(code **)(*(int *)local_174[0xc6] + 0x34))(local_174[0xc6],local_198);
            (**(code **)(*(int *)local_174[0xc6] + 0x2c))(local_174[0xc6],0,0,&local_178,0);
            for (local_19c = 0; local_19c < (int)(local_188 / 0x18); local_19c = local_19c + 1) {
              *(uint *)(local_178 + 0xc + local_19c * 0x18) =
                   local_180 << 0x10 | iVar1 << 8 | local_17c;
            }
            (**(code **)(*(int *)local_174[0xc6] + 0x30))(local_174[0xc6]);
            (**(code **)(*local_174 + 0xc))(0,0);
          }
        }
      }
      (**(code **)(*DAT_005ccf9c + 0x30))(0x16,3);
      (**(code **)(*DAT_005ccf9c + 0x30))(0x89,1);
      (**(code **)(*DAT_005ccf9c + 0x30))(0x13,2);
      (**(code **)(*DAT_005ccf9c + 0x30))(0x19,7);
      (**(code **)(*DAT_005ccf9c + 0x30))(0xf,1);
      (**(code **)(*DAT_005ccf9c + 0x30))(0xe,1);
      if (param_1[0x1b] == 1) {
        (**(code **)(*DAT_005ccf9c + 0x30))(0x14,6);
      }
    }
  }
  return 1;
}


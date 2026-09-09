// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004f2055 | Name: FUN_004f2055


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __fastcall FUN_004f2055(int *param_1)

{
  int iVar1;
  undefined4 uVar2;
  undefined1 local_334 [64];
  float local_2f4;
  int *local_2f0;
  float local_2ec;
  undefined1 local_2e8 [64];
  undefined1 local_2a8 [64];
  undefined1 local_268 [64];
  undefined1 local_228 [64];
  float local_1e8;
  float local_1e4;
  int *local_1e0;
  undefined1 local_1dc [64];
  float local_19c;
  undefined1 local_198 [64];
  undefined1 local_158 [64];
  undefined1 local_118 [64];
  float local_d8;
  int *local_d4;
  undefined1 local_d0 [64];
  int *local_90;
  undefined1 local_8c [64];
  undefined1 local_4c [64];
  int local_c;
  
  if ((short)param_1[0x58] == 0x167) {
    return 1;
  }
  iVar1 = (**(code **)(*param_1 + 0x4c))();
  if (iVar1 == 1) {
    (**(code **)(*(int *)DAT_005ccf9c[0xa8f9] + 0xa8))(DAT_005ccf9c[0xa8f9],param_1 + 0x5a);
    if ((0xc48 < (short)param_1[0x58]) && ((short)param_1[0x58] < 0xc4e)) {
      uVar2 = FUN_004f59b4();
      return uVar2;
    }
    if (DAT_005ccf9c[0xa8e4] == 1) {
      (**(code **)(*DAT_005ccf9c + 0x38))(1,0);
      (**(code **)(*DAT_005ccf9c + 0x34))(1,1,1);
    }
    else {
      local_c = FUN_004b14fd(param_1[0xbd],0,180000);
      if (local_c == 0) {
        return 0;
      }
      if (*(char *)(DAT_0092e640 + 0x43427 + *(int *)(local_c + 0x298) * 0x108) == 'N') {
        (**(code **)(*DAT_005ccf9c + 0x34))(1,0xb,0);
        uVar2 = FUN_004b925c(6,360000);
        (**(code **)(*DAT_005ccf9c + 0x38))(1,uVar2);
        (**(code **)(*DAT_005ccf9c + 0x34))(1,1,4);
      }
      (**(code **)(*DAT_005ccf9c + 0x34))(0,1,4);
    }
    if (*(short *)((int)param_1 + 0x302) == 0) {
      local_d8 = 0.0;
      if (param_1[0xbe] == 0x88) {
        local_d8 = 0.2;
      }
      else if (param_1[0xbe] == 0x6d5) {
        local_d8 = 0.75;
      }
      if ((short)param_1[0xbc] == 0x305) {
        local_d8 = 0.61;
      }
      if ((short)param_1[0xbc] == 800) {
        local_d8 = 0.61;
      }
      FUN_00430f10();
      FUN_00430f10();
      FUN_00430f10();
      FUN_00430f10();
      FUN_00430fc0(local_4c);
      FUN_00430fc0(local_d0);
      thunk_FUN_0056239d(local_8c,param_1[10],param_1[0xc],param_1[0xb]);
      thunk_FUN_00562821(local_4c,param_1[0xd],0,0);
      thunk_FUN_00561a5b(local_4c,local_4c,local_8c);
      thunk_FUN_0056239d(local_118,local_d8 + (float)param_1[0xbf],0,0);
      thunk_FUN_00562821(local_d0,-(float)param_1[0xb9] + _DAT_005a4148,0xbfc90fdb,0);
      thunk_FUN_00561a5b(local_d0,local_d0,local_118);
      thunk_FUN_00561a5b(local_4c,local_d0,local_4c);
      (**(code **)(*(int *)DAT_005ccf9c[0xa8f9] + 0x94))(DAT_005ccf9c[0xa8f9],0x100,local_4c);
      local_d4 = (int *)FUN_004b14fd(param_1[0xbd],0,180000);
      if (local_d4 == (int *)0x0) {
        return 0;
      }
      (**(code **)(*local_d4 + 0xc))(0,0);
      FUN_00430fc0(local_4c);
      FUN_00430fc0(local_d0);
      thunk_FUN_0056239d(local_8c,param_1[10],param_1[0xc],param_1[0xb]);
      thunk_FUN_00562821(local_4c,param_1[0xd],0,0);
      thunk_FUN_00561a5b(local_4c,local_4c,local_8c);
      thunk_FUN_0056239d(local_118,_DAT_005a54e4 * (float)param_1[0xbf] + local_d8,0,0);
      thunk_FUN_00562821(local_d0,param_1[0xb9],0xbfc90fdb,0);
      thunk_FUN_00561a5b(local_d0,local_d0,local_118);
      thunk_FUN_00561a5b(local_4c,local_d0,local_4c);
      (**(code **)(*(int *)DAT_005ccf9c[0xa8f9] + 0x94))(DAT_005ccf9c[0xa8f9],0x100,local_4c);
      local_90 = (int *)FUN_004b14fd(param_1[0xbe],0,180000);
      if (local_90 == (int *)0x0) {
        return 0;
      }
      (**(code **)(*local_90 + 0xc))(0,0);
    }
    else if (*(short *)((int)param_1 + 0x302) == 1) {
      local_1e8 = 1.2;
      local_1e4 = 0.0;
      local_19c = (float)param_1[0xc] - (float)param_1[0xb9];
      if ((param_1[0xbe] < 0x3dd) || (0x3e0 < param_1[0xbe])) {
        if (param_1[0xbe] == 0xae0) {
          return 1;
        }
      }
      else {
        local_1e8 = 0.0;
        local_1e4 = -1.2;
        local_19c = ((float)param_1[0xc] - (float)param_1[0xb9]) - _DAT_005a40f4;
        if (param_1[0xb8] != 0) {
          *(float *)(param_1[0xb8] + 0x10c) = -(float)param_1[0xb9] + _DAT_005a4158;
        }
      }
      FUN_00430f10();
      FUN_00430f10();
      FUN_00430f10();
      FUN_00430f10();
      FUN_00430fc0(local_158);
      FUN_00430fc0(local_1dc);
      thunk_FUN_0056239d(local_198,local_1e4 + (float)param_1[10],local_19c,
                         local_1e8 + (float)param_1[0xb]);
      thunk_FUN_00562821(local_158,param_1[0xd],0,0);
      thunk_FUN_00561a5b(local_158,local_158,local_198);
      thunk_FUN_0056239d(local_228,param_1[0xbf],0,0);
      thunk_FUN_00562821(local_1dc,0x40490fdb,0xbfc90fdb,0);
      thunk_FUN_00561a5b(local_1dc,local_1dc,local_228);
      thunk_FUN_00561a5b(local_158,local_1dc,local_158);
      (**(code **)(*(int *)DAT_005ccf9c[0xa8f9] + 0x94))(DAT_005ccf9c[0xa8f9],0x100,local_158);
      local_1e0 = (int *)FUN_004b14fd(param_1[0xbd],0,180000);
      if (local_1e0 == (int *)0x0) {
        return 0;
      }
      (**(code **)(*local_1e0 + 0xc))(0,0);
    }
    else if (*(short *)((int)param_1 + 0x302) == 2) {
      local_2f4 = 1.2;
      local_2ec = 0.3;
      FUN_00430f10();
      FUN_00430f10();
      FUN_00430f10();
      FUN_00430f10();
      FUN_00430fc0(local_268);
      FUN_00430fc0(local_2e8);
      thunk_FUN_0056239d(local_2a8,local_2ec + (float)param_1[10],
                         (float)param_1[0xc] + (float)param_1[0xb9],local_2f4 + (float)param_1[0xb])
      ;
      thunk_FUN_00562821(local_268,param_1[0xd],0,0);
      thunk_FUN_00561a5b(local_268,local_268,local_2a8);
      thunk_FUN_0056239d(local_334,param_1[0xbf],0,0);
      thunk_FUN_00562821(local_2e8,0x40490fdb,0xbfc90fdb,0);
      thunk_FUN_00561a5b(local_2e8,local_2e8,local_334);
      thunk_FUN_00561a5b(local_268,local_2e8,local_268);
      (**(code **)(*(int *)DAT_005ccf9c[0xa8f9] + 0x94))(DAT_005ccf9c[0xa8f9],0x100,local_268);
      local_2f0 = (int *)FUN_004b14fd(param_1[0xbd],0,180000);
      if (local_2f0 == (int *)0x0) {
        return 0;
      }
      (**(code **)(*local_2f0 + 0xc))(0,0);
    }
    if (DAT_005ccf9c[0xa8e4] == 1) {
      (**(code **)(*DAT_005ccf9c + 0x34))(1,1,1);
      (**(code **)(*DAT_005ccf9c + 0x34))(1,0xb,1);
    }
    else {
      (**(code **)(*DAT_005ccf9c + 0x34))(1,1,1);
      (**(code **)(*DAT_005ccf9c + 0x34))(1,0xb,1);
    }
  }
  iVar1 = *(int *)(DAT_0067cf38 + 0x4c);
  if (((iVar1 != 0) &&
      (((((float)param_1[10] < (float)(*(int *)(iVar1 + 0x47c) + -0x21) ||
         ((float)(*(int *)(iVar1 + 0x47c) + 0x21) < (float)param_1[10])) ||
        ((float)param_1[0xb] < (float)(*(int *)(iVar1 + 0x480) + -0x21))) ||
       ((float)(*(int *)(iVar1 + 0x480) + 0x21) < (float)param_1[0xb])))) &&
     (*(int **)(DAT_0067cf38 + 0x80) == param_1)) {
    *(undefined4 *)(DAT_0067cf38 + 0x80) = 0;
  }
  return 1;
}


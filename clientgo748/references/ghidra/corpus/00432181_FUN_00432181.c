// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00432181 | Name: FUN_00432181


undefined4 __fastcall FUN_00432181(int param_1)

{
  void *pvVar1;
  int iVar2;
  undefined4 uVar3;
  undefined4 local_188;
  undefined4 local_184;
  undefined4 local_180;
  undefined4 local_17c;
  undefined4 local_178;
  undefined4 local_174;
  undefined4 local_170;
  undefined4 local_16c;
  int local_114;
  undefined1 local_110 [128];
  undefined1 local_90 [128];
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0059f186;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  pvVar1 = operator_new(0x1e4);
  local_8 = 0;
  if (pvVar1 == (void *)0x0) {
    local_16c = 0;
  }
  else {
    local_16c = FUN_00401466(0xfffffffe,0,0,0x44480000,0x44160000,0,4);
  }
  local_8 = 0xffffffff;
  *(undefined4 *)(param_1 + 0x3a778) = local_16c;
  pvVar1 = operator_new(0x1e4);
  local_8 = 1;
  if (pvVar1 == (void *)0x0) {
    local_170 = 0;
  }
  else {
    local_170 = FUN_00401466(0xfffffffe,0,0,0x44480000,0x42c80000,0xff000000,4);
  }
  local_8 = 0xffffffff;
  pvVar1 = operator_new(0x1e4);
  local_8 = 2;
  if (pvVar1 == (void *)0x0) {
    local_174 = 0;
  }
  else {
    local_174 = FUN_00401466(0xfffffffe,0,0x43fa0000,0x44480000,0x42c80000,0xff000000,4);
  }
  local_8 = 0xffffffff;
  (**(code **)(**(int **)(param_1 + 0x28) + 0x44))(local_170);
  (**(code **)(**(int **)(param_1 + 0x28) + 0x44))(local_174);
  (**(code **)(**(int **)(param_1 + 0x28) + 0x44))(*(undefined4 *)(param_1 + 0x3a778));
  FUN_00431faf();
  (**(code **)(**(int **)(param_1 + 0x3a778) + 0x60))(0);
  pvVar1 = operator_new(0xe4c);
  local_8 = 3;
  if (pvVar1 == (void *)0x0) {
    local_178 = 0;
  }
  else {
    local_178 = FUN_00401d03(0xfffffffe,s_Press_anykey_to_exit__005b4c0c,0xffffffff,0,0x41f00000,
                             0x44480000,0x41a00000,1,0,1,1);
  }
  local_8 = 0xffffffff;
  *(undefined4 *)(param_1 + 0x3a77c) = local_178;
  FUN_0054ac09(*(undefined4 *)(param_1 + 0x3a77c));
  (**(code **)(**(int **)(param_1 + 0x3a77c) + 0x60))(0);
  *(undefined4 *)(DAT_005ccf9c + 0x2b034) = 0;
  *(undefined4 *)(DAT_005ccf9c + 0x2a5e8) = 0;
  (**(code **)(**(int **)(DAT_013b71e8 + 0x1b084) + 0x50))(0);
  _memset((void *)(param_1 + 0x94),0,0x10000);
  FUN_0058f078(local_110,s_env_Field1817_trn_005b4c24);
  FUN_0058f078(local_90,s_env_Field1817_dat_005b4c38);
  pvVar1 = operator_new(0x20a70);
  local_8 = 4;
  if (pvVar1 == (void *)0x0) {
    local_17c = 0;
  }
  else {
    local_17c = FUN_00532390();
  }
  local_8 = 0xffffffff;
  *(undefined4 *)(param_1 + 0x68) = local_17c;
  iVar2 = FUN_00533dd7(local_110);
  if (iVar2 == 0) {
    if (*(int *)(param_1 + 0x54) == 0) {
      FUN_0049ed58(1,0,0,0,0);
    }
    *(undefined4 *)(param_1 + 0x54) = 1;
  }
  *(undefined4 *)(param_1 + 0x40) = *(undefined4 *)(param_1 + 0x68);
  DAT_005b8924 = __ftol();
  DAT_005b8928 = __ftol();
  FUN_00534968();
  pvVar1 = operator_new(0x404c);
  local_8 = 5;
  if (pvVar1 == (void *)0x0) {
    local_180 = 0;
  }
  else {
    local_180 = FUN_00540ea3(*(undefined4 *)(param_1 + 0x40));
  }
  local_8 = 0xffffffff;
  *(undefined4 *)(param_1 + 0x70) = local_180;
  FUN_0054ac09(*(undefined4 *)(param_1 + 0x70));
  iVar2 = FUN_00541065(local_90);
  if (iVar2 == 0) {
    FUN_00431159(s_DataFile_Not_Found_005b4c4c);
    MessageBoxA(*(HWND *)(DAT_013b71e0 + 4),s_DataFile_Not_Found__005b4c70,s_File_Lost_005b4c64,0);
    PostMessageA(*(HWND *)(DAT_013b71e0 + 4),0x10,0,0);
    uVar3 = 0;
  }
  else {
    FUN_0054ac09(*(undefined4 *)(param_1 + 0x68));
    for (local_114 = 0; local_114 < 0x80; local_114 = local_114 + 1) {
      FUN_0058f220(param_1 + 0x94 + local_114 * 0x100,
                   *(int *)(param_1 + 0x40) + 0xc0cc + local_114 * 0x80,0x80);
    }
    pvVar1 = operator_new(0x16c);
    local_8 = 6;
    if (pvVar1 == (void *)0x0) {
      local_184 = 0;
    }
    else {
      local_184 = FUN_004eef50();
    }
    local_8 = 0xffffffff;
    *(undefined4 *)(param_1 + 0x48) = local_184;
    (**(code **)(**(int **)(param_1 + 0x48) + 0x38))();
    FUN_0054ac09(*(undefined4 *)(param_1 + 0x48));
    pvVar1 = operator_new(0x22a4);
    local_8 = 7;
    if (pvVar1 == (void *)0x0) {
      local_188 = 0;
    }
    else {
      local_188 = FUN_00547890();
    }
    local_8 = 0xffffffff;
    *(undefined4 *)(param_1 + 0x44) = local_188;
    FUN_0054ac09(*(undefined4 *)(param_1 + 0x44));
    FUN_00549bbf(0);
    *(undefined4 *)(*(int *)(param_1 + 0x44) + 0x16c) = 20000;
    _memset((void *)(param_1 + 0x26e30),0,200);
    uVar3 = FUN_00430f20(0x4513c000,0x450dc000);
    FUN_0040bd30(uVar3);
    *(undefined4 *)(*(int *)(DAT_013b71e8 + 0x1b084) + 0x28) = 0x3f490fdb;
    *(undefined4 *)(*(int *)(DAT_013b71e8 + 0x1b084) + 0x2c) = 0;
    *(undefined4 *)(param_1 + 0x26e1c) = 0;
    if (DAT_005c4958 == 1) {
      FUN_00432d52();
    }
    FUN_0043387d();
    if (*(int *)(DAT_013b71e0 + 0xec) != 0) {
      FUN_00429480();
    }
    if (*(int *)(DAT_013b71e0 + 0xec) != 0) {
      FUN_00428ba0(0xd);
      *(undefined4 *)(param_1 + 0x26e1c) = 1;
    }
    uVar3 = 1;
  }
  ExceptionList = local_10;
  return uVar3;
}


// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004bd420 | Name: FUN_004bd420


int * __thiscall
FUN_004bd420(int *param_1,undefined4 param_2,int param_3,int param_4,int param_5,int param_6,
            undefined2 param_7,undefined2 param_8)

{
  int *piVar1;
  int iVar2;
  int local_8;
  
  FUN_00430f10();
  FUN_00430f10();
  FUN_00430f10();
  FUN_004c5b40();
  FUN_004c5b40();
  FUN_004c5b40();
  *param_1 = param_4;
  param_1[2] = param_5;
  param_1[1] = 0;
  param_1[0xc] = 0;
  *(undefined1 *)((int)param_1 + 0x307) = 1;
  piVar1 = (int *)FUN_00430f50(0,0,0);
  param_1[3] = *piVar1;
  param_1[4] = piVar1[1];
  param_1[5] = piVar1[2];
  piVar1 = (int *)FUN_00430f50(0,0,0);
  param_1[6] = *piVar1;
  param_1[7] = piVar1[1];
  param_1[8] = piVar1[2];
  piVar1 = (int *)FUN_00430f50(0x3f800000,0x3f800000,0x3f800000);
  param_1[9] = *piVar1;
  param_1[10] = piVar1[1];
  param_1[0xb] = piVar1[2];
  param_1[0x2d] = 0;
  param_1[0xb3] = 0;
  param_1[0xbb] = 0;
  param_1[0xbc] = 0;
  param_1[0xb4] = 0;
  param_1[0xb9] = 0x1e;
  param_1[0xba] = 0;
  param_1[0xbd] = 0;
  FUN_0058f220(param_1 + 0x92,param_2,0x20);
  if (param_3 == 0) {
    _memset(param_1 + 0x9a,0,0x10);
  }
  else {
    FUN_0058f220(param_1 + 0x9a,param_3,0x10);
  }
  for (local_8 = 0; local_8 < 0x50; local_8 = local_8 + 1) {
    param_1[local_8 + 0x2e] = 0;
  }
  param_1[0xb7] = 0;
  if (0x3b < param_4) {
    iVar2 = _rand();
    param_1[0xb7] = (iVar2 % 5 + ((uint)param_1 >> 1) % 5) * 500;
  }
  _memset(param_1 + 0xa2,0,0x44);
  param_1[0xa2] = 0x3f800000;
  param_1[0xa3] = 0x3f800000;
  param_1[0xa4] = 0x3f800000;
  param_1[0xaa] = param_1[0xa2];
  param_1[0xab] = param_1[0xa3];
  param_1[0xac] = param_1[0xa4];
  param_1[0xad] = param_1[0xa5];
  param_1[0xb2] = 0;
  param_1[0xae] = 0x3e99999a;
  param_1[0xaf] = 0x3e99999a;
  param_1[0xb0] = 0x3e99999a;
  param_1[0xb8] = 0;
  param_1[0xb5] = 0;
  param_1[0xbe] = 0;
  param_1[0xbf] = 0;
  *(undefined1 *)(param_1 + 0xc1) = 0;
  *(undefined1 *)((int)param_1 + 0x305) = 0;
  *(undefined1 *)((int)param_1 + 0x306) = 1;
  if (*param_1 == 1) {
    param_1[0xc0] = 0x3d8f5c29;
  }
  else {
    param_1[0xc0] = 0x3dcccccd;
  }
  if (param_6 == 0) {
    _memset(param_1 + 0x9e,0,0x10);
  }
  else {
    FUN_0058f220(param_1 + 0x9e,param_6,0x10);
  }
  _memset(param_1 + 0xc2,0,0x40);
  param_1[0xd2] = 0;
  *(undefined1 *)(param_1 + 0xd3) = 0;
  param_1[0xd4] = 0;
  *(undefined2 *)(param_1 + 0xd5) = param_7;
  *(undefined2 *)((int)param_1 + 0x356) = param_8;
  return param_1;
}


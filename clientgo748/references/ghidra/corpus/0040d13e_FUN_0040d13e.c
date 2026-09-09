// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0040d13e | Name: FUN_0040d13e


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 * __thiscall
FUN_0040d13e(undefined4 *param_1,undefined4 param_2,short *param_3,undefined4 param_4,
            undefined4 param_5)

{
  byte bVar1;
  ushort uVar2;
  int iVar3;
  undefined4 uVar4;
  int local_20;
  int local_1c;
  int local_18;
  void *local_10;
  undefined1 *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &LAB_0059ee28;
  local_10 = ExceptionList;
  ExceptionList = &local_10;
  FUN_00401ac0(0,param_4,param_5,0x41c00000,0x41c00000);
  local_8 = 0;
  FUN_0040bea0(4,0xfffffffe,0,0,0x3f800000,0x3f800000,0,0x33ff0000);
  FUN_0040bea0(0,0xffffffff,0,0,0,0,0,0xffffffff);
  FUN_004219a0();
  local_8 = CONCAT31(local_8._1_3_,1);
  param_1[0x19c] = param_3;
  *param_1 = &PTR_FUN_005a3f88;
  if (((param_1[0x19c] != 0) && (-1 < *param_3)) && (*param_3 < 0x1965)) {
    param_1[0x27] = (int)*(short *)(&DAT_00d44990 + *param_3 * 0x8c);
    param_1[0x79] = param_2;
    *(undefined1 *)((int)param_1 + 0x39a) = 0;
    param_1[0x128] = param_1 + 0x130;
    param_1[0x78] = 0;
    param_1[0x19d] = 0x3f800000;
    local_18 = FUN_0054cd07(param_3,0x21);
    iVar3 = FUN_0054cd07(param_3,0x26);
    if ((7 < local_18) || (local_18 < 0)) {
      local_18 = 0;
    }
    param_1[0x76] = *(undefined4 *)(&DAT_005bec0c + local_18 * 8);
    param_1[0x77] = *(undefined4 *)(&DAT_005bec10 + local_18 * 8);
    param_1[0x15] = (float)(int)param_1[0x76] * (float)DAT_005b1094;
    param_1[0x16] = (float)(int)param_1[0x77] * (float)DAT_005b1094;
    if (*param_3 == 0x102d) {
      param_1[0x70] = 0xff8800bb;
    }
    if (*param_3 == 0x102e) {
      param_1[0x70] = 0xff444488;
    }
    if (*param_3 == 0x6cb) {
      param_1[0x70] = 0xffaa8888;
    }
    if (*param_3 == 0x1031) {
      param_1[0x70] = 0xffaa8888;
    }
    if ((iVar3 == 0xce) && ((char)param_3[1] == ';')) {
      param_1[0x70] = 0xffaa8888;
    }
    param_1[0x6e] = param_1[0x15];
    param_1[0x6f] = param_1[0x16];
    if ((0x6cf < *param_3) && (*param_3 < 0x6d8)) {
      param_1[0x70] = *(undefined4 *)(&DAT_005af75c + *param_3 * 4);
    }
    if ((*param_3 < 0x6e0) || (0x6e3 < *param_3)) {
      if (*param_3 == 0xfc0) {
        param_1[0x70] = 0xffffffaa;
      }
      else if (*param_3 == 0xc44) {
        param_1[0x70] = 0xffffffff;
      }
      else if ((*param_3 < 0xd4f) || (0xd58 < *param_3)) {
        if ((0x14d5 < *param_3) && (*param_3 < 0x14db)) {
          param_1[0x70] = 0xffffffaa;
        }
      }
      else {
        switch(*param_3) {
        case 0xd4f:
          param_1[0x70] = 0xffffffff;
          break;
        case 0xd50:
          param_1[0x70] = 0xffffffff;
          break;
        case 0xd51:
          param_1[0x70] = 0xffffffff;
          break;
        case 0xd52:
          param_1[0x70] = 0xffffffff;
          break;
        case 0xd53:
          param_1[0x70] = 0xffffffff;
          break;
        case 0xd54:
          param_1[0x70] = 0xffffffff;
          break;
        case 0xd55:
          param_1[0x70] = 0xffffffff;
          break;
        case 0xd56:
          param_1[0x70] = 0xffffffff;
          break;
        case 0xd57:
          param_1[0x70] = 0xffffffff;
          break;
        case 0xd58:
          param_1[0x70] = 0xffffffff;
        }
      }
    }
    else {
      param_1[0x70] = 0xffffffaa;
    }
    uVar4 = FUN_0040cea0(*param_3);
    FUN_00401bdc(uVar4);
    *(undefined2 *)(param_1 + 0x2a) = *(undefined2 *)(&DAT_00d449da + *param_3 * 0x8c);
    uVar4 = FUN_0054df49(param_3);
    switch(uVar4) {
    case 0x74:
      *(undefined2 *)(param_1 + 0x2a) = 0x74;
      break;
    case 0x75:
      *(undefined2 *)(param_1 + 0x2a) = 0x75;
      break;
    case 0x76:
      *(undefined2 *)(param_1 + 0x2a) = 0x76;
      break;
    case 0x77:
      *(undefined2 *)(param_1 + 0x2a) = 0x77;
      break;
    case 0x78:
      *(undefined2 *)(param_1 + 0x2a) = 0x78;
      break;
    case 0x79:
      *(undefined2 *)(param_1 + 0x2a) = 0x79;
      break;
    case 0x7a:
      *(undefined2 *)(param_1 + 0x2a) = 0x7a;
      break;
    case 0x7b:
      *(undefined2 *)(param_1 + 0x2a) = 0x7b;
      break;
    case 0x7c:
      *(undefined2 *)(param_1 + 0x2a) = 0x7c;
      break;
    case 0x7d:
      *(undefined2 *)(param_1 + 0x2a) = 0x7d;
    }
    if (*(short *)(&DAT_00d44990 + *(short *)param_1[0x19c] * 0x8c) < 0) {
      param_1[0x19] = 4;
      param_1[0x28] = 1;
      if (DAT_005b892c == 2) {
        param_1[0x28] = 199;
      }
      param_1[0x29] = (int)*(short *)(&DAT_00d44992 + *(short *)param_1[0x19c] * 0x8c);
      if (DAT_005b892c == 2) {
        if ((*(short *)param_1[0x19c] < 5000) || (0x13ee < *(short *)param_1[0x19c])) {
          param_1[0x15] = (float)(int)param_1[0x76] * _DAT_005a3f80;
          param_1[0x16] = (float)(int)param_1[0x77] * _DAT_005a3f80;
        }
        else {
          param_1[0x15] = (float)(int)param_1[0x76] * _DAT_005a3f84;
          param_1[0x16] = (float)(int)param_1[0x77] * _DAT_005a3f84;
        }
        param_1[0x6e] = param_1[0x15];
        param_1[0x6f] = param_1[0x16];
      }
    }
    else {
      FUN_0058f078((int)param_1 + 0x39a,&DAT_005ccf14);
      local_20 = FUN_0054e06c(param_3);
      if (0xc < local_20) {
        local_20 = 0xc;
      }
      param_1[0x29] = local_20;
      param_1[0x28] = (int)*(short *)(&DAT_00d44992 + *(short *)param_1[0x19c] * 0x8c);
      if ((((*(short *)(&DAT_00d449d4 + *(short *)param_1[0x19c] * 0x8c) == 0x33) ||
           (*(short *)(param_1 + 0x2a) != 0)) && (*(short *)(param_1 + 0x2a) < 5)) && (9 < local_20)
         ) {
        uVar2 = FUN_004173cf(param_1[0x19c]);
        *(ushort *)(param_1 + 0x2a) = (uVar2 & 0xff) + 4;
      }
      else if ((*(short *)(param_1 + 0x2a) == 4) && (9 < local_20)) {
        bVar1 = FUN_004173cf(param_1[0x19c]);
        *(ushort *)(param_1 + 0x2a) = (ushort)bVar1;
      }
      if (*param_3 == 0xa9) {
        param_1[0x28] = 0xfffffffc;
      }
      local_1c = FUN_0054e93b(param_3);
      if ((0x919 < *param_3) && (*param_3 < 0x956)) {
        local_1c = 0;
      }
      if (0 < local_1c) {
        FUN_0058f078((int)param_1 + 0x39a,&PTR_DAT_005b12c4,local_1c);
        FUN_00421b8f((int)param_1 + 0x39a,param_1[0xe1],0);
      }
    }
    if (param_1[0x27] == 0x1b) {
      if ((*(short *)(&DAT_00d44992 + *param_3 * 0x8c) < 6) ||
         (9 < *(short *)(&DAT_00d44992 + *param_3 * 0x8c))) {
        if ((*(short *)(&DAT_00d44992 + *param_3 * 0x8c) < 0xb) ||
           (0x11 < *(short *)(&DAT_00d44992 + *param_3 * 0x8c))) {
          if (*(short *)(&DAT_00d44992 + *param_3 * 0x8c) == 0x13) {
            param_1[0x28] = *(short *)(&DAT_00d44992 + *param_3 * 0x8c) + -2;
          }
          else if ((*(short *)(&DAT_00d44992 + *param_3 * 0x8c) == 0x15) ||
                  (*(short *)(&DAT_00d44992 + *param_3 * 0x8c) == 0x16)) {
            param_1[0x28] = *(short *)(&DAT_00d44992 + *param_3 * 0x8c) + -4;
          }
          else if (0x16 < *(short *)(&DAT_00d44992 + *param_3 * 0x8c)) {
            param_1[0x28] = *(short *)(&DAT_00d44992 + *param_3 * 0x8c) + -2;
          }
        }
        else {
          param_1[0x28] = *(short *)(&DAT_00d44992 + *param_3 * 0x8c) + -2;
        }
      }
      else {
        param_1[0x28] = *(short *)(&DAT_00d44992 + *param_3 * 0x8c) + -1;
      }
    }
    if (*param_3 == 0x1038) {
      param_1[0x27] = 0xb43;
      param_1[0x29] = 0x5d5;
    }
    else if (*param_3 == 0x1039) {
      param_1[0x27] = 0xb45;
      param_1[0x29] = 0x28e;
    }
  }
  ExceptionList = local_10;
  return param_1;
}


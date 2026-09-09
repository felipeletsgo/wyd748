// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004d414f | Name: FUN_004d414f


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __fastcall FUN_004d414f(int param_1)

{
  undefined4 uVar1;
  undefined4 *puVar2;
  float10 fVar3;
  float local_98;
  float local_94;
  float local_90;
  undefined4 local_8c;
  float local_88;
  float local_84;
  float local_80;
  undefined4 local_7c;
  float local_78;
  float local_74;
  float local_70;
  undefined4 local_6c;
  undefined4 local_68;
  undefined4 local_64;
  undefined4 local_60;
  float local_58;
  float local_54;
  float local_50;
  undefined4 local_4c;
  float local_44;
  int local_40;
  uint local_3c;
  int local_38;
  undefined1 local_34 [16];
  uint local_24;
  int local_1c;
  uint local_18;
  int local_14;
  uint local_10;
  int local_c;
  int *local_8;
  
  if (DAT_0092e660 == 1) {
    uVar1 = 1;
  }
  else if (*(int *)(param_1 + 0x28) == 0) {
    uVar1 = 0;
  }
  else {
    local_8 = (int *)0x0;
    (**(code **)(*DAT_005ccf9c + 0x30))(0x1b,1);
    (**(code **)(*DAT_005ccf9c + 0x30))(0xf,0);
    (**(code **)(*DAT_005ccf9c + 0x30))(0xe,0);
    (**(code **)(*DAT_005ccf9c + 0x30))(0x89,0);
    (**(code **)(*DAT_005ccf9c + 0x30))(0x19,8);
    if (*(int *)(param_1 + 0x6c) == 1) {
      local_8 = (int *)FUN_004b14fd(*(undefined4 *)(param_1 + 0xb4),1,180000);
      if (local_8 == (int *)0x0) {
        return 0;
      }
      (**(code **)(*DAT_005ccf9c + 0x30))(0x1c,0);
      (**(code **)(*DAT_005ccf9c + 0x30))(0x16,1);
      (**(code **)(*DAT_005ccf9c + 0x30))(0x13,5);
      (**(code **)(*DAT_005ccf9c + 0x30))(0x14,2);
      (**(code **)(*(int *)local_8[0xc6] + 0x34))(local_8[0xc6],local_34);
      (**(code **)(*(int *)local_8[0xc6] + 0x2c))(local_8[0xc6],0,0,&local_c,0);
      local_3c = local_24 / 0x18;
      local_44 = 1.0;
      if (*(int *)(param_1 + 0xb0) == 1) {
        local_44 = 1.0;
      }
      else {
        fVar3 = (float10)FUN_00423b00(*(float *)(param_1 + 0xb8) * _DAT_005a4148);
        local_44 = (float)fVar3;
      }
      local_14 = __ftol();
      local_1c = __ftol();
      local_40 = __ftol();
      local_18 = __ftol();
      local_10 = local_14 << 0x18 | local_1c << 0x10 | local_40 << 8 | local_18;
      for (local_38 = 0; local_38 < (int)local_3c; local_38 = local_38 + 1) {
        *(uint *)(local_c + 0xc + local_38 * 0x18) = local_10;
      }
      (**(code **)(*(int *)local_8[0xc6] + 0x30))(local_8[0xc6]);
    }
    else {
      local_8 = (int *)FUN_004b14fd(*(undefined4 *)(param_1 + 0xb4),0,180000);
      if (local_8 == (int *)0x0) {
        return 0;
      }
      (**(code **)(*DAT_005ccf9c + 0x30))(0x13,5);
      (**(code **)(*DAT_005ccf9c + 0x30))(0x14,6);
      (**(code **)(*DAT_005ccf9c + 0x34))(0,4,4);
      (**(code **)(*DAT_005ccf9c + 0x30))(0x89,1);
      (**(code **)(*DAT_005ccf9c + 0x30))(0x1c,DAT_005ccf9c[0xac0e]);
      puVar2 = (undefined4 *)
               FUN_00430f20(*(undefined4 *)(param_1 + 0x74),*(undefined4 *)(param_1 + 0x7c));
      FUN_0049cb58(&local_98,*puVar2,puVar2[1]);
      local_98 = (float)DAT_005ccf9c[0xa9b9] * _DAT_005a40ec + _DAT_005a4158 * local_98;
      local_94 = (float)DAT_005ccf9c[0xa9ba] * _DAT_005a40ec + _DAT_005a4158 * local_98;
      local_90 = (float)DAT_005ccf9c[0xa9bb] * _DAT_005a40ec + _DAT_005a4158 * local_98;
      local_8c = 0x3f800000;
      local_6c = 0x3f800000;
      local_7c = 0x3f800000;
      local_4c = 0x3f800000;
      local_68 = 0x3f800000;
      local_64 = 0x3f800000;
      local_60 = 0x3f800000;
      local_88 = local_98;
      local_84 = local_94;
      local_80 = local_90;
      local_78 = local_98;
      local_74 = local_94;
      local_70 = local_90;
      local_58 = local_98;
      local_54 = local_94;
      local_50 = local_90;
      (**(code **)(*(int *)DAT_005ccf9c[0xa8f9] + 0xa8))(DAT_005ccf9c[0xa8f9],&local_88);
    }
    if (local_8 == (int *)0x0) {
      uVar1 = 0;
    }
    else {
      local_8[0xcc] = *(int *)(param_1 + 0xb8);
      local_8[0xcb] = *(int *)(param_1 + 0xb8);
      if (*(int *)(param_1 + 0xb0) == 1) {
        (**(code **)(*local_8 + 8))
                  (*(undefined4 *)(param_1 + 0x74),*(undefined4 *)(param_1 + 0x78),
                   *(undefined4 *)(param_1 + 0x7c),*(undefined4 *)(param_1 + 0xa4),
                   *(undefined4 *)(param_1 + 0xa8),0x3fc90fdb,0,0);
      }
      else {
        (**(code **)(*local_8 + 8))
                  (*(undefined4 *)(param_1 + 0x74),*(undefined4 *)(param_1 + 0x78),
                   *(undefined4 *)(param_1 + 0x7c),*(undefined4 *)(param_1 + 0xa4),0,0x3fc90fdb,0,0)
        ;
      }
      uVar1 = 1;
    }
  }
  return uVar1;
}


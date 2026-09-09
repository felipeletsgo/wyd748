// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004d8bbe | Name: FUN_004d8bbe


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __fastcall FUN_004d8bbe(int *param_1)

{
  int iVar1;
  undefined4 uVar2;
  undefined1 local_78 [36];
  float local_54;
  float local_50;
  float local_4c;
  float local_48;
  undefined4 local_44;
  undefined4 local_40;
  undefined4 local_3c;
  undefined4 local_38;
  undefined4 local_34;
  undefined4 local_30;
  undefined4 local_2c;
  int local_28;
  int local_24;
  undefined1 local_20 [12];
  int local_14;
  undefined1 local_10 [12];
  
  iVar1 = (**(code **)(*DAT_0092e654 + 8))();
  (**(code **)(*param_1 + 0x38))();
  if ((param_1[0x2c] == 0) || ((uint)(iVar1 - param_1[0x2b]) <= (uint)param_1[0x2c])) {
    CSimpleArray<>();
    CSimpleArray<>();
    CSimpleArray<>();
    CSimpleArray<>();
    uVar2 = FUN_00430f50(0,0,0x41200000);
    uVar2 = FUN_004310a0(uVar2);
    FUN_004310a0(uVar2);
    local_44 = 0;
    local_14 = param_1[7];
    if (local_14 != 0) {
      local_24 = 0;
      if ((*(int *)(local_14 + 0x60) != 0) && (*(int *)(*(int *)(local_14 + 0x60) + 0xb4) != 0)) {
        local_24 = FUN_004c15fc(param_1[0x27]);
      }
      if (local_24 != 0) {
        thunk_FUN_00561462(local_10,param_1 + 0x2d,local_24 + 0x48);
        thunk_FUN_00561462(local_20,param_1 + 0x30,local_24 + 0x48);
        FUN_004310a0(local_10);
        for (local_28 = 0; local_28 < 5; local_28 = local_28 + 1) {
          FUN_004a8940(&local_34,local_10,local_20,(float)(local_28 + 1) / _DAT_005a4104);
          if (local_28 < 4) {
            local_54 = (float)(5 - local_28) * _DAT_005a442c;
            if (_DAT_005a40ec < local_54) {
              local_54 = 0.5;
            }
            iVar1 = _rand();
            local_50 = (float)(iVar1 % 9 + -5) * local_54 * (float)param_1[0x21];
            iVar1 = _rand();
            local_48 = (float)(iVar1 % 9 + -5) * local_54 * (float)param_1[0x21];
            iVar1 = _rand();
            local_4c = (float)(iVar1 % 9 + -5) * local_54 * (float)param_1[0x21];
            uVar2 = FUN_00430f50(local_50,local_48,local_4c);
            uVar2 = FUN_00493d50(local_78,uVar2);
            FUN_004310a0(uVar2);
          }
          if (param_1[local_28 + 0x22] != 0) {
            FUN_004ccfed(local_40,local_3c,local_38,local_34,local_30,local_2c);
          }
          FUN_004310a0(&local_34);
        }
      }
    }
  }
  else {
    param_1[10] = 0;
    (**(code **)(*DAT_013b71e8 + 100))(param_1);
  }
  return 1;
}


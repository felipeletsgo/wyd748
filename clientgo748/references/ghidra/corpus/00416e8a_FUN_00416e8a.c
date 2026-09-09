// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00416e8a | Name: FUN_00416e8a


void __thiscall
FUN_00416e8a(int *param_1,char param_2,char param_3,undefined4 param_4,undefined4 param_5,
            undefined4 param_6)

{
  char cVar1;
  char cVar2;
  undefined1 local_54 [4];
  undefined2 local_50;
  undefined2 local_4e;
  char local_48;
  char cStack_47;
  char cStack_46;
  char cStack_45;
  undefined1 uStack_44;
  undefined1 uStack_43;
  int local_40;
  undefined2 local_3c;
  int local_38;
  short local_34;
  short local_30;
  undefined1 local_2c [4];
  undefined2 local_28;
  undefined2 local_26;
  char local_20;
  char cStack_1f;
  char cStack_1e;
  char cStack_1d;
  undefined1 uStack_1c;
  undefined1 uStack_1b;
  undefined2 local_18;
  undefined4 local_14;
  undefined4 local_10;
  undefined4 local_c;
  uint local_8;
  
  local_c = (**(code **)(*DAT_005ccf08 + 0x84))();
  local_10 = __ftol();
  local_14 = __ftol();
  local_8 = 0;
  if (DAT_005ccf08 != (int *)0x0) {
    FUN_0058f220(param_6,DAT_005ccf08[0x19c],8);
    local_8 = FUN_0054cd07(param_6,0x11);
  }
  if (param_1[0x79] == 0) {
    local_18 = (**(code **)(*param_1 + 0xc0))
                         (*(undefined4 *)(DAT_005ccf08[0x79] + 0x1e4),
                          *(undefined4 *)(DAT_005ccf08[0x79] + 0x400));
    local_30 = (**(code **)(*param_1 + 0xc4))(*(undefined4 *)(DAT_005ccf08[0x79] + 0x1e4));
    if (local_30 == -1) {
      local_30 = (short)DAT_005ccf08[0x74] + (short)DAT_005ccf08[0x75] * 9;
    }
    _memset(local_2c,0,0x14);
    local_26 = *(undefined2 *)(DAT_013b71e8 + 0xc5c);
    local_28 = 0x376;
    local_20 = (**(code **)(*param_1 + 0xc0))(param_1[0x79],param_1[0x100]);
    cStack_1f = param_3 * '\t' + param_2;
    cStack_1e = (char)local_18;
    cStack_1d = (char)local_30;
    uStack_1c = (undefined1)DAT_005d03e8;
    uStack_1b = (undefined1)((ushort)DAT_005d03e8 >> 8);
    if (((char)local_30 != cStack_1f) || (local_20 != (char)local_18)) {
      FUN_0055f2dd(local_2c,0x14);
    }
  }
  else if ((local_8 & param_1[0x79]) == param_1[0x79]) {
    cVar1 = (**(code **)(*param_1 + 0xc0))(param_1[0x79],param_1[0x100]);
    local_3c = (**(code **)(*param_1 + 0xc4))(param_1[0x79]);
    local_34 = (**(code **)(*param_1 + 0xc0))
                         (*(undefined4 *)(DAT_005ccf08[0x79] + 0x1e4),
                          *(undefined4 *)(DAT_005ccf08[0x79] + 0x400));
    cVar2 = (**(code **)(*param_1 + 0xc4))(*(undefined4 *)(DAT_005ccf08[0x79] + 0x1e4));
    local_38 = DAT_005ccf08[0x74];
    local_40 = DAT_005ccf08[0x75];
    _memset(local_54,0,0x14);
    local_4e = *(undefined2 *)(DAT_013b71e8 + 0xc5c);
    local_50 = 0x376;
    cStack_47 = (char)local_3c;
    cStack_46 = (char)local_34;
    uStack_44 = (undefined1)DAT_005d03e8;
    uStack_43 = (undefined1)((ushort)DAT_005d03e8 >> 8);
    cStack_45 = cVar2;
    if (local_34 != 0) {
      cStack_45 = (char)local_40 * '\t' + (char)local_38;
    }
    if ((cStack_45 != (char)local_3c) || (cVar1 != (char)local_34)) {
      local_48 = cVar1;
      FUN_0055f2dd(local_54,0x14);
    }
  }
  DAT_005ccf08 = (int *)0x0;
  return;
}


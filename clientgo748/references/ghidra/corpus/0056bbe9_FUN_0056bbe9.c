// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0056bbe9 | Name: FUN_0056bbe9


void __fastcall FUN_0056bbe9(int *param_1)

{
  int *piVar1;
  int *piVar2;
  undefined1 local_28 [8];
  int local_20;
  int local_1c;
  int local_18;
  int local_14;
  int local_10;
  int local_c;
  int local_8;
  
  local_10 = param_1[0x416];
  local_14 = param_1[0x419];
  local_18 = param_1[6];
  local_1c = param_1[0xb];
  local_20 = param_1[4];
  param_1[0x419] = param_1[0x41a];
  param_1[0x416] = 1;
  param_1[6] = (int)local_28;
  param_1[0xb] = (int)&DAT_005c57c8;
  param_1[4] = 0;
  if (param_1[2] != 1) {
    piVar1 = param_1 + 0x412;
    local_8 = *piVar1;
    piVar2 = param_1 + 0x413;
    local_c = *piVar2;
    *piVar1 = 1;
    *piVar2 = (int)(param_1 + 7);
    FUN_0056b152(param_1 + 7);
    *piVar1 = local_8;
    *piVar2 = local_c;
  }
  (**(code **)(*param_1 + 8))(0,0,param_1 + 7);
  (**(code **)(*param_1 + 4))(0,0,param_1 + 7);
  param_1[0x416] = local_10;
  param_1[0x419] = local_14;
  param_1[6] = local_18;
  param_1[0xb] = local_1c;
  param_1[4] = local_20;
  return;
}


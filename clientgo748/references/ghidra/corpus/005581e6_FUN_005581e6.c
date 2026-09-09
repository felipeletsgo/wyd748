// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 005581e6 | Name: FUN_005581e6


undefined4 FUN_005581e6(char *param_1)

{
  int iVar1;
  int local_108;
  undefined1 local_104 [256];
  
  FUN_0058ee20(local_104,param_1);
  local_108 = 0;
  while( true ) {
    if (0x3ff < local_108) {
      return 0;
    }
    iVar1 = _strcmp(param_1,&DAT_00e22f2c + local_108 * 0x40);
    if (iVar1 == 0) break;
    local_108 = local_108 + 1;
  }
  FUN_0058ee20(param_1,&DAT_00e22f4c + local_108 * 0x40);
  FUN_0058ee20(&DAT_00e22f5c + local_108 * 0x40,&DAT_005c4840);
  return 1;
}


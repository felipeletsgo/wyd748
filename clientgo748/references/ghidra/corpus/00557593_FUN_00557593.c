// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00557593 | Name: FUN_00557593


undefined4 FUN_00557593(int param_1,int param_2)

{
  undefined4 uVar1;
  int iVar2;
  size_t local_1420;
  int local_141c;
  int local_1418;
  int local_1414;
  int local_1410;
  undefined4 local_140c;
  char local_1408 [11];
  undefined1 auStackY_13fd [4085];
  int local_408;
  CHAR local_404 [984];
  undefined4 uStackY_2c;
  CHAR *pCStackY_28;
  char *pcStackY_24;
  int *piStackY_20;
  int *piStackY_1c;
  int *piStackY_18;
  
  FUN_0058f730();
  local_408 = 0;
  if (param_2 == 0) {
    local_408 = FUN_0058f716();
  }
  else {
    local_408 = FUN_0058f716();
  }
  if (local_408 == 0) {
    piStackY_18 = (int *)0x557601;
    MessageBoxA((HWND)0x0,local_404,s_Can_t_parse_string_in_Guilds_005c473c,0);
    uVar1 = 0;
  }
  else {
    _memset(&DAT_00a41d78,0,0x300000);
    while( true ) {
      local_1414 = -1;
      local_141c = -1;
      local_1418 = -1;
      _memset(local_1408,0,0x1000);
      local_140c = FUN_00590a06();
      if (local_140c == 0) break;
      piStackY_18 = &local_1418;
      piStackY_1c = &local_141c;
      piStackY_20 = &local_1414;
      pcStackY_24 = s__d__d__d__s__s__s__s_005c475c;
      pCStackY_28 = local_404;
      uStackY_2c = 0x5576ce;
      FUN_005909d2();
      if (((((local_1414 < 0) || (9 < local_1414)) || (local_141c < 0)) ||
          ((0xf < local_141c || (local_1418 < 0)))) ||
         ((0xfff < local_1418 || (&stack0x00000000 == (undefined1 *)0x1408)))) {
        return 0;
      }
      if (local_1414 == param_1) {
        for (local_1410 = 0; local_1410 < 4; local_1410 = local_1410 + 1) {
          local_1420 = _strlen(local_1408 + local_1410 * 0x400);
          if (0xb < (int)local_1420) {
            auStackY_13fd[local_1410 * 0x400] = 0;
            local_1420 = 0xb;
          }
          if ((0 < (int)local_1420) && (iVar2 = FUN_00557556(), iVar2 == 0)) {
            local_1408[local_1420 + local_1410 * 0x400 + -1] = '\0';
          }
          FUN_0058ee20();
        }
      }
    }
    FUN_0058f62c();
    DAT_005be33c = param_1;
    uVar1 = 1;
  }
  return uVar1;
}


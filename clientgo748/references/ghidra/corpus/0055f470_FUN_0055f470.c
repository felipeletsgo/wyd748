// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0055f470 | Name: FUN_0055f470


undefined4 FUN_0055f470(char *param_1)

{
  char cVar1;
  char cVar2;
  size_t sVar3;
  undefined4 uVar4;
  int local_2c;
  char local_20 [6];
  undefined1 local_1a;
  char local_19;
  char *local_8;
  
  _memset(local_20,0,0x18);
  sVar3 = _strlen(param_1);
  if ((int)sVar3 < 4) {
    uVar4 = 0;
  }
  else if ((int)sVar3 < 0xd) {
    local_8 = param_1;
    for (local_2c = 0; cVar1 = local_19, local_2c < (int)sVar3; local_2c = local_2c + 1) {
      if ((local_8[local_2c] < '0') || ('9' < local_8[local_2c])) {
        if ((local_8[local_2c] < 'A') || ('Z' < local_8[local_2c])) {
          if ((local_8[local_2c] < 'a') || ('z' < local_8[local_2c])) {
            return 0;
          }
          cVar2 = (char)(local_8[local_2c] + -0x61);
          cVar1 = cVar2 - (char)((local_8[local_2c] + -0x61) / 2);
          local_20[local_2c * 2] = cVar1 + 'a';
          local_20[local_2c * 2 + 1] = (cVar2 - cVar1) + 'a';
        }
        else {
          cVar2 = (char)(local_8[local_2c] + -0x41);
          cVar1 = cVar2 - (char)((local_8[local_2c] + -0x41) / 2);
          local_20[local_2c * 2] = cVar1 + 'A';
          local_20[local_2c * 2 + 1] = (cVar2 - cVar1) + 'A';
        }
      }
      else {
        cVar2 = (char)(local_8[local_2c] + -0x30);
        cVar1 = cVar2 - (char)((local_8[local_2c] + -0x30) / 2);
        local_20[local_2c * 2] = cVar1 + '0';
        local_20[local_2c * 2 + 1] = (cVar2 - cVar1) + '0';
      }
    }
    local_20[3] = local_1a;
    local_19 = local_20[0];
    local_20[0] = cVar1;
    FUN_0058f220(param_1,local_20,sVar3 << 1);
    uVar4 = 1;
  }
  else {
    uVar4 = 0;
  }
  return uVar4;
}


// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00417215 | Name: FUN_00417215


undefined4 FUN_00417215(int param_1)

{
  char cVar1;
  char cVar2;
  int iVar3;
  undefined4 uVar4;
  int iVar5;
  int local_20;
  undefined4 local_1c;
  int local_10;
  undefined4 local_8;
  
  if (param_1 == 0) {
    local_8 = 0xff99ee99;
  }
  else {
    local_8 = 0xffaaaaff;
    cVar2 = '\0';
    for (local_10 = 0; local_10 < 0x31; local_10 = local_10 + 1) {
      cVar1 = (&DAT_005b1114)[local_10 * 4];
      if (cVar1 == '-') {
        cVar1 = 'E';
      }
      if (cVar1 == '.') {
        cVar1 = 'F';
      }
      iVar3 = FUN_0054cd07(param_1,0x11);
      if (((cVar1 == '*') || (cVar1 == '5')) || ((iVar3 == 0x20 && (cVar1 == '\x02')))) {
        local_1c = FUN_0054cd07(param_1,cVar1);
        local_20 = FUN_0054c799(param_1,cVar1);
      }
      else {
        local_1c = FUN_0054daac(param_1,cVar1);
        local_20 = FUN_0054dd2f(param_1,cVar1);
      }
      uVar4 = FUN_004175eb(iVar3,cVar1,local_20);
      if ((cVar2 == '\0') || (cVar2 != cVar1)) {
        if (local_20 != 0) {
          iVar3 = FUN_004171c6(local_8);
          iVar5 = FUN_004171c6(uVar4);
          if (iVar3 < iVar5) {
            local_8 = uVar4;
            cVar2 = cVar1;
          }
        }
      }
      else {
        local_8 = FUN_004175eb(iVar3,cVar1,local_1c);
      }
    }
  }
  return local_8;
}


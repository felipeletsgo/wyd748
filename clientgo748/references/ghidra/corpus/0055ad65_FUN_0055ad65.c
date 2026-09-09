// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0055ad65 | Name: FUN_0055ad65


undefined4 FUN_0055ad65(char *param_1,undefined4 param_2)

{
  size_t sVar1;
  undefined4 uVar2;
  int iVar3;
  undefined4 *puVar4;
  undefined1 local_4c;
  undefined4 local_4b [15];
  int local_c;
  undefined4 local_8;
  
  sVar1 = _strlen(param_1);
  if (sVar1 == 0) {
    uVar2 = 0;
  }
  else {
    local_8 = 0x3c;
    local_4c = DAT_00e38534;
    puVar4 = local_4b;
    for (iVar3 = 0xf; iVar3 != 0; iVar3 = iVar3 + -1) {
      *puVar4 = 0;
      puVar4 = puVar4 + 1;
    }
    local_c = FUN_0055a99d(param_1,&local_4c);
    if ((local_c == -1) || (0x3c < local_c)) {
      uVar2 = 0;
    }
    else {
      FUN_0055a170(&local_4c,&local_4c,local_c,&DAT_005a5dac,0xe,1);
      FUN_0058ee20(param_2,&local_4c);
      uVar2 = 1;
    }
  }
  return uVar2;
}


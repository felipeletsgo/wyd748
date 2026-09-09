// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0055adf6 | Name: FUN_0055adf6


undefined4 FUN_0055adf6(char *param_1,undefined4 param_2)

{
  size_t sVar1;
  undefined4 uVar2;
  int iVar3;
  undefined4 *puVar4;
  undefined1 local_11c;
  undefined4 local_11b [34];
  undefined1 local_90;
  undefined4 local_8f [34];
  
  sVar1 = _strlen(param_1);
  if (((int)sVar1 < 0x3d) && (sVar1 != 0)) {
    local_90 = DAT_00e38538;
    puVar4 = local_8f;
    for (iVar3 = 0x22; iVar3 != 0; iVar3 = iVar3 + -1) {
      *puVar4 = 0;
      puVar4 = puVar4 + 1;
    }
    *(undefined1 *)puVar4 = 0;
    local_11c = DAT_00e3853c;
    puVar4 = local_11b;
    for (iVar3 = 0x22; iVar3 != 0; iVar3 = iVar3 + -1) {
      *puVar4 = 0;
      puVar4 = puVar4 + 1;
    }
    *(undefined1 *)puVar4 = 0;
    FUN_0058ee20(&local_90,param_1);
    iVar3 = ((int)(sVar1 + 7 + ((int)(sVar1 + 7) >> 0x1f & 7U)) >> 3) << 3;
    FUN_0055a170(&local_11c,&local_90,iVar3,&DAT_005a5dac,0xe,0);
    _memset(&local_90,0,0x8a);
    FUN_0055aba9(&local_11c,iVar3,&local_90);
    FUN_0058ee20(param_2,&local_90);
    uVar2 = 1;
  }
  else {
    uVar2 = 0;
  }
  return uVar2;
}


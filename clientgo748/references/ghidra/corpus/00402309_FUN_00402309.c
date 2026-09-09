// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00402309 | Name: FUN_00402309


void __thiscall FUN_00402309(int param_1,char *param_2,undefined4 param_3)

{
  int iVar1;
  size_t sVar2;
  int local_18;
  int local_14;
  int local_c;
  
  if (param_2 != (char *)0x0) {
    FUN_0054fc81(param_2);
    iVar1 = _strcmp(param_2,&DAT_005b109c);
    if (iVar1 == 0) {
      param_2 = (char *)0x0;
    }
  }
  if (param_2 == (char *)0x0) {
    FUN_0058f078(param_1 + 0x76a,&DAT_005cced4);
    FUN_00421b8f(param_1 + 0x76a,*(undefined4 *)(param_1 + 0x754),param_3);
  }
  else {
    iVar1 = _strcmp((char *)(param_1 + 0x76a),param_2);
    if ((iVar1 == 0) && (*(char *)(param_1 + 0xcdd) == '\0')) {
      return;
    }
    sVar2 = _strlen(param_2);
    if (0xff < (int)sVar2) {
      param_2[0xff] = '\0';
    }
    FUN_0058ee20(param_1 + 0x76a,param_2);
    FUN_00421b8f(param_1 + 0x76a,*(undefined4 *)(param_1 + 0x754),param_3);
  }
  if (*(char *)(param_1 + 0xcdd) == '\x01') {
    sVar2 = _strlen(param_2);
    for (local_14 = 0; (local_14 < (int)sVar2 && (param_2[local_14] == ' '));
        local_14 = local_14 + 1) {
    }
    _memset((void *)(param_1 + 0x76a),0,0x100);
    _memset((void *)(param_1 + 0x8d6),0,0x100);
    _memset((void *)(param_1 + 0xa42),0,0x100);
    _memset((void *)(param_1 + 0xbae),0,0x100);
    local_18 = 9;
    if ((int)sVar2 < 9) {
      local_18 = 7;
    }
    for (local_c = 0; (local_c < (int)sVar2 && (local_c < 0xff)); local_c = local_c + 1) {
      if ((local_18 - local_c) / 3 == 0) {
        *(char *)(param_1 + local_c + 0x76a) = param_2[local_c];
      }
      else {
        *(undefined1 *)(param_1 + local_c + 0x76a) = 0x20;
      }
      if ((local_18 - local_c) / 3 == 1) {
        *(char *)(param_1 + local_c + 0x8d6) = param_2[local_c];
      }
      else {
        *(undefined1 *)(param_1 + local_c + 0x8d6) = 0x20;
      }
      if ((local_18 - local_c) / 3 == 2) {
        *(char *)(param_1 + local_c + 0xa42) = param_2[local_c];
      }
      else {
        *(undefined1 *)(param_1 + local_c + 0xa42) = 0x20;
      }
      if ((local_18 - local_c) / 3 == 3) {
        *(char *)(param_1 + local_c + 0xbae) = param_2[local_c];
      }
      else {
        *(undefined1 *)(param_1 + local_c + 0xbae) = 0x20;
      }
    }
    FUN_00421b8f(param_1 + 0x76a,0xffddffdd,param_3);
    FUN_00421b8f(param_1 + 0x8d6,0xff88ff88,param_3);
    FUN_00421b8f(param_1 + 0xa42,0xffffff66,param_3);
    FUN_00421b8f(param_1 + 0xbae,0xffffffff,param_3);
  }
  return;
}


// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00556d9b | Name: FUN_00556d9b


/* WARNING: Type propagation algorithm not settling */

undefined4 FUN_00556d9b(void)

{
  size_t sVar1;
  undefined4 uVar2;
  int local_21c;
  size_t local_218;
  int local_210;
  char acStack_209 [257];
  undefined1 local_108 [256];
  int local_8;
  
  _memset((void *)((int)acStack_209 + 1),0,0x100);
  _memset(local_108,0,0x100);
  GetModuleFileNameA((HMODULE)0x0,(LPSTR)((int)acStack_209 + 1),0x100);
  local_218 = _strlen((char *)((int)acStack_209 + 1));
  do {
    sVar1 = local_218;
    local_218 = sVar1 - 1;
    if ((int)local_218 < 1) goto LAB_00556e5b;
  } while (acStack_209[sVar1] != '\\');
  FUN_0058f078(local_108,&DAT_005c45b0,(int)acStack_209 + sVar1 + 1);
LAB_00556e5b:
  local_8 = 0x300;
  local_21c = FUN_0058f716(local_108,&DAT_005c45b4);
  if (local_21c == 0) {
    local_21c = FUN_0058f716(s_WYD_EXE_005c45bc,&DAT_005c45b8);
  }
  if (local_21c == 0) {
    MessageBoxA((HWND)0x0,s_Can_t_read_inititem_005c45cc,s_ERROR_005c45c4,0x1000);
    uVar2 = 0;
  }
  else {
    FUN_00591a3e(local_21c,-local_8,2);
    FUN_00590d68(&DAT_00e22c00,local_8,1,local_21c);
    FUN_0058f62c(local_21c);
    for (local_210 = 0; local_210 < local_8; local_210 = local_210 + 1) {
      *(byte *)((int)&DAT_00e22c00 + local_210) = *(byte *)((int)&DAT_00e22c00 + local_210) ^ 0xff;
    }
    for (local_210 = 0; local_210 < 0x60; local_210 = local_210 + 1) {
      if ((short)(&DAT_00e22c00)[local_210 * 4] < 1) {
        DAT_00e37d2c = local_210;
        break;
      }
    }
    uVar2 = 1;
  }
  return uVar2;
}


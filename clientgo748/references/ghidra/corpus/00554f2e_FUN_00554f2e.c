// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00554f2e | Name: FUN_00554f2e


void FUN_00554f2e(void)

{
  int iVar1;
  size_t sVar2;
  int local_390;
  int local_38c;
  char local_388 [256];
  undefined1 local_288 [128];
  undefined1 local_208 [128];
  undefined1 local_188 [127];
  char acStack_109 [129];
  char local_88 [128];
  int local_8;
  
  for (local_38c = 0; local_38c < 0x7f; local_38c = local_38c + 1) {
    (&DAT_00e32f2c)[local_38c * 0x18] = 0;
  }
  local_390 = FUN_0058f716(s_ItemEffect_h_005c4498,&DAT_005c4494);
  if (local_390 == 0) {
    local_390 = FUN_0058f716(s_______TMSRV_Run_ItemEffect_h_005c44ac,&DAT_005c44a8);
  }
  if (local_390 == 0) {
    MessageBoxA((HWND)0x0,s_ItemEffect_h_005c44e8,s_there_is_no_effect_file__005c44cc,0);
  }
  else {
    local_8 = 0;
    while (iVar1 = FUN_00590a06(local_388,0xff,local_390), iVar1 != 0) {
      if (local_388[0] != '\r') {
        local_88[0] = '\0';
        FUN_005909d2(local_388,&DAT_005c44f8,local_88);
        iVar1 = _strcmp(local_88,s__include_005c44fc);
        if (iVar1 == 0) {
          FUN_005909d2(local_388,s__s___s_005c4508,local_88,acStack_109 + 1);
          sVar2 = _strlen(acStack_109 + 1);
          acStack_109[sVar2] = '\0';
          FUN_00554f2e();
        }
        else {
          iVar1 = _strcmp(local_88,s__define_005c4510);
          if (iVar1 == 0) {
            FUN_005909d2(local_388,s__s__s__s__s__s_005c4518,local_88,acStack_109 + 1,local_188,
                         local_208,local_288);
            if (local_8 == 0) {
              local_8 = 1;
            }
            else {
              iVar1 = FUN_0059115a(local_188);
              if ((iVar1 < 1) || (0x7e < iVar1)) {
                MessageBoxA((HWND)0x0,local_388,&DAT_005c4528,0);
              }
              else if ((&DAT_00e32f2c)[iVar1 * 0x18] == '\0') {
                FUN_0058f220(&DAT_00e32f2c + iVar1 * 0x18,acStack_109 + 1,0x18);
              }
              else {
                MessageBoxA((HWND)0x0,local_388,&DAT_00e32f2c + iVar1 * 0x18,0);
              }
            }
          }
        }
      }
    }
    FUN_0058f62c(local_390);
  }
  return;
}


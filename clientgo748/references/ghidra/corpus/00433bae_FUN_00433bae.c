// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00433bae | Name: FUN_00433bae


void FUN_00433bae(void)

{
  int iVar1;
  int local_90;
  undefined4 local_8c;
  undefined1 local_88 [128];
  int local_8;
  
  FUN_0058f078(local_88,s_minimap_dat_005b7860);
  local_8 = 0;
  local_8 = FUN_0058f716(local_88,&DAT_005b786c);
  _memset(&DAT_005ccfb8,0,0x3400);
  if (local_8 != 0) {
    local_8c = 0;
    local_90 = 0;
    while ((local_90 < 0x100 &&
           (iVar1 = FUN_00590a68(local_8,s__d__d__d__d__d__s_005b7870,
                                 &DAT_005ccfb8 + local_90 * 0x34,&DAT_005ccfbc + local_90 * 0x34,
                                 &local_8c,&DAT_005ccfc0 + local_90 * 0x34,
                                 &DAT_005ccfc4 + local_90 * 0x34,&DAT_005ccfcc + local_90 * 0x34),
           iVar1 != -1))) {
      switch(local_8c) {
      case 1:
        *(undefined4 *)(&DAT_005ccfc8 + local_90 * 0x34) = 0xffffffff;
        break;
      case 2:
        *(undefined4 *)(&DAT_005ccfc8 + local_90 * 0x34) = 0xff44aa44;
        break;
      case 3:
        *(undefined4 *)(&DAT_005ccfc8 + local_90 * 0x34) = 0xff5555ff;
        break;
      case 4:
        *(undefined4 *)(&DAT_005ccfc8 + local_90 * 0x34) = 0xffaa00ff;
      }
      if (0xff < local_90) {
        return;
      }
      local_90 = local_90 + 1;
    }
  }
  return;
}


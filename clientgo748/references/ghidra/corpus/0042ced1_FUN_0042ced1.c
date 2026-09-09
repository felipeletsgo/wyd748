// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0042ced1 | Name: FUN_0042ced1


void __fastcall FUN_0042ced1(int param_1)

{
  HDC pHVar1;
  HBITMAP pHVar2;
  HFONT pHVar3;
  undefined1 local_20c [256];
  int local_10c;
  int local_108;
  CHAR local_104 [256];
  
  if (*(int *)(param_1 + 0x2a85c) == 0) {
    _memset((void *)(param_1 + 0x2a860),0,0x28);
    *(undefined4 *)(param_1 + 0x2a860) = 0x28;
    *(int *)(param_1 + 0x2a864) = DAT_005b4918;
    *(int *)(param_1 + 0x2a868) = -DAT_005b4918;
    *(undefined2 *)(param_1 + 0x2a86c) = 1;
    *(undefined4 *)(param_1 + 0x2a870) = 0;
    *(undefined2 *)(param_1 + 0x2a86e) = 0x20;
    pHVar1 = CreateCompatibleDC((HDC)0x0);
    *(HDC *)(param_1 + 0x2a858) = pHVar1;
    pHVar2 = CreateDIBSection(*(HDC *)(param_1 + 0x2a858),(BITMAPINFO *)(param_1 + 0x2a860),0,
                              (void **)(param_1 + 0x2a88c),(HANDLE)0x0,0);
    *(HBITMAP *)(param_1 + 0x2a85c) = pHVar2;
    local_108 = 500;
    if (DAT_005b892c == 2) {
      DAT_005b4914 = 0xd;
      FUN_0058f078(local_104,s_Tahoma_005b4974);
    }
    else {
      DAT_005b4914 = 0xe;
      FUN_0058f078(local_104,s_Arial_005b497c);
    }
    local_10c = FUN_0058f716(s_Font_txt_005b4988,&DAT_005b4984);
    if (local_10c != 0) {
      FUN_00590a06(local_20c,0x100,local_10c);
      FUN_005909d2(local_20c,&DAT_005b4994,local_104);
      FUN_00590a06(local_20c,0x100,local_10c);
      FUN_005909d2(local_20c,&DAT_005b4998,&local_108);
      if (local_108 < 1) {
        local_108 = 500;
      }
      FUN_0058f62c(local_10c);
    }
    pHVar3 = CreateFontA(DAT_005b4914,0,0,0,local_108,0,0,0,1,0,0,4,2,local_104);
    *(HFONT *)(param_1 + 0x2a854) = pHVar3;
    SelectObject(*(HDC *)(param_1 + 0x2a858),*(HGDIOBJ *)(param_1 + 0x2a85c));
    SelectObject(*(HDC *)(param_1 + 0x2a858),*(HGDIOBJ *)(param_1 + 0x2a854));
    SetTextColor(*(HDC *)(param_1 + 0x2a858),0xffffff);
    SetBkColor(*(HDC *)(param_1 + 0x2a858),0);
  }
  return;
}


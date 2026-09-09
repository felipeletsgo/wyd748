// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00539820 | Name: FUN_00539820


undefined4 __fastcall FUN_00539820(int param_1)

{
  int iVar1;
  int iVar2;
  undefined4 uVar3;
  uint uVar4;
  
  iVar1 = __ftol();
  iVar2 = __ftol();
  if (*(char *)(param_1 + 0x100cc + iVar1 + iVar2 * 0x80) == '\x01') {
    uVar3 = 1;
  }
  else {
    uVar4 = *(byte *)(param_1 + 0xcd + (iVar1 / 2 + (iVar2 / 2) * 0x40) * 0xc) + 10;
    if (((((uVar4 < 0xe) || (0x11 < uVar4)) && ((uVar4 < 0x26 || (0x4d < uVar4)))) &&
        ((uVar4 < 0x56 || (0x65 < uVar4)))) && ((uVar4 < 0x82 || (0x95 < uVar4)))) {
      if ((uVar4 < 0xba) || (0xc1 < uVar4)) {
        if ((uVar4 < 0xca) || (0xcd < uVar4)) {
          if ((uVar4 < 0xe6) || (0xe7 < uVar4)) {
            uVar3 = 3;
          }
          else {
            uVar3 = 9;
          }
        }
        else {
          uVar3 = 8;
        }
      }
      else {
        uVar3 = 0xb;
      }
    }
    else {
      uVar3 = 0;
    }
  }
  return uVar3;
}


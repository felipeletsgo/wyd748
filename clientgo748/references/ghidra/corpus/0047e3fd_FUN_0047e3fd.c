// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0047e3fd | Name: FUN_0047e3fd


undefined4 __thiscall FUN_0047e3fd(undefined4 param_1,int param_2)

{
  undefined4 uVar1;
  undefined1 local_40 [4];
  int local_3c;
  int local_38;
  uint local_32;
  short local_14 [5];
  int local_a;
  
  if (param_2 == 0) {
    uVar1 = 0;
  }
  else {
    FUN_0058f220(local_14,param_2,0xe,param_1);
    if (local_14[0] == 0x4d42) {
      FUN_0058f220(local_40,param_2 + 0xe,local_a + -0xe,param_1);
      if ((local_3c == 0x10) && (local_38 == 0xc)) {
        if ((local_32 & 0xffff) == 0x18) {
          uVar1 = 1;
        }
        else {
          uVar1 = 0;
        }
      }
      else {
        uVar1 = 0;
      }
    }
    else {
      uVar1 = 0;
    }
  }
  return uVar1;
}


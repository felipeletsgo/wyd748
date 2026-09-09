// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004f9334 | Name: FUN_004f9334


void __fastcall FUN_004f9334(int param_1)

{
  undefined4 uVar1;
  int local_8;
  
  if (*(int *)(param_1 + 0x430) != 0) {
    if (*(undefined4 **)(param_1 + 0x430) != (undefined4 *)0x0) {
      (**(code **)**(undefined4 **)(param_1 + 0x430))(1);
    }
    *(undefined4 *)(param_1 + 0x430) = 0;
  }
  *(undefined4 *)(param_1 + 0x254) = 0xffffffff;
  *(undefined1 *)(param_1 + 0x245) = 0;
  *(undefined4 *)(param_1 + 0x238) = 0;
  *(undefined1 *)(param_1 + 0x248) = 0;
  *(undefined4 *)(param_1 + 0x778) = 0;
  *(undefined4 *)(param_1 + 0x75c) = 0xffffffff;
  *(undefined4 *)(param_1 + 0x764) = 0;
  *(undefined4 *)(param_1 + 0x760) = 0;
  *(undefined1 *)(param_1 + 0x77c) = 0;
  *(undefined1 *)(param_1 + 0x77d) = 0;
  *(undefined1 *)(param_1 + 0x77e) = 0;
  *(undefined1 *)(param_1 + 0x77f) = 0;
  *(undefined1 *)(param_1 + 0x780) = 0;
  *(undefined1 *)(param_1 + 0x781) = 0;
  *(undefined1 *)(param_1 + 0x783) = 0;
  *(undefined1 *)(param_1 + 0x784) = 0;
  *(undefined1 *)(param_1 + 0x785) = 0;
  *(undefined1 *)(param_1 + 0x788) = 0;
  *(undefined1 *)(param_1 + 0x786) = 0;
  *(undefined1 *)(param_1 + 0x787) = 0;
  *(undefined1 *)(param_1 + 0x789) = 0;
  *(undefined1 *)(param_1 + 0x78a) = 0;
  *(undefined1 *)(param_1 + 0x78b) = 0;
  *(undefined1 *)(param_1 + 0x78c) = 0;
  *(undefined1 *)(param_1 + 0x78d) = 0;
  *(undefined1 *)(param_1 + 0x797) = 0;
  *(undefined1 *)(param_1 + 0x78e) = 0;
  *(undefined1 *)(param_1 + 0x791) = 0;
  *(undefined1 *)(param_1 + 0x78f) = 0;
  *(undefined1 *)(param_1 + 0x790) = 0;
  *(undefined1 *)(param_1 + 0x793) = 0;
  *(undefined1 *)(param_1 + 0x794) = 0;
  *(undefined1 *)(param_1 + 0x792) = 0;
  *(undefined1 *)(param_1 + 0x796) = 0;
  *(undefined1 *)(param_1 + 0x798) = 0;
  *(undefined1 *)(param_1 + 0x799) = 0;
  FUN_00524324(0);
  *(undefined4 *)(param_1 + 0x24) = 3;
  *(undefined4 *)(param_1 + 0x740) = 0xffffffff;
  *(undefined4 *)(param_1 + 0x73c) = 0xffffffff;
  for (local_8 = 0; local_8 < 4; local_8 = local_8 + 1) {
    *(undefined4 *)(param_1 + 0x744 + local_8 * 4) = 0xffffffff;
  }
  *(undefined4 *)(param_1 + 0x754) = 0xffffffff;
  *(undefined4 *)(param_1 + 0x220) = 0x40000000;
  *(undefined4 *)(param_1 + 0x25c) = 0;
  uVar1 = FUN_00430f20(0,0);
  FUN_0040bd30(uVar1);
  uVar1 = FUN_00430f20(0,0);
  FUN_0040bd30(uVar1);
  uVar1 = FUN_00430f50(0,0,0);
  FUN_004310a0(uVar1);
  uVar1 = FUN_00430f20(0,0);
  FUN_0040bd30(uVar1);
  *(undefined4 *)(param_1 + 0x234) = 0;
  *(undefined4 *)(param_1 + 0x47c) = 0xffffffff;
  *(undefined4 *)(param_1 + 0x480) = 0xffffffff;
  *(undefined4 *)(param_1 + 0x5ec) = 0;
  *(undefined4 *)(param_1 + 0x60c) = 0;
  *(undefined4 *)(param_1 + 0x610) = 0;
  *(undefined4 *)(param_1 + 0x614) = 0;
  *(undefined4 *)(param_1 + 0x5f0) = 0;
  *(undefined4 *)(param_1 + 0x5f4) = 0;
  *(undefined4 *)(param_1 + 0x5f8) = 0;
  *(undefined4 *)(param_1 + 0x5fc) = 0;
  *(undefined4 *)(param_1 + 0x624) = 0;
  *(undefined4 *)(param_1 + 0x628) = 0;
  *(undefined4 *)(param_1 + 0x630) = 0;
  *(undefined4 *)(param_1 + 0x634) = 0;
  *(undefined4 *)(param_1 + 0x638) = 0;
  *(undefined4 *)(param_1 + 0x600) = 0;
  *(undefined4 *)(param_1 + 0x620) = 0;
  *(undefined4 *)(param_1 + 0x604) = 0;
  *(undefined4 *)(param_1 + 0x62c) = 0;
  *(undefined4 *)(param_1 + 0x648) = 0;
  *(undefined4 *)(param_1 + 0x608) = 0;
  *(undefined4 *)(param_1 + 0x7c0) = 0;
  *(undefined4 *)(param_1 + 0x61c) = 3000;
  *(undefined4 *)(param_1 + 0x640) = 0;
  *(undefined4 *)(param_1 + 0x5e8) = 1;
  *(undefined4 *)(param_1 + 0x410) = 1;
  *(undefined4 *)(param_1 + 0x230) = 0;
  *(undefined4 *)(param_1 + 0x414) = 0x2f;
  *(undefined2 *)(param_1 + 0x66c) = 0;
  *(undefined2 *)(param_1 + 0x66e) = 0;
  *(undefined4 *)(param_1 + 0x670) = 0;
  *(undefined4 *)(param_1 + 0x674) = 0;
  *(undefined4 *)(param_1 + 0x678) = 0;
  *(undefined4 *)(param_1 + 0x67c) = 0;
  *(undefined2 *)(param_1 + 0x7b4) = 0xffff;
  *(undefined2 *)(param_1 + 0x7b6) = 0xffff;
  *(undefined4 *)(param_1 + 0x770) = 0;
  _memset((void *)(param_1 + 0x684),0,0x20);
  _memset((void *)(param_1 + 0x6a4),0,0x80);
  _memset((void *)(param_1 + 0x260),0,0x30);
  _memset((void *)(param_1 + 0x1d2),0,0x20);
  _memset((void *)(param_1 + 0x1f2),0,0x10);
  _memset((void *)(param_1 + 0x202),0,0x10);
  _memset((void *)(param_1 + 0x168),0,0x10);
  _memset((void *)(param_1 + 0x178),0,0x1a);
  _memset((void *)(param_1 + 0x444),0,0x1c);
  _memset((void *)(param_1 + 0x4ac),0,0x10);
  _memset((void *)(param_1 + 0x4bc),0,0x18);
  _memset((void *)(param_1 + 0x4d4),0,0x40);
  *(undefined4 *)(param_1 + 0x4d4) = 0x40;
  *(undefined4 *)(param_1 + 0x508) = 0x3d4ccccd;
  *(undefined4 *)(param_1 + 0x50c) = 0x41600000;
  *(undefined4 *)(param_1 + 0x4f8) = 0x3f800000;
  *(undefined4 *)(param_1 + 0x500) = 0x3f800000;
  if (*(int *)(param_1 + 0x42c) != 0) {
    (**(code **)(**(int **)(param_1 + 0x42c) + 0x60))(0);
  }
  return;
}


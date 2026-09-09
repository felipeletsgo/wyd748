// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0054f0f2 | Name: FUN_0054f0f2


uint FUN_0054f0f2(void)

{
  uint uVar1;
  uint uVar2;
  
  _memset(&DAT_00d42700,0,0x1e00);
  _memset(&DAT_00d41d78,0,200);
  FUN_0055784d();
  uVar2 = 1;
  uVar1 = FUN_0054f946(1,0);
  uVar2 = uVar2 & uVar1;
  uVar1 = FUN_0054fa27();
  uVar2 = uVar2 & uVar1;
  uVar1 = FUN_00556d9b();
  uVar2 = uVar2 & uVar1;
  _memset(&DAT_00a41d78,0,0x300000);
  FUN_0054b140();
  FUN_00557943();
  return uVar2;
}


// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004b5bbd | Name: FUN_004b5bbd


void FUN_004b5bbd(undefined4 param_1,undefined4 param_2,undefined4 param_3)

{
  int iVar1;
  undefined4 uVar2;
  undefined1 local_24 [4];
  undefined1 local_20 [20];
  undefined4 local_c;
  undefined4 local_8;
  
  if ((DAT_0069cfc0 & 1) == 0) {
    DAT_0069cfc0 = DAT_0069cfc0 | 1;
    FUN_004b6490(local_20);
    FUN_00591f89(FUN_004b6188);
  }
  FUN_004b5cd8(&DAT_0069cfb0,param_2,param_1,param_3);
  DAT_0069cfac = FUN_004b6540();
  while( true ) {
    iVar1 = FUN_004b6430();
    if (DAT_0069cfac == iVar1) break;
    uVar2 = FUN_004b6400();
    local_8 = FUN_005917f7(uVar2);
    uVar2 = FUN_004b6400();
    local_c = FUN_005917f7(uVar2);
    (*DAT_0092e618)(param_1,0,local_8,local_c,*(undefined4 *)(DAT_0069cfac + 0x20),0,
                    *(undefined4 *)(DAT_0069cfac + 0x24));
    FUN_0058ffd8(local_8);
    FUN_0058ffd8(local_c);
    FUN_004b6270(local_24);
    FUN_004b6310();
    DAT_0069cfac = DAT_0069cfac + 0x28;
  }
  return;
}


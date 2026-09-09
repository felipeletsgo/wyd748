// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 004a3253 | Name: FUN_004a3253


undefined4 FUN_004a3253(int param_1)

{
  int iVar1;
  undefined4 extraout_var;
  undefined4 uVar2;
  undefined1 local_84 [4];
  undefined2 local_80;
  undefined2 local_7e;
  undefined1 local_78 [116];
  
  iVar1 = FUN_0049abde(param_1);
  if ((iVar1 != 1) && (param_1 == 0x2d)) {
    uVar2 = extraout_var;
    _memset(local_84,0,0x80);
    local_7e = *(undefined2 *)(DAT_013b71e8 + 0xc5c);
    local_80 = 0x334;
    FUN_0058f078(local_78,&DAT_005b93fc,uVar2);
    FUN_0042550e(local_84,0x80);
  }
  return 1;
}


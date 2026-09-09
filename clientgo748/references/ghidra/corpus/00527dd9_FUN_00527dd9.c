// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00527dd9 | Name: FUN_00527dd9


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __thiscall FUN_00527dd9(int param_1,int param_2)

{
  int iVar1;
  int iVar2;
  size_t sVar3;
  undefined4 uVar4;
  int local_74;
  int local_70;
  uint local_44;
  int local_40;
  int local_3c;
  char local_38 [48];
  uint local_8;
  
  if (*(char *)(param_1 + 0x245) == '\x01') {
    return 1;
  }
  if (*(int *)(param_2 + 0x14) == 2) {
    *(undefined4 *)(param_1 + 0x23c) = 1;
  }
  *(float *)(param_1 + 0x220) = (float)*(int *)(param_2 + 0x10);
  if (*(int *)(param_2 + 0x10) < 1) {
    *(undefined4 *)(param_1 + 0x220) = 0x3f800000;
  }
  _memset(local_38,0,0x30);
  local_3c = __ftol();
  local_40 = __ftol();
  sVar3 = _strlen((char *)(param_2 + 0x1c));
  if (sVar3 == 0) {
    *(undefined1 *)(param_1 + 0x246) = 1;
  }
  local_44 = 0;
  local_8 = 0;
  if (((local_3c == *(short *)(param_2 + 0xc)) && (local_40 == *(short *)(param_2 + 0xe))) &&
     (*(char *)(param_1 + 0x246) != '\x01')) {
    FUN_0058f220(param_1 + 0x260,param_2 + 0x1c,0x18);
    *(uint *)(param_1 + 0x47c) = (uint)*(ushort *)(param_2 + 0x18);
    *(uint *)(param_1 + 0x480) = (uint)*(ushort *)(param_2 + 0x1a);
    *(undefined1 *)(param_1 + 0x774) = 0;
    FUN_005211be((int)*(short *)(param_2 + 0xc),(int)*(short *)(param_2 + 0xe),param_1 + 0x260,
                 param_1 + 0x290,param_1 + 0x410);
    goto LAB_00528271;
  }
  if (*(short *)(param_2 + 0xc) == local_3c || *(short *)(param_2 + 0xc) - local_3c < 0) {
    local_70 = -(*(short *)(param_2 + 0xc) - local_3c);
  }
  else {
    local_70 = *(short *)(param_2 + 0xc) - local_3c;
  }
  if (local_70 < 0x22) {
    if (*(short *)(param_2 + 0xe) == local_40 || *(short *)(param_2 + 0xe) - local_40 < 0) {
      local_74 = -(*(short *)(param_2 + 0xe) - local_40);
    }
    else {
      local_74 = *(short *)(param_2 + 0xe) - local_40;
    }
    if (0x21 < local_74) goto LAB_00527f5b;
  }
  else {
LAB_00527f5b:
    local_3c = (int)*(short *)(param_2 + 0xc);
    local_40 = (int)*(short *)(param_2 + 0xe);
  }
  local_44 = (uint)*(ushort *)(param_2 + 0x18);
  local_8 = (uint)*(ushort *)(param_2 + 0x1a);
  FUN_00555ad8(local_3c,local_40,&local_44,&local_8,local_38,0xc,DAT_0067cf38 + 0x94,8);
  sVar3 = _strlen(local_38);
  iVar2 = local_3c;
  iVar1 = local_40;
  if (sVar3 == 0) {
    return 1;
  }
  if ((local_44 == *(ushort *)(param_2 + 0x18)) && (local_8 == *(ushort *)(param_2 + 0x1a))) {
    FUN_0058f220(param_1 + 0x260,local_38,0x30);
    *(uint *)(param_1 + 0x47c) = (uint)*(ushort *)(param_2 + 0x18);
    *(uint *)(param_1 + 0x480) = (uint)*(ushort *)(param_2 + 0x1a);
    *(undefined1 *)(param_1 + 0x774) = 0;
    FUN_005211be(local_3c,local_40,param_1 + 0x260,param_1 + 0x290,param_1 + 0x410);
  }
  else if ((DAT_0067cf38 != 0) &&
          ((*(int *)(DAT_0067cf38 + 0x24) == 30000 && (*(int *)(DAT_0067cf38 + 0x4c) != param_1))))
  {
    local_3c = (int)*(short *)(param_2 + 0xc);
    local_40 = (int)*(short *)(param_2 + 0xe);
    local_44 = (uint)*(ushort *)(param_2 + 0x18);
    local_8 = (uint)*(ushort *)(param_2 + 0x1a);
    FUN_00555ad8(local_3c,local_40,&local_44,&local_8,local_38,0xc,DAT_0067cf38 + 0x94,8);
    sVar3 = _strlen(local_38);
    if ((sVar3 != 0) &&
       ((local_44 == *(ushort *)(param_2 + 0x18) && (local_8 == *(ushort *)(param_2 + 0x1a))))) {
      FUN_0058f220(param_1 + 0x260,local_38,0x30);
      *(uint *)(param_1 + 0x47c) = (uint)*(ushort *)(param_2 + 0x18);
      *(uint *)(param_1 + 0x480) = (uint)*(ushort *)(param_2 + 0x1a);
      *(undefined1 *)(param_1 + 0x774) = 0;
      FUN_005211be(local_3c,local_40,param_1 + 0x260,param_1 + 0x290,param_1 + 0x410);
      FUN_00521a90(iVar2,iVar1,param_1 + 0x290,param_1 + 0x410);
    }
  }
LAB_00528271:
  iVar1 = DAT_0067cf38;
  if ((*(char *)(param_1 + 0x246) != '\0') && (param_1 == *(int *)(DAT_0067cf38 + 0x4c))) {
    *(int *)(DAT_0067cf38 + 0x2878c) = (int)*(short *)(param_2 + 0xc);
    *(int *)(iVar1 + 0x28790) = (int)*(short *)(param_2 + 0xe);
    *(uint *)(iVar1 + 0x28794) = (uint)*(ushort *)(param_2 + 0x18);
    *(uint *)(iVar1 + 0x28798) = (uint)*(ushort *)(param_2 + 0x1a);
  }
  FUN_00520fa4(*(undefined4 *)(param_1 + 0x298),*(undefined4 *)(param_1 + 0x29c));
  *(undefined4 *)(param_1 + 0x224) = *(undefined4 *)(param_1 + 0x34);
  uVar4 = (**(code **)(*DAT_0092e654 + 8))();
  *(undefined4 *)(param_1 + 0x5f4) = uVar4;
  return 1;
}


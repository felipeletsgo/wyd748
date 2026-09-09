// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00455ea7 | Name: FUN_00455ea7


undefined4 __thiscall FUN_00455ea7(int param_1,uint param_2)

{
  undefined4 uVar1;
  int iVar2;
  undefined1 local_1c [4];
  undefined2 local_18;
  undefined2 local_16;
  uint local_10;
  undefined2 local_c;
  int local_8;
  
  if (*(char *)(DAT_013b71e8 + 0xef6 + (int)*(char *)(DAT_013b71e8 + 0xef5)) == '\x1f') {
    uVar1 = 0;
  }
  else {
    local_8 = *(int *)(param_1 + 0x80);
    if (local_8 == 0) {
      uVar1 = 0;
    }
    else if (*(int *)(param_1 + 0x27474) + 1000U < param_2) {
      if (*(int *)(local_8 + 0x1b4) == 0) {
        uVar1 = 0;
      }
      else if ((*(short *)(local_8 + 0x160) < 0x6c5) || (0x6c8 < *(short *)(local_8 + 0x160))) {
        if ((*(short *)(local_8 + 0x160) < 0xc49) || (0xc4d < *(short *)(local_8 + 0x160))) {
          iVar2 = FUN_0054cd07(local_8 + 0x160,0x22);
          if (iVar2 < 1) {
            FUN_00455d02(local_8);
          }
          else {
            iVar2 = FUN_0054cd07(local_8 + 0x160,0x22);
            if (((iVar2 == 10) && (0x1003 < *(short *)(local_8 + 0x160))) &&
               (*(short *)(local_8 + 0x160) < 0x1068)) {
              if (*(int *)(*(int *)(param_1 + 0x27be4) + 0x28) == 1) {
                return 1;
              }
              if (*(short *)(local_8 + 0x160) == 0x1006) {
                FUN_0044a745(1,2);
                return 1;
              }
              if (*(short *)(local_8 + 0x160) == 0x1007) {
                FUN_0044a745(1,1);
                return 1;
              }
            }
            uVar1 = 0;
            iVar2 = local_8;
            _memset(local_1c,0,0x14);
            local_16 = *(undefined2 *)(*(int *)(param_1 + 0x4c) + 0x20);
            local_10 = (uint)*(ushort *)(iVar2 + 0x20);
            local_18 = 0x374;
            local_c = 1;
            FUN_0055f2dd(local_1c,0x14,param_1,iVar2,uVar1);
            uVar1 = (**(code **)(*DAT_0092e654 + 8))();
            *(undefined4 *)(param_1 + 0x27474) = uVar1;
          }
          uVar1 = 1;
        }
        else {
          uVar1 = 1;
        }
      }
      else {
        uVar1 = 1;
      }
    }
    else {
      uVar1 = 0;
    }
  }
  return uVar1;
}


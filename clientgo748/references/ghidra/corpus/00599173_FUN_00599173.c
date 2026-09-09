// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 00599173 | Name: FUN_00599173


byte * FUN_00599173(byte *param_1,uint param_2)

{
  int iVar1;
  uint uVar2;
  byte *pbVar3;
  undefined4 local_3c;
  uint local_38;
  byte *local_34;
  undefined4 local_30;
  int local_2c;
  byte *local_28;
  uint local_24;
  void *local_14;
  undefined1 *puStack_10;
  undefined *puStack_c;
  undefined4 local_8;
  
  local_8 = 0xffffffff;
  puStack_c = &DAT_005aa338;
  puStack_10 = &LAB_00596b2c;
  local_14 = ExceptionList;
  pbVar3 = (byte *)0x0;
  if (param_1 == (byte *)0x0) {
    ExceptionList = &local_14;
    pbVar3 = _malloc(param_2);
  }
  else {
    if (param_2 == 0) {
      ExceptionList = &local_14;
      FUN_0058ffd8(param_1);
    }
    else {
      ExceptionList = &local_14;
      if (DAT_013b8608 == 3) {
        do {
          local_28 = (byte *)0x0;
          if (param_2 < 0xffffffe1) {
            FUN_00596aad(9);
            local_8 = 0;
            local_2c = FUN_005958b8(param_1);
            if (local_2c != 0) {
              if (param_2 <= DAT_013b8600) {
                iVar1 = FUN_005960c1(local_2c,param_1,param_2);
                if (iVar1 == 0) {
                  local_28 = (byte *)FUN_00595c0c(param_2);
                  if (local_28 != (byte *)0x0) {
                    local_24 = *(int *)(param_1 + -4) - 1;
                    uVar2 = local_24;
                    if (param_2 <= local_24) {
                      uVar2 = param_2;
                    }
                    FUN_0058f220(local_28,param_1,uVar2);
                    local_2c = FUN_005958b8(param_1);
                    FUN_005958e3(local_2c,param_1);
                  }
                }
                else {
                  local_28 = param_1;
                }
              }
              if (local_28 == (byte *)0x0) {
                if (param_2 == 0) {
                  param_2 = 1;
                }
                param_2 = param_2 + 0xf & 0xfffffff0;
                local_28 = HeapAlloc(DAT_013b8604,0,param_2);
                if (local_28 != (byte *)0x0) {
                  local_24 = *(int *)(param_1 + -4) - 1;
                  uVar2 = local_24;
                  if (param_2 <= local_24) {
                    uVar2 = param_2;
                  }
                  FUN_0058f220(local_28,param_1,uVar2);
                  FUN_005958e3(local_2c,param_1);
                }
              }
            }
            local_8 = 0xffffffff;
            FUN_005992fe();
            if (local_2c == 0) {
              if (param_2 == 0) {
                param_2 = 1;
              }
              param_2 = param_2 + 0xf & 0xfffffff0;
              local_28 = HeapReAlloc(DAT_013b8604,0,param_1,param_2);
            }
          }
          if (local_28 != (byte *)0x0) {
            ExceptionList = local_14;
            return local_28;
          }
          if (DAT_013b804c == (byte *)0x0) {
            ExceptionList = local_14;
            return (byte *)0x0;
          }
          iVar1 = FUN_00595683(param_2);
        } while (iVar1 != 0);
      }
      else {
        ExceptionList = &local_14;
        if (DAT_013b8608 == 2) {
          ExceptionList = &local_14;
          if (param_2 < 0xffffffe1) {
            if (param_2 == 0) {
              param_2 = 0x10;
              ExceptionList = &local_14;
            }
            else {
              param_2 = param_2 + 0xf & 0xfffffff0;
              ExceptionList = &local_14;
            }
          }
          do {
            local_28 = pbVar3;
            if (param_2 < 0xffffffe1) {
              FUN_00596aad(9);
              local_8 = 1;
              pbVar3 = (byte *)FUN_00596613(param_1,&local_3c,&local_30);
              local_34 = pbVar3;
              if (pbVar3 == (byte *)0x0) {
                local_28 = HeapReAlloc(DAT_013b8604,0,param_1,param_2);
              }
              else {
                if (param_2 < DAT_005cbf5c) {
                  iVar1 = FUN_005969db(local_3c,local_30,pbVar3,param_2 >> 4);
                  if (iVar1 == 0) {
                    local_28 = (byte *)FUN_005966af(param_2 >> 4);
                    if (local_28 != (byte *)0x0) {
                      local_38 = (uint)*pbVar3 << 4;
                      uVar2 = local_38;
                      if (param_2 <= local_38) {
                        uVar2 = param_2;
                      }
                      FUN_0058f220(local_28,param_1,uVar2);
                      FUN_0059666a(local_3c,local_30,pbVar3);
                    }
                  }
                  else {
                    local_28 = param_1;
                  }
                }
                if ((local_28 == (byte *)0x0) &&
                   (local_28 = HeapAlloc(DAT_013b8604,0,param_2), local_28 != (byte *)0x0)) {
                  local_38 = (uint)*pbVar3 << 4;
                  uVar2 = local_38;
                  if (param_2 <= local_38) {
                    uVar2 = param_2;
                  }
                  FUN_0058f220(local_28,param_1,uVar2);
                  FUN_0059666a(local_3c,local_30,pbVar3);
                }
              }
              local_8 = 0xffffffff;
              FUN_0059944c();
            }
            if (local_28 != pbVar3) {
              ExceptionList = local_14;
              return local_28;
            }
            if (DAT_013b804c == pbVar3) {
              ExceptionList = local_14;
              return local_28;
            }
            iVar1 = FUN_00595683(param_2);
          } while (iVar1 != 0);
        }
        else {
          do {
            pbVar3 = (byte *)0x0;
            if (param_2 < 0xffffffe1) {
              if (param_2 == 0) {
                param_2 = 1;
              }
              param_2 = param_2 + 0xf & 0xfffffff0;
              pbVar3 = HeapReAlloc(DAT_013b8604,0,param_1,param_2);
            }
            if (pbVar3 != (byte *)0x0) {
              ExceptionList = local_14;
              return pbVar3;
            }
            if (DAT_013b804c == (byte *)0x0) {
              ExceptionList = local_14;
              return (byte *)0x0;
            }
            iVar1 = FUN_00595683(param_2);
          } while (iVar1 != 0);
        }
      }
    }
    pbVar3 = (byte *)0x0;
  }
  ExceptionList = local_14;
  return pbVar3;
}


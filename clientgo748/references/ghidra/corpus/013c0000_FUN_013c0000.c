// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 013c0000 | Name: FUN_013c0000


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __thiscall FUN_013c0000(int param_1,ushort *param_2)

{
  if ((((0xeb < *param_2) && (*(int *)(param_2 + 0x74) == 0x32435358)) && (DAT_0067cf38 != 0)) &&
     (param_1 == *(int *)(DAT_0067cf38 + 0x4c))) {
    DAT_013c1f8c = 1;
    DAT_013c1f00 = *(undefined4 *)(param_2 + 0x2e);
    DAT_013c1f04 = *(undefined4 *)(param_2 + 0x30);
    DAT_013c1f08 = *(undefined4 *)(param_2 + 0x32);
    DAT_013c1f0c = *(undefined4 *)(param_2 + 0x34);
    DAT_013c1f10 = *(undefined4 *)(param_2 + 0x36);
    DAT_013c1f14 = *(undefined4 *)(param_2 + 0x38);
    DAT_013c1f18 = *(undefined4 *)(param_2 + 0x3a);
    DAT_013c1f1c = *(undefined4 *)(param_2 + 0x3c);
    DAT_013c1f20 = *(undefined4 *)(param_2 + 0x3e);
    DAT_013c1f24 = *(undefined4 *)(param_2 + 0x40);
    DAT_013c1f28 = *(undefined4 *)(param_2 + 0x42);
    _DAT_013c1f2c = *(undefined4 *)(param_2 + 0x44);
    _DAT_013c1f30 = *(undefined4 *)(param_2 + 0x46);
    DAT_013c1f34 = *(undefined4 *)(param_2 + 0x48);
    DAT_013c1f38 = *(undefined4 *)(param_2 + 0x4a);
    DAT_013c1f3c = *(undefined4 *)(param_2 + 0x4c);
    _DAT_013c1f40 = *(undefined4 *)(param_2 + 0x4e);
    _DAT_013c1f44 = *(undefined4 *)(param_2 + 0x50);
    _DAT_013c1f48 = *(undefined4 *)(param_2 + 0x52);
    _DAT_013c1f4c = *(undefined4 *)(param_2 + 0x54);
    _DAT_013c1f50 = *(undefined4 *)(param_2 + 0x56);
    _DAT_013c1f54 = *(undefined4 *)(param_2 + 0x58);
    _DAT_013c1f58 = *(undefined4 *)(param_2 + 0x5a);
    _DAT_013c1f5c = *(undefined4 *)(param_2 + 0x5c);
    _DAT_013c1f60 = *(undefined4 *)(param_2 + 0x5e);
    _DAT_013c1f64 = *(undefined4 *)(param_2 + 0x60);
    _DAT_013c1f68 = *(undefined4 *)(param_2 + 0x62);
    _DAT_013c1f6c = *(undefined4 *)(param_2 + 100);
    _DAT_013c1f70 = *(undefined4 *)(param_2 + 0x66);
    _DAT_013c1f74 = *(undefined4 *)(param_2 + 0x68);
    _DAT_013c1f78 = *(undefined4 *)(param_2 + 0x6a);
    _DAT_013c1f7c = *(undefined4 *)(param_2 + 0x6c);
    _DAT_013c1f80 = *(undefined4 *)(param_2 + 0x6e);
    _DAT_013c1f84 = *(undefined4 *)(param_2 + 0x70);
    _DAT_013c1f88 = *(undefined4 *)(param_2 + 0x72);
  }
  FUN_0052b97d(param_2);
  return 1;
}


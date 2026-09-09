// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0055a815 | Name: FUN_0055a815


undefined4 FUN_0055a815(undefined1 *param_1,byte *param_2)

{
  char cVar1;
  byte bVar2;
  byte bVar3;
  byte bVar4;
  undefined4 local_14;
  
  local_14 = 3;
  cVar1 = FUN_0055a7b4(*param_1);
  bVar2 = FUN_0055a7b4(param_1[1]);
  bVar3 = FUN_0055a7b4(param_1[2]);
  bVar4 = FUN_0055a7b4(param_1[3]);
  if ((((cVar1 == -1) || (bVar2 == 0xff)) || (bVar3 == 0xff)) || (bVar4 == 0xff)) {
    local_14 = 0xffffffff;
  }
  else if ((param_1[3] == '=') || (param_1[2] != '=')) {
    if ((param_1[3] == '=') && (param_1[2] != '=')) {
      local_14 = 2;
    }
    if ((param_1[3] == '=') && (param_1[2] == '=')) {
      local_14 = 1;
    }
    *param_2 = cVar1 << 2 | (byte)((int)(uint)bVar2 >> 4);
    param_2[1] = bVar2 << 4 | (byte)((int)(uint)bVar3 >> 2);
    param_2[2] = bVar3 << 6 | bVar4;
  }
  else {
    local_14 = 0xffffffff;
  }
  return local_14;
}


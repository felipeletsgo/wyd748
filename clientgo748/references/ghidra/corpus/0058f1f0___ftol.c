// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0058f1f0 | Name: __ftol


/* Library Function - Single Match
    __ftol
   
   Library: Visual Studio */

longlong __ftol(void)

{
  float10 in_ST0;
  
  return (longlong)ROUND(in_ST0);
}


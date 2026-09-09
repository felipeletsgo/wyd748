// Ghidra pseudocode; binary: WYD.exe nativo 7.48
// Entry: 0059a366 | Name: __frnd


/* Library Function - Single Match
    __frnd
   
   Libraries: Visual Studio 2008 Release, Visual Studio 2010 Release, Visual Studio 2012 Release,
   Visual Studio 2019 Release */

float10 __frnd(double param_1)

{
  return (float10)ROUND(param_1);
}


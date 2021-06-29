@echo off

set CommonCompilerFlags=-MTd -nologo -fp:fast -Gm- -GR- -EHa- -Od -Oi -WX -W4 -FC -Z7 -DUNICODE
set CommonLinkerFlags= -incremental:no -opt:ref

IF NOT EXIST build mkdir build
pushd build


del *.pdb > NUL 2> NUL
echo WAITING FOR PDB > lock.tmp
:: cl %CommonCompilerFlags% ..\code\tetris.cpp /LD /link -incremental:no -opt:ref -PDB:tetris_%random%.pdb -EXPORT:GameUpdateAndRender
del lock.tmp
cl %CommonCompilerFlags% /EHsc ..\code\tetris_console.cpp /link %CommonLinkerFlags% User32.lib
popd
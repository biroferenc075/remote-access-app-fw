copy %cd%\setup\testimages %cd%\x64\Release
copy %cd%\setup %cd%\x64\Release

copy %cd%\setup\testimages %cd%\x64\Debug
copy %cd%\setup %cd%\x64\Debug

mkdir %cd%\x64\Debug\assets\models
mkdir  %cd%\x64\Debug\assets\shaders 
mkdir  %cd%\x64\Debug\assets\textures
copy %cd%\assets\models %cd%\x64\Debug\assets\models
copy %cd%\assets\shaders %cd%\x64\Debug\assets\shaders 
copy %cd%\assets\textures %cd%\x64\Debug\assets\textures

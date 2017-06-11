for %%i in (1, 1, 21) do (
cd %%i
set 3dsPath=*.3ds
echo "loadSence("../TexturedScene/%%i/%3dsPath%", "../TexturedScene/%%i/", shapeIndexCount, vec3(0, 0, 0), 1);">> ans.txt
cd ..
)
pause
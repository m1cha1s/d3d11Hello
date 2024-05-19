set "src=main.c"
set "libs=user32.lib d3d11.lib dxgi.lib d3dcompiler.lib dxguid.lib"

if %1==DEBUG set flags=/Zi
if %1==RELEASE set flags=/D"NDEBUG" /O2

cl %src% %flags% %libs%
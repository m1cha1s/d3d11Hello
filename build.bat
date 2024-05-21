set "src=main.cpp"
set "libs=user32.lib d3d11.lib dxgi.lib d3dcompiler.lib dxguid.lib"

if %1==DEBUG set flags=/Zi /diagnostics:column
if %1==RELEASE set flags=/D"NDEBUG" /O2 /diagnostics:column

cl %src% %flags% %libs%
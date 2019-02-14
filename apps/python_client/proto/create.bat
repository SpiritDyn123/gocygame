@echo off
set DEST_PATH=%cd%

echo ===current dir:
echo %DEST_PATH%

cd ../../common/proto/proto-file/
echo %cd%
echo ===current dir:
for %%f in (%cd%\*.proto) do (
   protoc  --python_out=%DEST_PATH% -I=%cd% %%f
)
cd %DEST_PATH%
echo ===current dir:
echo %cd%
echo success

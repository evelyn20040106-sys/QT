$env:Path = "E:\Qt\Tools\CMake_64\bin;$env:Path"
$env:Path = "E:\Qt\Tools\mingw1310_64\bin;$env:Path"
$env:Path = "E:\Qt\6.11.1\mingw_64\bin;$env:Path"

$buildDir = "E:\qt_project\FaceRecognition\build"
if (!(Test-Path $buildDir)) {
    New-Item -ItemType Directory -Path $buildDir | Out-Null
}
Set-Location $buildDir

$cmakeArgs = @(
    "-G", "MinGW Makefiles"
    "-DCMAKE_CXX_COMPILER=g++"
    "-DCMAKE_C_COMPILER=gcc"
    "-DCMAKE_BUILD_TYPE=Release"
    "-DCMAKE_PREFIX_PATH=E:/Qt/6.11.1/mingw_64"
    "-DOpenCV_DIR=E:/opencv"
    ".."
)

Write-Output "Running cmake ..."
& "cmake" $cmakeArgs 2>&1

if ($LASTEXITCODE -eq 0) {
    Write-Output "`nCMake succeeded. Running mingw32-make ..."
    mingw32-make -j8 2>&1
} else {
    Write-Output "`nCMake failed with exit code: $LASTEXITCODE"
}

[CmdletBinding()]
param(
    [ValidateSet('Release')]
    [string]$Configuration = 'Release',

    [ValidateSet('Win32')]
    [string]$Platform = 'Win32',

    [string]$BuildRoot
)

$ErrorActionPreference = 'Stop'
$ProgressPreference = 'SilentlyContinue'

$repositoryRoot = Split-Path -Parent $PSScriptRoot
if ([string]::IsNullOrWhiteSpace($BuildRoot)) {
    $BuildRoot = Join-Path $repositoryRoot '.build'
}
$BuildRoot = [System.IO.Path]::GetFullPath($BuildRoot)

$projectPath = Join-Path $repositoryRoot 'SkinEditor_DX9\SkinEditor_DX9.vcxproj'
$variant = "$Configuration-$Platform"
$outputDirectory = Join-Path $repositoryRoot 'SkinEditor_DX9\Release'
$intermediateDirectory = Join-Path $BuildRoot "obj\$variant"
$logDirectory = Join-Path $BuildRoot 'logs'
$packageRoot = Join-Path $BuildRoot `
    'packages\microsoft.dxsdk.d3dx\9.29.952.8'

New-Item -ItemType Directory -Force -Path $outputDirectory,
    $intermediateDirectory, $logDirectory | Out-Null

$d3dxTargets = Join-Path $packageRoot `
    'build\native\Microsoft.DXSDK.D3DX.targets'
if (-not (Test-Path -LiteralPath $d3dxTargets)) {
    $downloadDirectory = Join-Path $BuildRoot 'downloads'
    New-Item -ItemType Directory -Force -Path $downloadDirectory | Out-Null
    $packageArchive = Join-Path $downloadDirectory `
        'microsoft.dxsdk.d3dx.9.29.952.8.nupkg'
    $packageUri = 'https://api.nuget.org/v3-flatcontainer/' +
        'microsoft.dxsdk.d3dx/9.29.952.8/' +
        'microsoft.dxsdk.d3dx.9.29.952.8.nupkg'
    $expectedSha512 = 'n2qV7YWFVcHEOKhSGe3jLIJykGiyHdfs8R3gHPPN1SWy8EpYZDv8wUxIopQD3ByAJG8KEqHvQ3e5G4VfbW15hg=='

    Write-Host 'Downloading Microsoft.DXSDK.D3DX 9.29.952.8'
    Invoke-WebRequest -UseBasicParsing -Uri $packageUri -OutFile $packageArchive
    $packageBytes = [System.IO.File]::ReadAllBytes($packageArchive)
    $sha512 = [System.Security.Cryptography.SHA512]::Create()
    try {
        $actualSha512 = [Convert]::ToBase64String(
            $sha512.ComputeHash($packageBytes))
    }
    finally {
        $sha512.Dispose()
    }
    if ($actualSha512 -ne $expectedSha512) {
        throw 'Microsoft.DXSDK.D3DX package SHA-512 verification failed.'
    }

    $extractDirectory = "$packageRoot.extracting"
    if (Test-Path -LiteralPath $extractDirectory) {
        Remove-Item -LiteralPath $extractDirectory -Recurse -Force
    }
    New-Item -ItemType Directory -Force -Path $extractDirectory | Out-Null
    Add-Type -AssemblyName System.IO.Compression.FileSystem
    [System.IO.Compression.ZipFile]::ExtractToDirectory(
        $packageArchive, $extractDirectory)
    if (-not (Test-Path -LiteralPath (Join-Path $extractDirectory `
        'build\native\Microsoft.DXSDK.D3DX.targets'))) {
        throw 'The downloaded D3DX package is missing its native build assets.'
    }
    if (Test-Path -LiteralPath $packageRoot) {
        Remove-Item -LiteralPath $packageRoot -Recurse -Force
    }
    Move-Item -LiteralPath $extractDirectory -Destination $packageRoot
}

$vswherePath = Join-Path ${env:ProgramFiles(x86)} `
    'Microsoft Visual Studio\Installer\vswhere.exe'
if (-not (Test-Path -LiteralPath $vswherePath)) {
    throw 'vswhere.exe was not found. Install Visual Studio 2022 Build Tools.'
}

$visualStudioPath = & $vswherePath -latest -products * `
    -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 `
    -property installationPath
if ([string]::IsNullOrWhiteSpace($visualStudioPath)) {
    throw 'Visual Studio 2022 C++ build tools were not found.'
}

$msbuildPath = Join-Path $visualStudioPath 'MSBuild\Current\Bin\MSBuild.exe'
if (-not (Test-Path -LiteralPath $msbuildPath)) {
    throw "MSBuild was not found at $msbuildPath"
}

$outputWithSeparator = $outputDirectory.Replace('\', '/').TrimEnd('/') + '/'
$intermediateWithSeparator = $intermediateDirectory.Replace('\', '/').TrimEnd('/') + '/'
$textLog = Join-Path $logDirectory "build-$variant.log"
$binaryLog = Join-Path $logDirectory "build-$variant.binlog"

$msbuildArguments = @(
    $projectPath,
    '/t:Build',
    '/m',
    '/nologo',
    "/p:Configuration=$Configuration",
    "/p:Platform=$Platform",
    "/p:OutDir=$outputWithSeparator",
    "/p:IntDir=$intermediateWithSeparator",
    "/p:BaseIntermediateOutputPath=$intermediateWithSeparator",
    "/p:D3DXPackageRoot=$($packageRoot.Replace('\\', '/'))",
    "/bl:$binaryLog",
    '/fl',
    "/flp:logfile=$textLog;verbosity=normal"
)

Write-Host "Building $Configuration | $Platform"
Write-Host "MSBuild: $msbuildPath"
Write-Host "Output:  $outputDirectory"

& $msbuildPath @msbuildArguments
if ($LASTEXITCODE -ne 0) {
    throw "MSBuild failed with exit code $LASTEXITCODE. See $textLog"
}

$executablePath = Join-Path $outputDirectory 'SkinEditor_DX9.exe'
if (-not (Test-Path -LiteralPath $executablePath)) {
    throw "Build completed without the expected executable: $executablePath"
}

$d3dxRuntime = Join-Path $packageRoot `
    'build\native\release\bin\x86\D3DX9_43.dll'
if (-not (Test-Path -LiteralPath $d3dxRuntime)) {
    throw "D3DX runtime was not found: $d3dxRuntime"
}
Copy-Item -LiteralPath $d3dxRuntime -Destination $outputDirectory -Force

Write-Host "Build succeeded: $executablePath"
Write-Output $executablePath

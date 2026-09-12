[CmdletBinding()]
param([string]$ExecutablePath)
$ErrorActionPreference = 'Stop'
$repositoryRoot = Split-Path -Parent $PSScriptRoot
if ([string]::IsNullOrWhiteSpace($ExecutablePath)) {
    $ExecutablePath = Join-Path $repositoryRoot 'SkinEditor_DX9\Release\SkinEditor_DX9.exe'
}
$ExecutablePath = [System.IO.Path]::GetFullPath($ExecutablePath)
if (-not (Test-Path -LiteralPath $ExecutablePath)) { throw 'Build the experimental editor first.' }
$previousEditor = $env:LR2_AGENT_EDITOR
Push-Location $repositoryRoot
try {
    $env:LR2_AGENT_EDITOR = $ExecutablePath
    & python -B -m unittest discover -s tests -p 'test_skin_agent*.py' -v
    if ($LASTEXITCODE -ne 0) { throw 'Agent utility unit/integration tests failed.' }
}
finally {
    $env:LR2_AGENT_EDITOR = $previousEditor
    Pop-Location
}

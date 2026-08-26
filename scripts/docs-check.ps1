[CmdletBinding()]
param(
    [string]$OutputDirectory
)

$ErrorActionPreference = 'Stop'

$repositoryRoot = Split-Path -Parent $PSScriptRoot
if ([string]::IsNullOrWhiteSpace($OutputDirectory)) {
    $OutputDirectory = Join-Path $repositoryRoot '.build\docs-check'
}
$OutputDirectory = [System.IO.Path]::GetFullPath($OutputDirectory)

function Relative-Path([string]$Path) {
    $fullPath = [System.IO.Path]::GetFullPath($Path)
    return $fullPath.Substring($repositoryRoot.Length).TrimStart('\', '/').Replace('\', '/')
}

function Write-Utf8NoBom([string]$Path, [string]$Content) {
    $encoding = New-Object System.Text.UTF8Encoding($false)
    [System.IO.File]::WriteAllText($Path, $Content, $encoding)
}

$markdownFiles = @(
    Get-ChildItem -LiteralPath $repositoryRoot -File -Filter '*.md'
    Get-ChildItem -LiteralPath (Join-Path $repositoryRoot 'docs') -File -Recurse -Filter '*.md'
    Get-ChildItem -LiteralPath (Join-Path $repositoryRoot '.github') -File -Recurse -Filter '*.md'
) | Sort-Object FullName -Unique

$errors = [System.Collections.Generic.List[object]]::new()
$linkCount = 0
$linkPattern = [regex]::new('\[[^\]]+\]\((?<target><[^>]+>|[^\s\)]+)')

foreach ($file in $markdownFiles) {
    $text = Get-Content -LiteralPath $file.FullName -Raw
    $fenceCount = @([regex]::Matches($text, '(?m)^\s*```')).Count
    if (($fenceCount % 2) -ne 0) {
        $errors.Add([pscustomobject][ordered]@{
            file = Relative-Path $file.FullName
            code = 'unbalanced-code-fence'
            message = "Unbalanced fenced code block count: $fenceCount"
        })
    }

    foreach ($match in $linkPattern.Matches($text)) {
        $target = $match.Groups['target'].Value.Trim('<', '>')
        if ($target -match '^(?:https?|mailto):' -or $target.StartsWith('#')) {
            continue
        }
        $linkCount++
        $pathOnly = ($target -split '#', 2)[0]
        if ([string]::IsNullOrWhiteSpace($pathOnly)) { continue }
        $decoded = [System.Uri]::UnescapeDataString($pathOnly).Replace('/', '\')
        $resolved = [System.IO.Path]::GetFullPath((Join-Path $file.DirectoryName $decoded))
        if (-not (Test-Path -LiteralPath $resolved)) {
            $errors.Add([pscustomobject][ordered]@{
                file = Relative-Path $file.FullName
                code = 'missing-link-target'
                message = "Missing link target: $target"
            })
        }
    }
}

$result = [pscustomobject][ordered]@{
    schemaVersion = 1
    checkedFiles = $markdownFiles.Count
    checkedRelativeLinks = $linkCount
    errors = @($errors)
}

New-Item -ItemType Directory -Force -Path $OutputDirectory | Out-Null
$resultPath = Join-Path $OutputDirectory 'docs-check.json'
Write-Utf8NoBom $resultPath (($result | ConvertTo-Json -Depth 6) + "`n")

Write-Host "Documentation: $($markdownFiles.Count) files, $linkCount relative links, $($errors.Count) errors"
Write-Host "Result: $resultPath"
if ($errors.Count -gt 0) {
    foreach ($error in $errors) {
        Write-Error "[$($error.code)] $($error.file): $($error.message)" -ErrorAction Continue
    }
    exit 1
}

Write-Output $resultPath

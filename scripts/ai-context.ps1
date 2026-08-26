[CmdletBinding()]
param(
    [string]$OutputDirectory,
    [string]$UiKey,
    [switch]$Check
)

$ErrorActionPreference = 'Stop'

$repositoryRoot = Split-Path -Parent $PSScriptRoot
$uiMapDirectory = Join-Path $repositoryRoot '.build\ui-map'
if ([string]::IsNullOrWhiteSpace($OutputDirectory)) {
    $OutputDirectory = Join-Path $repositoryRoot '.build\ai-context'
    if (-not [string]::IsNullOrWhiteSpace($UiKey)) {
        $OutputDirectory = Join-Path $OutputDirectory $UiKey
    }
}
$OutputDirectory = [System.IO.Path]::GetFullPath($OutputDirectory)

function Write-Utf8NoBom([string]$Path, [string]$Content) {
    $encoding = New-Object System.Text.UTF8Encoding($false)
    [System.IO.File]::WriteAllText($Path, $Content, $encoding)
}

function Relative-Path([string]$Path) {
    $fullPath = [System.IO.Path]::GetFullPath($Path)
    return $fullPath.Substring($repositoryRoot.Length).TrimStart('\', '/').Replace('\', '/')
}

function Escape-MarkdownCell([object]$Value) {
    if ($null -eq $Value) { return '' }
    return ([string]$Value).Replace('|', '\|').Replace("`r", ' ').Replace("`n", ' ')
}

$requiredFiles = @(
    'AGENTS.md',
    'README.md',
    'docs\AI_COLLABORATION.md',
    'docs\PROJECT_STATE.md',
    'docs\BUILD_AND_TEST.md',
    'docs\UI_ARCHITECTURE.md',
    'docs\UI_MAP.md',
    'docs\HANDOFF_TEMPLATE.md',
    'docs\decisions\0001-ai-readable-ui-and-handoff-contract.md',
    'SkinEditor_DX9\uiCatalog.h',
    'skinHelper.txt',
    'skinObjGroup.txt',
    'scripts\ui-map.ps1',
    'scripts\build.ps1',
    'scripts\test.ps1',
    'scripts\docs-check.ps1',
    'scripts\ai-context.ps1',
    '.github\workflows\ci.yml'
)

$missingFiles = @($requiredFiles | Where-Object {
    -not (Test-Path -LiteralPath (Join-Path $repositoryRoot $_))
})
if ($missingFiles.Count -gt 0) {
    throw "Required collaboration files are missing: $($missingFiles -join ', ')"
}

$uiMapScript = Join-Path $PSScriptRoot 'ui-map.ps1'
& $uiMapScript -OutputDirectory $uiMapDirectory -Check:$Check | Out-Null

$uiMapPath = Join-Path $uiMapDirectory 'ui-map.json'
$uiMap = Get-Content -LiteralPath $uiMapPath -Raw | ConvertFrom-Json
if ($uiMap.summary.errorCount -ne 0) {
    throw "UI map has $($uiMap.summary.errorCount) contract errors."
}

$allEntries = @($uiMap.windows | ForEach-Object {
    $_ | Add-Member -NotePropertyName entryType -NotePropertyValue 'window' -PassThru -Force
}) + @($uiMap.surfaces | ForEach-Object {
    $_ | Add-Member -NotePropertyName entryType -NotePropertyValue 'surface' -PassThru -Force
})

$focusEntry = $null
$focusControls = @()
if (-not [string]::IsNullOrWhiteSpace($UiKey)) {
    $focusEntry = @($allEntries | Where-Object key -eq $UiKey)
    if ($focusEntry.Count -ne 1) {
        $availableKeys = @($allEntries | ForEach-Object key) -join ', '
        throw "Unknown UI key '$UiKey'. Available keys: $availableKeys"
    }
    $focusEntry = $focusEntry[0]
    $focusControls = @($uiMap.controls | Where-Object {
        $_.windowKeys -contains $UiKey -or $_.surfaceKeys -contains $UiKey
    })
}

$contextFiles = foreach ($relativePath in $requiredFiles) {
    $path = Join-Path $repositoryRoot $relativePath
    $item = Get-Item -LiteralPath $path
    [pscustomobject][ordered]@{
        path = $relativePath.Replace('\', '/')
        sha256 = (Get-FileHash -LiteralPath $path -Algorithm SHA256).Hash.ToLowerInvariant()
        bytes = $item.Length
    }
}

$git = [pscustomobject][ordered]@{
    available = $false
    branch = $null
    head = $null
    dirty = $null
    remote = 'https://github.com/GOMazk/SkinEditor'
    targetBranch = 'AI'
}
if (Test-Path -LiteralPath (Join-Path $repositoryRoot '.git')) {
    $branch = (& git -C $repositoryRoot branch --show-current 2>$null)
    $head = (& git -C $repositoryRoot rev-parse HEAD 2>$null)
    $status = @(& git -C $repositoryRoot status --short 2>$null)
    if ($LASTEXITCODE -eq 0) {
        $git.available = $true
        $git.branch = [string]$branch
        $git.head = [string]$head
        $git.dirty = $status.Count -gt 0
    }
}

$testResult = $null
$junitPath = Join-Path $repositoryRoot '.build\test-results\skineditor-self-tests.xml'
if (Test-Path -LiteralPath $junitPath) {
    [xml]$junit = Get-Content -LiteralPath $junitPath -Raw
    $testResult = [pscustomobject][ordered]@{
        path = Relative-Path $junitPath
        tests = [int]$junit.testsuite.tests
        failures = [int]$junit.testsuite.failures
        seconds = [double]::Parse([string]$junit.testsuite.time,
            [System.Globalization.CultureInfo]::InvariantCulture)
    }
}

$docsResult = $null
$docsCheckPath = Join-Path $repositoryRoot '.build\docs-check\docs-check.json'
if (Test-Path -LiteralPath $docsCheckPath) {
    $docsCheck = Get-Content -LiteralPath $docsCheckPath -Raw | ConvertFrom-Json
    $docsResult = [pscustomobject][ordered]@{
        path = Relative-Path $docsCheckPath
        checkedFiles = [int]$docsCheck.checkedFiles
        checkedRelativeLinks = [int]$docsCheck.checkedRelativeLinks
        errors = @($docsCheck.errors).Count
    }
}

$focus = $null
if ($null -ne $focusEntry) {
    $ownerPeers = @($allEntries | Where-Object {
        $_.ownerFunction -eq $focusEntry.ownerFunction -and
            $_.key -ne $focusEntry.key
    } | ForEach-Object key)
    $focus = [pscustomobject][ordered]@{
        entry = $focusEntry
        ownerPeers = $ownerPeers
        controls = $focusControls
    }
}

$manifest = [pscustomobject][ordered]@{
    schemaVersion = 1
    project = 'GOMazk/SkinEditor'
    baseline = [pscustomobject][ordered]@{
        branch = 'AI'
        configuration = 'Release'
        platform = 'Win32'
        toolset = 'Visual Studio 2022 v143'
    }
    git = $git
    uiMap = [pscustomobject][ordered]@{
        path = Relative-Path $uiMapPath
        summary = $uiMap.summary
    }
    lastSelfTest = $testResult
    lastDocsCheck = $docsResult
    focus = $focus
    validationCommands = @(
        'powershell -ExecutionPolicy Bypass -File .\scripts\ui-map.ps1 -Check',
        'powershell -ExecutionPolicy Bypass -File .\scripts\docs-check.ps1',
        'powershell -ExecutionPolicy Bypass -File .\scripts\build.ps1',
        'powershell -ExecutionPolicy Bypass -File .\scripts\test.ps1',
        'powershell -ExecutionPolicy Bypass -File .\scripts\ai-context.ps1 -Check'
    )
    contextFiles = @($contextFiles)
}

New-Item -ItemType Directory -Force -Path $OutputDirectory | Out-Null
$manifestPath = Join-Path $OutputDirectory 'context-manifest.json'
$manifestJson = $manifest | ConvertTo-Json -Depth 12
Write-Utf8NoBom $manifestPath ($manifestJson + "`n")

$markdown = New-Object System.Text.StringBuilder
$null = $markdown.AppendLine('# SkinEditor AI context pack')
$null = $markdown.AppendLine()
$null = $markdown.AppendLine('Generated artifact. Do not edit it by hand; update the source documents or catalog.')
$null = $markdown.AppendLine()
$null = $markdown.AppendLine('## Baseline')
$null = $markdown.AppendLine()
$null = $markdown.AppendLine('- Repository: `GOMazk/SkinEditor`')
$null = $markdown.AppendLine('- Target branch: `AI`')
$null = $markdown.AppendLine('- Build: `Release | Win32(x86)` with Visual Studio 2022 `v143`')
$gitText = if ($git.available) {
    "branch ``$($git.branch)`` at ``$($git.head)``; dirty=$($git.dirty)"
} else {
    'no `.git` directory; treat this folder as an export, not a publishing checkout'
}
$null = $markdown.AppendLine("- Current Git snapshot: $gitText")
$null = $markdown.AppendLine()
$null = $markdown.AppendLine('## Read order')
$null = $markdown.AppendLine()
$null = $markdown.AppendLine('1. `AGENTS.md`')
$null = $markdown.AppendLine('2. `docs/AI_COLLABORATION.md`')
$null = $markdown.AppendLine('3. The relevant section of `docs/PROJECT_STATE.md`')
$null = $markdown.AppendLine('4. `docs/UI_ARCHITECTURE.md` for UI/state changes')
$null = $markdown.AppendLine('5. `docs/BUILD_AND_TEST.md` before claiming completion')
$null = $markdown.AppendLine()
$null = $markdown.AppendLine('## Machine summary')
$null = $markdown.AppendLine()
$null = $markdown.AppendLine("- UI windows: $($uiMap.summary.windowCount)")
$null = $markdown.AppendLine("- Flows and shells: $($uiMap.summary.surfaceCount)")
$null = $markdown.AppendLine("- Source controls: $($uiMap.summary.controlCount)")
$null = $markdown.AppendLine("- Unassigned controls: $($uiMap.summary.unmappedControlCount)")
$null = $markdown.AppendLine("- UI contract errors: $($uiMap.summary.errorCount)")
if ($null -eq $testResult) {
    $null = $markdown.AppendLine('- Last self-test: no JUnit result in this workspace')
} else {
    $null = $markdown.AppendLine("- Last self-test: $($testResult.tests) tests, $($testResult.failures) failures")
}
if ($null -eq $docsResult) {
    $null = $markdown.AppendLine('- Last docs check: no result in this workspace')
} else {
    $null = $markdown.AppendLine("- Last docs check: $($docsResult.checkedFiles) files, $($docsResult.checkedRelativeLinks) links, $($docsResult.errors) errors")
}

if ($null -ne $focusEntry) {
    $null = $markdown.AppendLine()
    $null = $markdown.AppendLine("## Focus: $($focusEntry.title)")
    $null = $markdown.AppendLine()
    $null = $markdown.AppendLine("- Key: ``$($focusEntry.key)``")
    $null = $markdown.AppendLine("- Type: ``$($focusEntry.entryType)``")
    $null = $markdown.AppendLine("- Owner: ``$($focusEntry.ownerFunction)``")
    $null = $markdown.AppendLine("- Purpose: $($focusEntry.purpose)")
    if ($focus.ownerPeers.Count -gt 0) {
        $peers = @($focus.ownerPeers | ForEach-Object { "``$_``" }) -join ', '
        $null = $markdown.AppendLine("- Shared owner warning: controls may also belong to $peers")
    }
    $null = $markdown.AppendLine()
    $null = $markdown.AppendLine('| Kind | Label | Function | Source |')
    $null = $markdown.AppendLine('|---|---|---|---|')
    foreach ($control in $focusControls) {
        $source = "$($control.file):$($control.line)"
        $null = $markdown.AppendLine("| $(Escape-MarkdownCell $control.kind) | $(Escape-MarkdownCell $control.visibleLabel) | ``$(Escape-MarkdownCell $control.function)`` | ``$(Escape-MarkdownCell $source)`` |")
    }
} else {
    $null = $markdown.AppendLine()
    $null = $markdown.AppendLine('## UI ownership')
    $null = $markdown.AppendLine()
    $null = $markdown.AppendLine('| Key | Title | Owner | Placement/type | Controls |')
    $null = $markdown.AppendLine('|---|---|---|---|---:|')
    foreach ($entry in $allEntries) {
        $placement = if ($entry.entryType -eq 'window') { $entry.defaultDock } else { $entry.kind }
        $null = $markdown.AppendLine("| ``$(Escape-MarkdownCell $entry.key)`` | $(Escape-MarkdownCell $entry.title) | ``$(Escape-MarkdownCell $entry.ownerFunction)`` | $(Escape-MarkdownCell $placement) | $($entry.controlCount) |")
    }
}

$null = $markdown.AppendLine()
$null = $markdown.AppendLine('## Required verification')
$null = $markdown.AppendLine()
$null = $markdown.AppendLine('```powershell')
foreach ($command in $manifest.validationCommands) {
    $null = $markdown.AppendLine($command)
}
$null = $markdown.AppendLine('```')
$null = $markdown.AppendLine()
$null = $markdown.AppendLine('Native GUI and real LR2 skin scenarios remain manual unless explicitly exercised.')
$null = $markdown.AppendLine()
$null = $markdown.AppendLine('## Handoff minimum')
$null = $markdown.AppendLine()
$null = $markdown.AppendLine('Report the user-visible result, changed files/symbols, actual verification, unverified manual scenarios, existing warnings, and Git publication state. Use `docs/HANDOFF_TEMPLATE.md`.')
$null = $markdown.AppendLine()
$null = $markdown.AppendLine('## Context integrity')
$null = $markdown.AppendLine()
$null = $markdown.AppendLine('| File | SHA-256 | Bytes |')
$null = $markdown.AppendLine('|---|---|---:|')
foreach ($file in $contextFiles) {
    $null = $markdown.AppendLine("| ``$($file.path)`` | ``$($file.sha256)`` | $($file.bytes) |")
}

$packPath = Join-Path $OutputDirectory 'context-pack.md'
Write-Utf8NoBom $packPath ($markdown.ToString())

if ($Check) {
    $roundTrip = Get-Content -LiteralPath $manifestPath -Raw | ConvertFrom-Json
    if ($roundTrip.schemaVersion -ne 1 -or $roundTrip.uiMap.summary.errorCount -ne 0) {
        throw 'Generated context manifest failed validation.'
    }
    if ((Get-Item -LiteralPath $packPath).Length -eq 0) {
        throw 'Generated context pack is empty.'
    }
}

Write-Host "AI context: $($uiMap.summary.windowCount) windows, $($uiMap.summary.surfaceCount) flows/shells"
if (-not [string]::IsNullOrWhiteSpace($UiKey)) {
    Write-Host "Focus: $UiKey ($($focusControls.Count) controls)"
}
Write-Host "Manifest: $manifestPath"
Write-Host "Markdown: $packPath"
Write-Output $packPath

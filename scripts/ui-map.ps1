[CmdletBinding()]
param(
    [string]$SourceRoot,
    [string]$OutputDirectory,
    [switch]$Check
)

$ErrorActionPreference = 'Stop'

$repositoryRoot = Split-Path -Parent $PSScriptRoot
if ([string]::IsNullOrWhiteSpace($SourceRoot)) {
    $SourceRoot = Join-Path $repositoryRoot 'SkinEditor_DX9'
}
if ([string]::IsNullOrWhiteSpace($OutputDirectory)) {
    $OutputDirectory = Join-Path $repositoryRoot '.build\ui-map'
}
$SourceRoot = [System.IO.Path]::GetFullPath($SourceRoot)
$OutputDirectory = [System.IO.Path]::GetFullPath($OutputDirectory)
$sourceRootDisplay = $SourceRoot.Substring($repositoryRoot.Length).TrimStart('\', '/')
$sourceRootDisplay = $sourceRootDisplay.Replace('\', '/')

$catalogPath = Join-Path $SourceRoot 'uiCatalog.h'
$templatePath = Join-Path $repositoryRoot 'tools\ui-map\template.html'
if (-not (Test-Path -LiteralPath $catalogPath)) {
    throw "UI catalog was not found: $catalogPath"
}
if (-not (Test-Path -LiteralPath $templatePath)) {
    throw "UI map template was not found: $templatePath"
}

function Write-Utf8NoBom([string]$Path, [string]$Content) {
    $encoding = New-Object System.Text.UTF8Encoding($false)
    [System.IO.File]::WriteAllText($Path, $Content, $encoding)
}

function Escape-MarkdownCell([object]$Value) {
    if ($null -eq $Value) { return '' }
    return ([string]$Value).Replace('|', '\|').Replace("`r", ' ').Replace("`n", ' ')
}

$catalogPattern = [regex]::new(
    '^\s*\{\s*SEUIWindowId::(?<enum>[A-Za-z0-9_]+),\s*"(?<key>[^"]+)",\s*"(?<title>[^"]+)",\s*"(?<purpose>[^"]+)",\s*"(?<owner>[^"]+)",\s*"(?<group>[^"]+)",\s*"(?<dock>[^"]+)",\s*(?<visible>true|false)\s*\},?\s*$')
$windows = [System.Collections.Generic.List[object]]::new()
foreach ($line in Get-Content -LiteralPath $catalogPath) {
    $match = $catalogPattern.Match($line)
    if (-not $match.Success) { continue }
    $windows.Add([pscustomobject][ordered]@{
        enum = $match.Groups['enum'].Value
        key = $match.Groups['key'].Value
        title = $match.Groups['title'].Value
        purpose = $match.Groups['purpose'].Value
        ownerFunction = $match.Groups['owner'].Value
        group = $match.Groups['group'].Value
        defaultDock = $match.Groups['dock'].Value
        defaultVisible = $match.Groups['visible'].Value -eq 'true'
    })
}
if ($windows.Count -eq 0) {
    throw 'No UI window entries were parsed from uiCatalog.h.'
}

$surfacePattern = [regex]::new(
    '^\s*\{\s*SEUISurfaceId::(?<enum>[A-Za-z0-9_]+),\s*"(?<key>[^"]+)",\s*"(?<title>[^"]+)",\s*"(?<purpose>[^"]+)",\s*"(?<owner>[^"]+)",\s*"(?<kind>[^"]+)"\s*\},?\s*$')
$surfaces = [System.Collections.Generic.List[object]]::new()
foreach ($line in Get-Content -LiteralPath $catalogPath) {
    $match = $surfacePattern.Match($line)
    if (-not $match.Success) { continue }
    $surfaces.Add([pscustomobject][ordered]@{
        enum = $match.Groups['enum'].Value
        key = $match.Groups['key'].Value
        title = $match.Groups['title'].Value
        purpose = $match.Groups['purpose'].Value
        ownerFunction = $match.Groups['owner'].Value
        kind = $match.Groups['kind'].Value
    })
}
if ($surfaces.Count -eq 0) {
    throw 'No UI surface entries were parsed from uiCatalog.h.'
}

$sourceFiles = @(Get-ChildItem -LiteralPath $SourceRoot -File |
    Where-Object { $_.Extension -eq '.cpp' -or $_.Extension -eq '.h' } |
    Sort-Object FullName)
$functionPattern = [regex]::new(
    '^\s*(?:(?:static|inline|constexpr|virtual)\s+)*(?:[A-Za-z_][A-Za-z0-9_:<>,~*&]*\s+)+(?<function>(?:[A-Za-z_][A-Za-z0-9_]*::)*[A-Za-z_][A-Za-z0-9_]*)\s*\([^;]*\)\s*(?:const\s*)?(?:\{|$)')
$widgetPattern = [regex]::new(
    '(?<namespace>ImGui|SEUI)::(?<kind>BeginPopupModal|BeginPopup|BeginChild|BeginTabItem|BeginMenu|BeginCombo|Begin|MenuItem|ActionButton|SmallButton|InvisibleButton|Button|Checkbox|RadioButton|Input[A-Za-z0-9_]*|Slider[A-Za-z0-9_]*|Drag[A-Za-z0-9_]*|Combo|Selectable|CollapsingHeader|TreeNode[A-Za-z0-9_]*|SeparatorText|SectionHeader|TextUnformatted|TextDisabled|TextColored|Text)\s*\(\s*(?<argument>"(?:\\.|[^"])*"|[A-Za-z_][A-Za-z0-9_]*)')

$controls = [System.Collections.Generic.List[object]]::new()
$functionsSeen = [System.Collections.Generic.HashSet[string]]::new(
    [System.StringComparer]::Ordinal)
$nonFunctionKeywords = @('if', 'for', 'while', 'switch', 'catch', 'sizeof')
foreach ($file in $sourceFiles) {
    $relativePath = $file.FullName.Substring($repositoryRoot.Length).TrimStart('\', '/')
    $relativePath = $relativePath.Replace('\', '/')
    $currentFunction = '<global>'
    $lines = Get-Content -LiteralPath $file.FullName
    for ($lineIndex = 0; $lineIndex -lt $lines.Count; ++$lineIndex) {
        $line = $lines[$lineIndex]
        $functionMatch = $functionPattern.Match($line)
        if ($functionMatch.Success) {
            $candidateFunction = $functionMatch.Groups['function'].Value
            if ($nonFunctionKeywords -notcontains $candidateFunction) {
                $currentFunction = $candidateFunction
                $null = $functionsSeen.Add($currentFunction)
            }
        }

        $trimmed = $line.TrimStart()
        if ($trimmed.StartsWith('//') -or $trimmed.StartsWith('*')) { continue }
        foreach ($widgetMatch in $widgetPattern.Matches($line)) {
            $argument = $widgetMatch.Groups['argument'].Value
            if ($argument.StartsWith('"')) {
                $label = $argument.Substring(1, $argument.Length - 2)
                $label = $label.Replace('\"', '"').Replace('\\', '\')
            }
            else {
                $label = "<dynamic:$argument>"
            }
            $visibleLabel = ($label -split '##', 2)[0]
            if ([string]::IsNullOrWhiteSpace($visibleLabel)) {
                $visibleLabel = '(hidden id)'
            }
            $windowKeys = @($windows |
                Where-Object { $_.ownerFunction -eq $currentFunction } |
                ForEach-Object { $_.key })
            $surfaceKeys = @($surfaces |
                Where-Object { $_.ownerFunction -eq $currentFunction } |
                ForEach-Object { $_.key })
            $controls.Add([pscustomobject][ordered]@{
                namespace = $widgetMatch.Groups['namespace'].Value
                kind = $widgetMatch.Groups['kind'].Value
                label = $label
                visibleLabel = $visibleLabel
                function = $currentFunction
                windowKeys = $windowKeys
                surfaceKeys = $surfaceKeys
                file = $relativePath
                line = $lineIndex + 1
            })
        }
    }
}

$diagnostics = [System.Collections.Generic.List[object]]::new()
$allCatalogEntries = @($windows) + @($surfaces)
$duplicateKeys = @($allCatalogEntries | Group-Object key | Where-Object Count -gt 1)
$duplicateTitles = @($allCatalogEntries | Group-Object title | Where-Object Count -gt 1)
foreach ($duplicate in $duplicateKeys) {
    $diagnostics.Add([pscustomobject]@{
        level = 'error'; code = 'duplicate-key'
        message = "Duplicate window key: $($duplicate.Name)"
    })
}
foreach ($duplicate in $duplicateTitles) {
    $diagnostics.Add([pscustomobject]@{
        level = 'error'; code = 'duplicate-title'
        message = "Duplicate visible window title: $($duplicate.Name)"
    })
}
$allowedGroups = @('Workspace', 'Assets', 'Data', 'Advanced')
$allowedDocks = @('left-browser', 'left-inspector', 'center-tabs', 'center-bottom',
    'right-upper', 'right-lower')
foreach ($window in $windows) {
    if ($allowedGroups -notcontains $window.group) {
        $diagnostics.Add([pscustomobject]@{
            level = 'error'; code = 'unknown-window-group'
            message = "Unknown window group for $($window.title): $($window.group)"
        })
    }
    if ($allowedDocks -notcontains $window.defaultDock) {
        $diagnostics.Add([pscustomobject]@{
            level = 'error'; code = 'unknown-dock-region'
            message = "Unknown dock region for $($window.title): $($window.defaultDock)"
        })
    }
}

$runtimeSourcePath = Join-Path $SourceRoot 'winWorkspace.cpp'
$runtimeSource = Get-Content -LiteralPath $runtimeSourcePath -Raw
foreach ($window in $windows) {
    if (-not $functionsSeen.Contains($window.ownerFunction)) {
        $diagnostics.Add([pscustomobject]@{
            level = 'error'; code = 'missing-owner'
            message = "Owner function was not found: $($window.ownerFunction)"
        })
    }
    $runtimePattern = 'FormatSEUIWindowTitle\([^;\r\n]+SEUIWindowId::' +
        [regex]::Escape($window.enum)
    if ($runtimeSource -notmatch $runtimePattern) {
        $diagnostics.Add([pscustomobject]@{
            level = 'error'; code = 'runtime-title-not-catalogued'
            message = "Runtime title does not use the catalog: $($window.title)"
        })
    }
}
foreach ($surface in $surfaces) {
    if (-not $functionsSeen.Contains($surface.ownerFunction)) {
        $diagnostics.Add([pscustomobject]@{
            level = 'error'; code = 'missing-surface-owner'
            message = "Surface owner function was not found: $($surface.ownerFunction)"
        })
    }
}

$windowOutput = foreach ($window in $windows) {
    $windowControls = @($controls |
        Where-Object { $_.windowKeys -contains $window.key })
    [pscustomobject][ordered]@{
        enum = $window.enum
        key = $window.key
        title = $window.title
        purpose = $window.purpose
        ownerFunction = $window.ownerFunction
        group = $window.group
        defaultDock = $window.defaultDock
        defaultVisible = $window.defaultVisible
        controlCount = $windowControls.Count
        sourceFiles = @($windowControls.file | Sort-Object -Unique)
    }
}
$surfaceOutput = foreach ($surface in $surfaces) {
    $surfaceControls = @($controls |
        Where-Object { $_.surfaceKeys -contains $surface.key })
    [pscustomobject][ordered]@{
        enum = $surface.enum
        key = $surface.key
        title = $surface.title
        purpose = $surface.purpose
        ownerFunction = $surface.ownerFunction
        kind = $surface.kind
        controlCount = $surfaceControls.Count
        sourceFiles = @($surfaceControls.file | Sort-Object -Unique)
    }
}
$unmappedControls = @($controls | Where-Object {
    $_.windowKeys.Count -eq 0 -and $_.surfaceKeys.Count -eq 0 })
$map = [pscustomobject][ordered]@{
    schemaVersion = 2
    generatedAt = [DateTime]::UtcNow.ToString('o')
    sourceRoot = $sourceRootDisplay
    summary = [pscustomobject][ordered]@{
        windowCount = $windows.Count
        surfaceCount = $surfaces.Count
        controlCount = $controls.Count
        sourceFileCount = $sourceFiles.Count
        unmappedControlCount = $unmappedControls.Count
        errorCount = @($diagnostics | Where-Object level -eq 'error').Count
    }
    windows = @($windowOutput)
    surfaces = @($surfaceOutput)
    controls = @($controls)
    diagnostics = @($diagnostics)
}

New-Item -ItemType Directory -Force -Path $OutputDirectory | Out-Null
$json = $map | ConvertTo-Json -Depth 8
$jsonPath = Join-Path $OutputDirectory 'ui-map.json'
Write-Utf8NoBom $jsonPath $json

$markdown = [System.Text.StringBuilder]::new()
$null = $markdown.AppendLine('# SkinEditor UI Map')
$null = $markdown.AppendLine()
$null = $markdown.AppendLine("Generated from ``SkinEditor_DX9/uiCatalog.h`` and top-level application sources.")
$null = $markdown.AppendLine()
$null = $markdown.AppendLine("- Windows: $($map.summary.windowCount)")
$null = $markdown.AppendLine("- Flows and shells: $($map.summary.surfaceCount)")
$null = $markdown.AppendLine("- Controls: $($map.summary.controlCount)")
$null = $markdown.AppendLine("- Source files: $($map.summary.sourceFileCount)")
$null = $markdown.AppendLine("- Unmapped controls: $($map.summary.unmappedControlCount)")
$null = $markdown.AppendLine()
$null = $markdown.AppendLine('## Window catalog')
$null = $markdown.AppendLine()
$null = $markdown.AppendLine('| Window | Group | Key | Default dock | Owner | Controls | Purpose |')
$null = $markdown.AppendLine('|---|---|---|---|---|---:|---|')
foreach ($window in $windowOutput) {
    $null = $markdown.AppendLine("| $(Escape-MarkdownCell $window.title) | $(Escape-MarkdownCell $window.group) | ``$(Escape-MarkdownCell $window.key)`` | $(Escape-MarkdownCell $window.defaultDock) | ``$(Escape-MarkdownCell $window.ownerFunction)`` | $($window.controlCount) | $(Escape-MarkdownCell $window.purpose) |")
}
$null = $markdown.AppendLine()
$null = $markdown.AppendLine('## Flow and shell catalog')
$null = $markdown.AppendLine()
$null = $markdown.AppendLine('| Surface | Key | Kind | Owner | Controls | Purpose |')
$null = $markdown.AppendLine('|---|---|---|---|---:|---|')
foreach ($surface in $surfaceOutput) {
    $null = $markdown.AppendLine("| $(Escape-MarkdownCell $surface.title) | ``$(Escape-MarkdownCell $surface.key)`` | $(Escape-MarkdownCell $surface.kind) | ``$(Escape-MarkdownCell $surface.ownerFunction)`` | $($surface.controlCount) | $(Escape-MarkdownCell $surface.purpose) |")
}
foreach ($window in $windowOutput) {
    $null = $markdown.AppendLine()
    $null = $markdown.AppendLine("## $($window.title)")
    $null = $markdown.AppendLine()
    $null = $markdown.AppendLine($window.purpose)
    $null = $markdown.AppendLine()
    $null = $markdown.AppendLine('| Kind | Label | Function | Source |')
    $null = $markdown.AppendLine('|---|---|---|---|')
    $windowControls = @($controls |
        Where-Object { $_.windowKeys -contains $window.key })
    foreach ($control in $windowControls) {
        $source = "$($control.file):$($control.line)"
        $null = $markdown.AppendLine("| $(Escape-MarkdownCell $control.kind) | $(Escape-MarkdownCell $control.visibleLabel) | ``$(Escape-MarkdownCell $control.function)`` | ``$(Escape-MarkdownCell $source)`` |")
    }
}
foreach ($surface in $surfaceOutput) {
    $null = $markdown.AppendLine()
    $null = $markdown.AppendLine("## $($surface.title)")
    $null = $markdown.AppendLine()
    $null = $markdown.AppendLine($surface.purpose)
    $null = $markdown.AppendLine()
    $null = $markdown.AppendLine('| Kind | Label | Function | Source |')
    $null = $markdown.AppendLine('|---|---|---|---|')
    $surfaceControls = @($controls |
        Where-Object { $_.surfaceKeys -contains $surface.key })
    foreach ($control in $surfaceControls) {
        $source = "$($control.file):$($control.line)"
        $null = $markdown.AppendLine("| $(Escape-MarkdownCell $control.kind) | $(Escape-MarkdownCell $control.visibleLabel) | ``$(Escape-MarkdownCell $control.function)`` | ``$(Escape-MarkdownCell $source)`` |")
    }
}
$markdownPath = Join-Path $OutputDirectory 'ui-map.md'
Write-Utf8NoBom $markdownPath $markdown.ToString()

$template = Get-Content -LiteralPath $templatePath -Raw
if (-not $template.Contains('__UI_MAP_JSON__')) {
    throw 'UI map template is missing the __UI_MAP_JSON__ placeholder.'
}
$jsonForHtml = $json.Replace('</script', '<\/script')
$html = $template.Replace('__UI_MAP_JSON__', $jsonForHtml)
$htmlPath = Join-Path $OutputDirectory 'index.html'
Write-Utf8NoBom $htmlPath $html

Write-Host "UI map: $($windows.Count) windows, $($surfaces.Count) flows/shells, $($controls.Count) controls"
Write-Host "JSON: $jsonPath"
Write-Host "Markdown: $markdownPath"
Write-Host "HTML: $htmlPath"

$errors = @($diagnostics | Where-Object level -eq 'error')
if ($Check -and $errors.Count -gt 0) {
    foreach ($diagnostic in $errors) {
        Write-Error "[$($diagnostic.code)] $($diagnostic.message)" -ErrorAction Continue
    }
    exit 1
}

Write-Output $htmlPath

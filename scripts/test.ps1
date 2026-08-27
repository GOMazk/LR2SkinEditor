[CmdletBinding()]
param(
    [string]$ExecutablePath,
    [string]$ResultsDirectory,

    [ValidateRange(1, 600)]
    [int]$TimeoutSeconds = 60
)

$ErrorActionPreference = 'Stop'

$repositoryRoot = Split-Path -Parent $PSScriptRoot
if ([string]::IsNullOrWhiteSpace($ExecutablePath)) {
    $ExecutablePath = Join-Path $repositoryRoot `
        'SkinEditor_DX9\Release\SkinEditor_DX9.exe'
}
if ([string]::IsNullOrWhiteSpace($ResultsDirectory)) {
    $ResultsDirectory = Join-Path $repositoryRoot '.build\test-results'
}

$ExecutablePath = [System.IO.Path]::GetFullPath($ExecutablePath)
$ResultsDirectory = [System.IO.Path]::GetFullPath($ResultsDirectory)
if (-not (Test-Path -LiteralPath $ExecutablePath)) {
    throw "Test executable was not found: $ExecutablePath"
}
New-Item -ItemType Directory -Force -Path $ResultsDirectory | Out-Null

$tests = @(
    [pscustomobject]@{
        Name = 'schema-contract'
        Argument = '--self-test-schema-contract'
    },
    [pscustomobject]@{
        Name = 'ui-contract'
        Argument = '--self-test-ui-contract'
    },
    [pscustomobject]@{
        Name = 'skin-browser'
        Argument = '--self-test-skin-browser'
    },
    [pscustomobject]@{
        Name = 'preview-simulator'
        Argument = '--self-test-preview-simulator'
    },
    [pscustomobject]@{
        Name = 'resolution-estimator'
        Argument = '--self-test-resolution-estimator'
    },
    [pscustomobject]@{
        Name = 'olr-package'
        Argument = '--self-test-olr-package'
    },
    [pscustomobject]@{
        Name = 'asset-metadata'
        Argument = '--self-test-asset-metadata'
    },
    [pscustomobject]@{
        Name = 'pixel-paint'
        Argument = '--self-test-pixel-paint'
    }
)

$workingDirectory = Split-Path -Parent $ExecutablePath
$results = @()
foreach ($test in $tests) {
    Write-Host "[ RUN      ] $($test.Name)"
    $stopwatch = [System.Diagnostics.Stopwatch]::StartNew()
    $exitCode = -1
    $errorMessage = $null
    $process = $null
    try {
        $process = Start-Process -FilePath $ExecutablePath `
            -ArgumentList $test.Argument `
            -WorkingDirectory $workingDirectory `
            -PassThru
        if (-not $process.WaitForExit($TimeoutSeconds * 1000)) {
            $process.Kill()
            $process.WaitForExit()
            $errorMessage = "Timed out after $TimeoutSeconds seconds"
        }
        else {
            $exitCode = $process.ExitCode
            if ($exitCode -ne 0) {
                $errorMessage = "Exited with code $exitCode"
            }
        }
    }
    catch {
        $errorMessage = $_.Exception.Message
    }
    finally {
        if ($null -ne $process -and -not $process.HasExited) {
            $process.Kill()
            $process.WaitForExit()
        }
        $stopwatch.Stop()
    }

    $passed = [string]::IsNullOrEmpty($errorMessage)
    $status = if ($passed) { 'OK' } else { 'FAILED' }
    Write-Host "[ $($status.PadRight(8)) ] $($test.Name) ($([math]::Round($stopwatch.Elapsed.TotalSeconds, 3))s)"
    $results += [pscustomobject]@{
        Name = $test.Name
        Passed = $passed
        ExitCode = $exitCode
        Duration = $stopwatch.Elapsed.TotalSeconds
        Error = $errorMessage
    }
}

$resultPath = Join-Path $ResultsDirectory 'skineditor-self-tests.xml'
$failed = @($results | Where-Object { -not $_.Passed })
$totalSeconds = ($results | Measure-Object -Property Duration -Sum).Sum
$culture = [System.Globalization.CultureInfo]::InvariantCulture
$settings = New-Object System.Xml.XmlWriterSettings
$settings.Indent = $true
$settings.Encoding = New-Object System.Text.UTF8Encoding($false)
$writer = [System.Xml.XmlWriter]::Create($resultPath, $settings)
try {
    $writer.WriteStartDocument()
    $writer.WriteStartElement('testsuite')
    $writer.WriteAttributeString('name', 'SkinEditor self-tests')
    $writer.WriteAttributeString('tests', [string]$results.Count)
    $writer.WriteAttributeString('failures', [string]$failed.Count)
    $writer.WriteAttributeString('time', $totalSeconds.ToString('0.000', $culture))
    foreach ($result in $results) {
        $writer.WriteStartElement('testcase')
        $writer.WriteAttributeString('classname', 'SkinEditor_DX9')
        $writer.WriteAttributeString('name', $result.Name)
        $writer.WriteAttributeString('time', $result.Duration.ToString('0.000', $culture))
        if (-not $result.Passed) {
            $writer.WriteStartElement('failure')
            $writer.WriteAttributeString('message', $result.Error)
            $writer.WriteString("$($result.Name) failed: $($result.Error)")
            $writer.WriteEndElement()
        }
        $writer.WriteEndElement()
    }
    $writer.WriteEndElement()
    $writer.WriteEndDocument()
}
finally {
    $writer.Dispose()
}

Write-Host "JUnit results: $resultPath"
if ($failed.Count -gt 0) {
    Write-Error "$($failed.Count) of $($results.Count) self-tests failed."
    exit 1
}

Write-Host "$($results.Count) self-tests passed."

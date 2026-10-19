# Runs the adjacent Python script, forwarding optional command-line settings.
# Example: .\run_model.ps1 --spring-width 2.0 --name spring_2mm
$pythonCandidates = @(
    'C:\Users\justi\.cache\codex-runtimes\codex-primary-runtime\dependencies\python\python.exe',
    'C:\Justin\Coding\Languages\Python\pyEnv\pyenv-win\versions\3.10.11\python.exe'
)
foreach ($candidate in $pythonCandidates) {
    if (Test-Path -LiteralPath $candidate) {
        & $candidate (Join-Path $PSScriptRoot 'build_model.py') @args
        exit $LASTEXITCODE
    }
}
& python (Join-Path $PSScriptRoot 'build_model.py') @args
exit $LASTEXITCODE

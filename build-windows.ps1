param(
	[string]$BuildDirectory = 'build-windows-release'
)

$ErrorActionPreference = 'Stop'

if ($BuildDirectory -notmatch '^[A-Za-z0-9_.-]+$') {
	throw 'The build folder name may only contain letters, numbers, dots, underscores, and dashes.'
}

& (Join-Path $PSScriptRoot 'bootstrap.ps1')

$vswhere = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio\Installer\vswhere.exe'
if (-not (Test-Path $vswhere)) {
	throw 'Visual Studio 2022 Build Tools with the C++ workload are required for the Windows build.'
}

$vcvars = & $vswhere -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 `
	-find 'VC\Auxiliary\Build\vcvars64.bat' | Select-Object -First 1
if (-not $vcvars) {
	throw 'The 64-bit Microsoft C++ compiler could not be found. Add the C++ workload to Visual Studio Build Tools.'
}

$environment = & cmd.exe /d /c "call `"$vcvars`" >nul && set"
if ($LASTEXITCODE -ne 0) {
	throw 'The Microsoft C++ build environment could not be started.'
}
foreach ($line in $environment) {
	if ($line -match '^([^=]+)=(.*)$') {
		Set-Item -Path "Env:$($matches[1])" -Value $matches[2]
	}
}

$python = (Get-Command python.exe).Source
$env:PYTHONPATH = Join-Path $PSScriptRoot '.tools\ambuild'
$configuredBuild = Join-Path $PSScriptRoot "$BuildDirectory\.ambuild2\graph"

Push-Location $PSScriptRoot
try {
	if (-not (Test-Path $configuredBuild)) {
		& $python configure.py --enable-optimize --out $BuildDirectory
		if ($LASTEXITCODE -ne 0) {
			throw 'The Windows build could not be configured.'
		}
	}
	& $python -c 'from ambuild2.run import cli_run; cli_run()' $BuildDirectory
	if ($LASTEXITCODE -ne 0) {
		throw 'The Windows build failed.'
	}
}
finally {
	Pop-Location
}

$plugin = Join-Path $PSScriptRoot "$BuildDirectory\package\game\csgo\addons\cs2ac\bin\win64\cs2ac.dll"
if (-not (Test-Path $plugin)) {
	throw 'The build finished without producing cs2ac.dll.'
}
Write-Host "Windows package ready: $plugin"

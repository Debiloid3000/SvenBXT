# Create an independent GitHub repository from SvenBXT; NOT a GitHub Actions workflow.
# Requires git and gh (run gh auth login first).
# Usage: ./scripts/create_standalone_repo.ps1 [-DestinationRepo 'Debiloid3000/SvenBXT-Standalone'] [-KeepWorkflows]
param(
    [string]$SourceRepo = 'Debiloid3000/SvenBXT',
    [string]$DestinationRepo = 'Debiloid3000/SvenBXT-Standalone',
    [string]$Directory = 'SvenBXT-Standalone',
    [switch]$KeepWorkflows
)

$ErrorActionPreference = 'Stop'
function EnsureSuccess([string]$Step) {
    if ($LASTEXITCODE -ne 0) { throw "$Step failed: $LASTEXITCODE" }
}

foreach ($exe in @('git', 'gh')) {
    if (-not (Get-Command $exe -ErrorAction SilentlyContinue)) {
        throw "Missing program: $exe"
    }
}
gh auth status
EnsureSuccess 'GitHub authentication'

if (Test-Path -LiteralPath $Directory) { throw "Destination folder already exists: $Directory" }
gh repo view $DestinationRepo --json name --jq .name 2>$null | Out-Null
if ($LASTEXITCODE -eq 0) { throw "GitHub repository already exists: $DestinationRepo" }

git clone --branch new --recurse-submodules "https://github.com/$SourceRepo.git" $Directory
EnsureSuccess 'Clone SvenBXT'
Push-Location $Directory
try {
    git remote remove origin
    EnsureSuccess 'Remove old origin'
    git branch -m main
    EnsureSuccess 'Rename default branch'
    if (-not $KeepWorkflows) {
        git rm -r .github/workflows
        EnsureSuccess 'Remove Actions workflows'
        git -c user.name='Standalone Copy' -c user.email='standalone@users.noreply.github.com' commit -m 'Remove GitHub Actions workflows from standalone source copy'
        EnsureSuccess 'Commit workflow removal'
    }
    gh repo create $DestinationRepo --public --source . --remote origin --push
    EnsureSuccess 'Create independent repository'
    Write-Host "Created: https://github.com/$DestinationRepo"
}
finally {
    Pop-Location
}

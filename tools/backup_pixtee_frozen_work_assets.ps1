#requires -Version 5.1
<#
.SYNOPSIS
Creates an independent, SHA-256-verified, COPY-ONLY snapshot of the frozen
Pixtee Hole 1 Work output and any explicitly supplied approved asset packs.
This script NEVER modifies the original course, artwork or repository.
Does not push to GitHub automatically.
.EXAMPLE
powershell -ExecutionPolicy Bypass -File .\tools\backup_pixtee_frozen_work_assets.ps1 `
  -AssetPaths 'C:\path\to\Pixtee golf assets(1).zip'
.EXAMPLE
powershell -ExecutionPolicy Bypass -File .\tools\backup_pixtee_frozen_work_assets.ps1 `
  -CourseFolder 'C:\Users\86185\Documents\Codex\2026-10-11\can\outputs\pixtee-hole-01-reference-2026-10-12' `
  -AssetPaths @('C:\Pixtee\Pixtee golf assets(1).zip','C:\Pixtee\FinalApprovedAssets')
#>
[CmdletBinding()]
param(
    [string] $CourseFolder = 'C:\Users\86185\Documents\Codex\2026-10-11\can\outputs\pixtee-hole-01-reference-2026-10-12',
    [string[]] $AssetPaths = @(),
    [string] $BackupParent = 'C:\Users\86185\Documents\Codex\pixtee-canonical-backups'
)
Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

function Resolved-Directory([string] $Path) {
    if (-not (Test-Path -LiteralPath $Path -PathType Container)) {
        throw "Directory not found: $Path"
    }
    return (Resolve-Path -LiteralPath $Path).ProviderPath.TrimEnd('\', '/')
}

function Resolved-File([string] $Path) {
    if (-not (Test-Path -LiteralPath $Path -PathType Leaf)) {
        throw "Required file not found: $Path"
    }
    return (Resolve-Path -LiteralPath $Path).ProviderPath
}

function Hash-File([string] $Path) {
    return (Get-FileHash -LiteralPath $Path -Algorithm SHA256).Hash.ToLowerInvariant()
}

$sourceCourse = Resolved-Directory $CourseFolder
foreach ($required in @('index.html', 'COURSE_PRODUCTION_STANDARD.md', 'COURSE_CATALOGUE_50.csv')) {
    [void](Resolved-File (Join-Path $sourceCourse $required))
}
# A backup is stored OUTSIDE the working/frozen Hole 1 folder.
if (-not (Test-Path -LiteralPath $BackupParent -PathType Container)) {
    [void](New-Item -ItemType Directory -Path $BackupParent -Force)
}
$parent = Resolved-Directory $BackupParent
if ($parent.StartsWith(($sourceCourse + '\'), [StringComparison]::OrdinalIgnoreCase) -or
    $sourceCourse.StartsWith(($parent + '\'), [StringComparison]::OrdinalIgnoreCase)) {
    throw 'Backup root and frozen course directory must not be nested within one another.'
}

# Validate all asset package inputs before any copying.
$assetSources = @()
foreach ($item in $AssetPaths) {
    if (Test-Path -LiteralPath $item -PathType Leaf) {
        $assetSources += @{ kind = 'file'; path = (Resolved-File $item) }
    } elseif (Test-Path -LiteralPath $item -PathType Container) {
        $assetSources += @{ kind = 'folder'; path = (Resolved-Directory $item) }
    } else {
        throw "Approved asset pack/folder does not exist: $item"
    }
}
$stamp = (Get-Date).ToUniversalTime().ToString('yyyyMMddTHHmmssZ')
$backup = Join-Path $parent ("PIXTEE_CANONICAL_FROZEN_" + $stamp)
if (Test-Path -LiteralPath $backup) {
    throw "Archive already exists; refusing to overwrite: $backup"
}
[void](New-Item -ItemType Directory -Path $backup)

$results = New-Object 'System.Collections.Generic.List[object]'
function Copy-VerifiedFile([string] $InputFile, [string] $OutputFile, [string] $Group, [string] $Relative) {
    $before = Hash-File $InputFile
    $folder = Split-Path -Parent $OutputFile
    if (-not (Test-Path -LiteralPath $folder -PathType Container)) {
        [void](New-Item -ItemType Directory -Path $folder -Force)
    }
    Copy-Item -LiteralPath $InputFile -Destination $OutputFile -ErrorAction Stop
    $afterSource = Hash-File $InputFile
    $afterCopy = Hash-File $OutputFile
    if ($before -ne $afterSource -or $before -ne $afterCopy) {
        throw "SHA-256 mismatch or concurrent change detected: $InputFile"
    }
    $results.Add([pscustomobject]@{
        group = $Group
        relative_path = ($Relative -replace '\\','/')
        sha256 = $before
        size_bytes = (Get-Item -LiteralPath $InputFile).Length
        source = $InputFile
        archived_at = $OutputFile
    })
}
function Copy-VerifiedDirectory([string] $InputDir, [string] $OutputDir, [string] $Group) {
    [void](New-Item -ItemType Directory -Path $OutputDir -Force)
    $inputRoot = $InputDir.TrimEnd('\', '/')
    $files = @(Get-ChildItem -LiteralPath $inputRoot -Recurse -Force -File | Sort-Object FullName)
    foreach ($file in $files) {
        $relative = $file.FullName.Substring($inputRoot.Length).TrimStart('\', '/')
        $output = Join-Path $OutputDir $relative
        Copy-VerifiedFile $file.FullName $output $Group $relative
    }
}
Copy-VerifiedDirectory $sourceCourse (Join-Path $backup 'FROZEN_HOLE_01') 'frozen-hole-01'

$assetsDir = Join-Path $backup 'APPROVED_ASSET_PACKS'
[void](New-Item -ItemType Directory -Path $assetsDir)
for ($i=0; $i -lt $assetSources.Count; $i++) {
    $source = $assetSources[$i]
    $prefix = "pack_{0:D2}" -f ($i + 1)
    $target = Join-Path $assetsDir $prefix
    if ($source.kind -eq 'file') {
        [void](New-Item -ItemType Directory -Path $target)
        $fileName = Split-Path $source.path -Leaf
        Copy-VerifiedFile $source.path (Join-Path $target $fileName) "approved-asset-$prefix" $fileName
    } else {
        Copy-VerifiedDirectory $source.path $target "approved-asset-$prefix"
    }
}

# Do not claim full asset backup if the caller did not provide the ZIP/folder.
$status = if ($assetSources.Count -gt 0) {
    'LOCAL_COURSE_AND_SUPPLIED_ASSET_PATHS_HASH_VERIFIED'
} else {
    'LOCAL_COURSE_HASH_VERIFIED_ASSET_PACK_MISSING'
}
$manifest = [ordered]@{
    schema = 'pixtee.canonical-local-snapshot.v1'
    created_utc = (Get-Date).ToUniversalTime().ToString('o')
    status = $status
    source_course_folder = $sourceCourse
    independent_backup_folder = $backup
    owner_asset_packages_verified = $assetSources.Count
    required_course_files = @('index.html', 'COURSE_PRODUCTION_STANDARD.md', 'COURSE_CATALOGUE_50.csv')
    file_count = $results.Count
    files = @($results.ToArray())
    github_engine_baseline_sha = '1203f8c440cc96f56e18c3a50336a632ab10efaf'
    github_portrait_baseline_sha = '7b849d29db2a7e37ac039e51c39a80dce0106e0c'
    caveat = 'GitHub archive branches are separate. This is a COPY-ONLY snapshot; ensure an independent second storage/remote copy for disaster recovery. No commercial readiness claim.'
}
$manifestPath = Join-Path $backup 'PIXTEE_CANONICAL_LOCAL_SHA256_MANIFEST.json'
($manifest | ConvertTo-Json -Depth 8) | Set-Content -LiteralPath $manifestPath -Encoding UTF8
$instructions = @'
PIXTEE CANONICAL ARCHIVE — NEVER EDIT THESE COPIES
This folder contains a copy of the frozen Work Hole 1 package and optionally
the explicitly supplied approved art package/folder.
PIXTEE_CANONICAL_LOCAL_SHA256_MANIFEST.json contains the SHA-256 hashes
verified against the source file BEFORE and AFTER copying.

DO NOT EDIT FROZEN_HOLE_01, APPROVED_ASSET_PACKS OR THEIR HASHES.
Create new working versions in a separate folder.
This local copy is NOT a substitute for a second backup drive/cloud copy.
The GitHub engine and portrait framework have separate archival branches.
'@
$instructions | Set-Content -LiteralPath (Join-Path $backup 'READ_ONLY_ARCHIVE_NOTICE.txt') -Encoding UTF8

Write-Host "Pixtee verified COPY-ONLY snapshot: $backup"
Write-Host ("Files hash-verified: {0}; supplied asset packages: {1}" -f $results.Count, $assetSources.Count)
Write-Host "Manifest: $manifestPath"
Write-Host "Status: $status"
if ($assetSources.Count -eq 0) {
    Write-Warning 'APPROVED ASSET ZIP NOT PROVIDED. Local course was backed up, but canonical approved art is NOT preserved by this operation.'
    exit 2
}
exit 0

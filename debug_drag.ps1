# Drag-Drop Debugging Script
# Compare file created by Rigkeeper vs a regular file

Write-Host "=== Drag-Drop File Comparison ===" -ForegroundColor Cyan
Write-Host ""

# Path to Rigkeeper-created file
$rigkeeperFile = Join-Path $env:USERPROFILE "Documents\Rigkeeper\*.kipr"
$rigFiles = Get-ChildItem $rigkeeperFile -ErrorAction SilentlyContinue | Select-Object -First 1

if ($rigFiles) {
    Write-Host "Rigkeeper File Found: $($rigFiles.FullName)" -ForegroundColor Green
    Write-Host ""
    
    # Get detailed file info
    Write-Host "--- File Attributes ---" -ForegroundColor Yellow
    Write-Host "Name: $($rigFiles.Name)"
    Write-Host "Size: $($rigFiles.Length) bytes"
    Write-Host "Created: $($rigFiles.CreationTime)"
    Write-Host "Modified: $($rigFiles.LastWriteTime)"
    Write-Host "Accessed: $($rigFiles.LastAccessTime)"
    Write-Host "Attributes: $($rigFiles.Attributes)"
    Write-Host "Extension: $($rigFiles.Extension)"
    Write-Host ""
    
    # Check file streams (Alternate Data Streams can affect drag-drop)
    Write-Host "--- Alternate Data Streams ---" -ForegroundColor Yellow
    $streams = Get-Item $rigFiles.FullName -Stream * -ErrorAction SilentlyContinue
    if ($streams) {
        $streams | ForEach-Object {
            Write-Host "Stream: $($_.Stream) - Size: $($_.Length)"
        }
    } else {
        Write-Host "No alternate data streams"
    }
    Write-Host ""
    
    # Check security/ACL
    Write-Host "--- Security/ACL ---" -ForegroundColor Yellow
    $acl = Get-Acl $rigFiles.FullName
    Write-Host "Owner: $($acl.Owner)"
    Write-Host "Access Rules:"
    $acl.Access | ForEach-Object {
        Write-Host "  $($_.IdentityReference): $($_.FileSystemRights) - $($_.AccessControlType)"
    }
    Write-Host ""
    
    # Check Zone Identifier (Mark of the Web)
    Write-Host "--- Zone Identifier (Mark of the Web) ---" -ForegroundColor Yellow
    try {
        $zoneId = Get-Content -Path "$($rigFiles.FullName):Zone.Identifier" -ErrorAction SilentlyContinue
        if ($zoneId) {
            Write-Host "FOUND - File is marked as downloaded/untrusted:"
            $zoneId
        } else {
            Write-Host "Not present - File is trusted"
        }
    } catch {
        Write-Host "Not present - File is trusted"
    }
    Write-Host ""
    
    # File handle info
    Write-Host "--- File Handle Check ---" -ForegroundColor Yellow
    $handleCheck = handle.exe $rigFiles.FullName 2>$null
    if ($LASTEXITCODE -eq 0 -and $handleCheck) {
        Write-Host "Open handles found:"
        Write-Host $handleCheck
    } else {
        Write-Host "No open handles (or handle.exe not in PATH)"
    }
    
} else {
    Write-Host "No .kipr files found in Documents\Rigkeeper\" -ForegroundColor Red
    Write-Host "Please create a file using Rigkeeper first."
}

Write-Host ""
Write-Host "=== Suggested Next Steps ===" -ForegroundColor Cyan
Write-Host "1. Use Spy++ to monitor Rig Manager window messages during drag"
Write-Host "2. Compare drag from Explorer vs drag from plugin"
Write-Host "3. Look for WM_DRAGENTER, WM_DRAGOVER, WM_DROP messages"
Write-Host "4. Check if Rig Manager queries file properties differently"

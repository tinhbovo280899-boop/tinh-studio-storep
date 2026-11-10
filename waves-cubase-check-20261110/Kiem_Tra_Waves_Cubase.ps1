#requires -Version 5.1
# Read metadata only. No plugin loading, installer execution or system changes.

function Read-PeHeader {
    param([Parameter(Mandatory=$true)][string]$Path)
    $stream = $null
    $reader = $null
    try {
        $stream = [IO.File]::Open($Path, [IO.FileMode]::Open, [IO.FileAccess]::Read,
            [IO.FileShare]::ReadWrite -bor [IO.FileShare]::Delete)
        $reader = New-Object IO.BinaryReader($stream)
        if ($stream.Length -lt 64 -or $reader.ReadUInt16() -ne 0x5A4D) {
            throw 'Missing DOS header'
        }
        $stream.Position = 60
        $offset = $reader.ReadInt32()
        if ($offset -lt 64 -or ([long]$offset + 26) -gt $stream.Length) {
            throw 'Invalid PE offset'
        }
        $stream.Position = $offset
        if ($reader.ReadUInt32() -ne 0x4550) { throw 'Missing PE signature' }
        $machine = $reader.ReadUInt16()
        $stream.Position = $offset + 20
        $optionalSize = $reader.ReadUInt16()
        if ($optionalSize -lt 2 -or ([long]$offset + 24 + $optionalSize) -gt $stream.Length) {
            throw 'Invalid optional header size'
        }
        $stream.Position = $offset + 24
        $magic = $reader.ReadUInt16()
        $arch = switch ($machine) {
            0x8664 { 'x64' }
            0x014c { 'x86' }
            0xAA64 { 'ARM64' }
            default { 'Other' }
        }
        [pscustomobject]@{
            HeaderReadable = $true
            Architecture = $arch
            X64Header = ($machine -eq 0x8664 -and $magic -eq 0x20B)
            Detail = ('Machine=0x{0:X4}; OptionalMagic=0x{1:X4}' -f $machine,$magic)
        }
    } catch {
        [pscustomobject]@{
            HeaderReadable = $false
            Architecture = 'Unknown'
            X64Header = $false
            Detail = $_.Exception.Message
        }
    } finally {
        if ($null -ne $reader) { $reader.Dispose() }
        elseif ($null -ne $stream) { $stream.Dispose() }
    }
}

function Find-WaveShellItems {
    param([string]$Root, [int]$Depth = 0)
    if (-not (Test-Path -LiteralPath $Root -PathType Container)) { return }
    foreach ($item in @(Get-ChildItem -LiteralPath $Root -Force -ErrorAction Stop)) {
        if ($item.Attributes -band [IO.FileAttributes]::ReparsePoint) { continue }
        if ($item.Name -match '(?i)^WaveShell.*\.vst3$') {
            $item
        } elseif ($item.PSIsContainer -and $Depth -lt 3 -and $item.Extension -ne '.vst3') {
            Find-WaveShellItems -Root $item.FullName -Depth ($Depth + 1)
        }
    }
}

function Get-WaveShellModules {
    param([IO.FileSystemInfo]$Item)
    if (-not $Item.PSIsContainer) { $Item; return }
    $native = Join-Path $Item.FullName 'Contents\x86_64-win'
    if (Test-Path -LiteralPath $native -PathType Container) {
        Get-ChildItem -LiteralPath $native -Filter '*.vst3' -File -Force -ErrorAction Stop
    }
}

function Find-ScanFiles {
    param([string]$Root, [int]$Depth = 0)
    foreach ($item in @(Get-ChildItem -LiteralPath $Root -Force -ErrorAction Stop)) {
        if ($item.Attributes -band [IO.FileAttributes]::ReparsePoint) { continue }
        if ($item.PSIsContainer) {
            if ($Depth -lt 2) { Find-ScanFiles -Root $item.FullName -Depth ($Depth + 1) }
        } elseif ($item.Name -match '(?i)vst3|blocklist|blacklist|scanner' -and
                  $item.Extension -match '(?i)^\.(xml|txt|log|json)$' -and
                  $item.Length -le 4MB) {
            $item
        }
    }
}

function Get-WavesMentions {
    param([string]$Text)
    $matches = [regex]::Matches($Text, '(?i)WaveShell|Waves Audio|Plug-Ins V16')
    foreach ($match in @($matches | Select-Object -First 15)) {
        $start = [Math]::Max(0, $match.Index - 110)
        $length = [Math]::Min(320, $Text.Length - $start)
        [regex]::Replace($Text.Substring($start,$length), '\s+', ' ')
    }
}

function Protect-ReportText {
    param([string]$Text)
    if ($env:USERPROFILE) {
        $Text = [regex]::Replace($Text, [regex]::Escape($env:USERPROFILE),
            [Text.RegularExpressions.MatchEvaluator]{ param($m) '%USERPROFILE%' },
            [Text.RegularExpressions.RegexOptions]::IgnoreCase)
    }
    return $Text
}

function Add-ReportLine {
    param([string]$Text = '')
    $script:ReportLines.Add((Protect-ReportText -Text $Text))
}

function Invoke-WavesDiagnostic {
    if ($env:OS -ne 'Windows_NT') { throw 'Run this diagnostic on the affected Windows machine.' }
    $script:ReportLines = New-Object 'Collections.Generic.List[string]'
    $issues = New-Object 'Collections.Generic.List[string]'
    Add-ReportLine 'WAVES / CUBASE - KIEM TRA FILE VA DAU VET QUET'
    Add-ReportLine ('Thoi gian: ' + (Get-Date -Format 'yyyy-MM-dd HH:mm:ss zzz'))
    Add-ReportLine ('Windows: ' + [Environment]::OSVersion.VersionString)
    Add-ReportLine ('OS64=' + [Environment]::Is64BitOperatingSystem +
        '; PS64=' + [Environment]::Is64BitProcess + '; PowerShell=' + $PSVersionTable.PSVersion)
    Add-ReportLine 'Chi doc thong tin; ket qua KHONG xac nhan plugin nap/chay thanh cong.'
    Add-ReportLine
    $cf64 = $env:CommonProgramW6432
    if (-not $cf64) { $cf64 = $env:CommonProgramFiles }
    $pf32 = [Environment]::GetEnvironmentVariable('ProgramFiles(x86)')
    if (-not $pf32) { $pf32 = $env:ProgramFiles }
    $roots = @(
        [pscustomobject]@{ Label='VST3 toan may x64'; Path=(Join-Path $cf64 'VST3') },
        [pscustomobject]@{ Label='VST3 tai khoan dang chay'; Path=(Join-Path $env:LOCALAPPDATA 'Programs\Common\VST3') }
    )
    $cf32 = [Environment]::GetEnvironmentVariable('CommonProgramFiles(x86)')
    if ($cf32) {
        $roots += [pscustomobject]@{
            Label='VST3 x86 - Cubase 15 khong nap ban 32-bit'
            Path=(Join-Path $cf32 'VST3')
        }
    }
    Add-ReportLine '=== 1. WAVESHELL VST3 ==='
    $machineX64Count = 0
    foreach ($root in $roots) {
        Add-ReportLine ($root.Label + ': ' + $root.Path)
        try {
            $shells = @(Find-WaveShellItems -Root $root.Path)
            if ($shells.Count -eq 0) { Add-ReportLine '  KHONG TIM THAY WaveShell*.vst3' }
            foreach ($shell in $shells) {
                Add-ReportLine ('  WaveShell: ' + $shell.FullName)
                $modules = @(Get-WaveShellModules -Item $shell)
                if ($modules.Count -eq 0) {
                    Add-ReportLine '    KHONG TIM THAY module Contents\x86_64-win\*.vst3'
                    $issues.Add('Bundle WaveShell khong co module x64 tai vi tri VST3 tieu chuan.')
                }
                foreach ($module in $modules) {
                    $pe = Read-PeHeader -Path $module.FullName
                    Add-ReportLine ('    Module: ' + $module.FullName)
                    Add-ReportLine ('    Bytes=' + $module.Length + '; PE=' + $pe.Architecture + '; ' + $pe.Detail)
                    Add-ReportLine ('    Version=' + $module.VersionInfo.FileVersion)
                    Add-ReportLine ('    SHA256=' + (Get-FileHash -LiteralPath $module.FullName -Algorithm SHA256).Hash)
                    if ($root.Label -eq 'VST3 toan may x64' -and $pe.X64Header) { $machineX64Count++ }
                    if (-not $pe.HeaderReadable) { $issues.Add('Co file WaveShell khong doc duoc PE header.') }
                    if ($root.Label -eq 'VST3 toan may x64' -and -not $pe.X64Header) {
                        $issues.Add('WaveShell trong VST3 x64 khong co PE header x64 hop le.')
                    }
                }
            }
        } catch {
            Add-ReportLine ('  LOI DOC: ' + $_.Exception.Message)
            $issues.Add('Khong doc duoc day du WaveShell tai mot duong dan; xem chi tiet.')
        }
    }
    if ($machineX64Count -eq 0) {
        $issues.Add('Chua tim thay WaveShell co PE header x64 trong VST3 toan may.')
    }
    Add-ReportLine
    Add-ReportLine '=== 2. CAC GOI PLUGIN V16 ==='
    $pluginRoot = Join-Path $pf32 'Waves\Plug-Ins V16'
    Add-ReportLine ('Thu muc: ' + $pluginRoot)
    try {
        if (Test-Path -LiteralPath $pluginRoot -PathType Container) {
            $bundles = @(Get-ChildItem -LiteralPath $pluginRoot -Filter '*.bundle' -Force -ErrorAction Stop)
            Add-ReportLine ('So muc *.bundle: ' + $bundles.Count)
            foreach ($bundle in $bundles) { Add-ReportLine ('  ' + $bundle.Name) }
            if ($bundles.Count -eq 0) { $issues.Add('Plug-Ins V16 ton tai nhung khong co muc *.bundle.') }
        } else {
            Add-ReportLine 'KHONG CO thu muc Plug-Ins V16'
            $issues.Add('Thieu thu muc Waves\Plug-Ins V16 tai vi tri mac dinh.')
        }
    } catch { Add-ReportLine ('LOI DOC: ' + $_.Exception.Message) }
    Add-ReportLine
    Add-ReportLine '=== 3. MOT SO FILE MICROSOFT VC++ ==='
    Add-ReportLine 'Danh sach tham khao; chua phan tich dependency cua WaveShell cu the.'
    $system64 = Join-Path $env:SystemRoot 'System32'
    foreach ($name in @('vcruntime140.dll','vcruntime140_1.dll','msvcp140.dll')) {
        $path = Join-Path $system64 $name
        if (Test-Path -LiteralPath $path -PathType Leaf) {
            $file = Get-Item -LiteralPath $path -ErrorAction Stop
            Add-ReportLine ('CO: ' + $name + '; Version=' + $file.VersionInfo.FileVersion)
        } else { Add-ReportLine ('KHONG CO: ' + $name) }
    }
    Add-ReportLine
    Add-ReportLine '=== 4. CUBASE VA CAC FILE SCAN/BLOCKLIST ==='
    Add-ReportLine 'Chi trich doan nhac Waves; khong tim thay dau vet KHONG chung minh scan thanh cong.'
    $cubaseProcesses = @(Get-Process -ErrorAction SilentlyContinue | Where-Object { $_.ProcessName -match '(?i)^Cubase' })
    Add-ReportLine ('Tien trinh Cubase dang chay: ' + $cubaseProcesses.Count)
    $seen = @{}
    $traceFileCount = 0
    $mentionCount = 0
    foreach ($base in @($env:APPDATA,$env:LOCALAPPDATA)) {
        $steinberg = Join-Path $base 'Steinberg'
        if (-not (Test-Path -LiteralPath $steinberg -PathType Container)) { continue }
        foreach ($profile in @(Get-ChildItem -LiteralPath $steinberg -Directory -Filter 'Cubase*' -ErrorAction SilentlyContinue)) {
            Add-ReportLine ('Cau hinh: ' + $profile.FullName)
            try {
                foreach ($file in @(Find-ScanFiles -Root $profile.FullName | Select-Object -First 100)) {
                    if ($seen.ContainsKey($file.FullName)) { continue }
                    $seen[$file.FullName] = $true
                    $traceFileCount++
                    $text = Get-Content -LiteralPath $file.FullName -Raw -ErrorAction Stop
                    $mentions = @(Get-WavesMentions -Text $text)
                    if ($mentions.Count -gt 0) {
                        $mentionCount += $mentions.Count
                        Add-ReportLine ('FILE: ' + $file.FullName)
                        Add-ReportLine ('Sua doi: ' + $file.LastWriteTime.ToString('yyyy-MM-dd HH:mm:ss'))
                        if ($file.Name -match '(?i)blocklist|blacklist') {
                            Add-ReportLine '  CHU Y: file co ten blacklist/blocklist co nhac Waves; can doc tung muc.'
                        }
                        foreach ($mention in $mentions) { Add-ReportLine ('  ' + $mention) }
                    }
                }
            } catch { Add-ReportLine ('LOI DOC SCAN: ' + $_.Exception.Message) }
        }
    }
    Add-ReportLine ('So file scan da doc: ' + $traceFileCount + '; So trich doan Waves: ' + $mentionCount)
    Add-ReportLine
    Add-ReportLine '=== 5. DIEM CAN KIEM TRA TIEP ==='
    if ($issues.Count -eq 0) {
        Add-ReportLine 'Chua thay thieu sot o cac file da kiem tra. Can doi chieu scan trong Cubase.'
    } else {
        foreach ($issue in @($issues | Select-Object -Unique)) { Add-ReportLine ('- ' + $issue) }
    }
    Add-ReportLine
    Add-ReportLine 'Khong nap VST; khong sua registry, cache, blacklist hay license; khong chay Patch.exe.'
    Add-ReportLine 'Khong gui ket qua len mang. Chi tao file TXT nay trong thu muc cua cong cu.'
    $name = 'KetQua_Waves_Cubase_' + (Get-Date -Format 'yyyyMMdd_HHmmss') +
        '_' + [Guid]::NewGuid().ToString('N').Substring(0,6) + '.txt'
    $destination = Join-Path $PSScriptRoot $name
    $encoding = New-Object Text.UTF8Encoding($true)
    [IO.File]::WriteAllLines($destination, $script:ReportLines.ToArray(), $encoding)
    Write-Host
    Write-Host 'Da tao file ket qua:' -ForegroundColor Green
    Write-Host $destination
    Write-Host 'Gui file TXT nay cho em de doi chieu. Cong cu chua sua bo cai.'
}

if ($MyInvocation.InvocationName -ne '.') {
    try { Invoke-WavesDiagnostic }
    catch {
        Write-Host ('Khong tao duoc ket qua: ' + $_.Exception.Message) -ForegroundColor Red
        exit 1
    }
}

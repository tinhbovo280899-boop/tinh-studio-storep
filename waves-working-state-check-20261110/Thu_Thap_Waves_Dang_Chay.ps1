#requires -Version 5.1
# Metadata collection only. Does not load plugins or copy their binaries.

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

function Protect-PathText {
    param([string]$Text)
    if ($env:USERPROFILE) {
        return [regex]::Replace($Text,[regex]::Escape($env:USERPROFILE),
            [Text.RegularExpressions.MatchEvaluator]{param($m) '%USERPROFILE%'},
            [Text.RegularExpressions.RegexOptions]::IgnoreCase)
    }
    return $Text
}

function Test-SensitivePath {
    param([string]$Path)
    return $Path -match '(?i)(^|[\\/])(licenses?|licensing|devices?|tokens?|logs?|caches?)([\\/]|$)|\.(wle|lic|key)$'
}

function Get-SafeFiles {
    param([string]$Path)
    $item=Get-Item -LiteralPath $Path -Force -ErrorAction Stop
    if ($item.Attributes -band [IO.FileAttributes]::ReparsePoint) { return }
    if (Test-SensitivePath $item.FullName) { return }
    if (-not $item.PSIsContainer) { $item; return }
    $queue=New-Object 'Collections.Generic.Queue[string]'
    $queue.Enqueue($item.FullName)
    while ($queue.Count -gt 0) {
        $dir=$queue.Dequeue()
        try { $children=@(Get-ChildItem -LiteralPath $dir -Force -ErrorAction Stop) }
        catch { $script:ReadErrors.Add('Directory: '+(Protect-PathText $dir)+': '+$_.Exception.GetType().Name); continue }
        foreach ($child in $children) {
            if ($child.Attributes -band [IO.FileAttributes]::ReparsePoint) { continue }
            if (Test-SensitivePath $child.FullName) { continue }
            if ($child.PSIsContainer) { $queue.Enqueue($child.FullName) }
            else { $child }
        }
    }
}

function Find-WaveShells {
    param([string]$Path,[int]$Depth=0)
    if (-not (Test-Path -LiteralPath $Path -PathType Container)) { return }
    foreach ($item in @(Get-ChildItem -LiteralPath $Path -Force -ErrorAction Stop)) {
        if ($item.Attributes -band [IO.FileAttributes]::ReparsePoint) { continue }
        if ($item.Name -match '(?i)^WaveShell.*\.(vst3|dll)$') { $item }
        elseif ($item.PSIsContainer -and $item.Extension -ne '.vst3' -and $Depth -lt 3) {
            Find-WaveShells -Path $item.FullName -Depth ($Depth+1)
        }
    }
}

function Test-WavesModule {
    param([string]$Path)
    return $Path -match '(?i)(^|[\\/])Waves(?: Audio| Central|LocalServer)?([\\/]|$)|[\\/]WaveShell[^\\/]*\.(vst3|dll)$'
}

function Get-FileFacts {
    param([string]$Path,[switch]$HashNative)
    $key=$Path.ToLowerInvariant()+'|'+[bool]$HashNative
    if ($script:FactCache.ContainsKey($key)) { return $script:FactCache[$key] }
    $file=Get-Item -LiteralPath $Path -Force -ErrorAction Stop
    $native=$file.Extension -match '(?i)^\.(exe|dll|vst3)$'
    $architecture=''; $version=''; $sha=''
    if ($native) {
        $architecture=(Read-PeHeader -Path $Path).Architecture
        $version=$file.VersionInfo.FileVersion
        if ($HashNative) { $sha=(Get-FileHash -LiteralPath $Path -Algorithm SHA256 -ErrorAction Stop).Hash }
    }
    $result=[pscustomobject]@{
        Path=(Protect-PathText $file.FullName); Bytes=$file.Length
        ModifiedUtc=$file.LastWriteTimeUtc.ToString('o')
        Architecture=$architecture; FileVersion=$version; SHA256=$sha
    }
    $script:FactCache[$key]=$result
    return $result
}

function Get-WavesInstallRoots {
    $pf64=$env:ProgramW6432
    if (-not $pf64) { $pf64=$env:ProgramFiles }
    $cf64=$env:CommonProgramW6432
    if (-not $cf64) { $cf64=$env:CommonProgramFiles }
    $bases=@(
        $pf64,[Environment]::GetEnvironmentVariable('ProgramFiles(x86)'),
        $cf64,[Environment]::GetEnvironmentVariable('CommonProgramFiles(x86)')
    )
    $paths=New-Object 'Collections.Generic.List[string]'
    foreach ($base in $bases) {
        if (-not $base) { continue }
        foreach ($name in @('Waves','Waves Audio','WavesLocalServer','Waves Central')) {
            $paths.Add((Join-Path $base $name))
        }
        foreach ($name in @('VST3','VST2','VstPlugins','Steinberg\VstPlugins')) {
            $path=Join-Path $base $name
            try { foreach ($shell in @(Find-WaveShells -Path $path)) { $paths.Add($shell.FullName) } }
            catch { $script:ReadErrors.Add('WaveShell search: '+(Protect-PathText $path)) }
        }
    }
    foreach ($base in @($env:ProgramData,$env:PUBLIC,$env:APPDATA,$env:LOCALAPPDATA)) {
        if ($base) { $paths.Add((Join-Path $base 'Waves Audio')) }
    }
    foreach ($path in @($paths | Select-Object -Unique)) {
        if (Test-Path -LiteralPath $path) { $path }
    }
}

function Get-LoadedWavesHosts {
    $session=(Get-Process -Id $PID -ErrorAction Stop).SessionId
    foreach ($process in @(Get-Process -ErrorAction Stop | Where-Object {$_.SessionId -eq $session -and $_.Id -ne $PID})) {
        try {
            $modules=@($process.Modules)
            $waves=@($modules | Where-Object {Test-WavesModule $_.FileName})
            if ($waves.Count -eq 0) { continue }
            $hostPath=$process.MainModule.FileName
            $hostArch=(Read-PeHeader -Path $hostPath).Architecture
            foreach ($module in $modules) {
                try {
                    $isWaves=Test-WavesModule $module.FileName
                    $facts=Get-FileFacts -Path $module.FileName -HashNative:$isWaves
                    [pscustomobject]@{
                        Host=$process.ProcessName; ProcessId=$process.Id; HostArchitecture=$hostArch
                        Module=$module.ModuleName; WavesCandidate=$isWaves
                        Path=$facts.Path; Architecture=$facts.Architecture
                        Version=$facts.FileVersion; SHA256=$facts.SHA256
                    }
                } catch { $script:ReadErrors.Add('Loaded module: '+$module.ModuleName+': '+$_.Exception.GetType().Name) }
            }
        } catch { $script:DeniedProcessCount++ }
    }
}

function Test-RegistryPathValue {
    param([string]$Key,[string]$Name,$Value)
    if (Test-SensitivePath $Key) { return $false }
    if ($Name -match '(?i)licen|token|serial|device|machine|challenge|password|secret|key') { return $false }
    return ($Name -match '(?i)path|dir|folder') -and ($Value -is [string]) -and
        ($Value -match '^(?:[A-Za-z]:\\|\\\\)')
}

function Read-WavesRegistryBranch {
    param($Base,[string]$Path,[string]$ViewName,[int]$Depth=0)
    if (Test-SensitivePath $Path) { return }
    $key=$null
    try {
        $key=$Base.OpenSubKey($Path,$false)
        if ($null -eq $key) { return }
        foreach ($name in $key.GetValueNames()) {
            if ($name -notmatch '(?i)path|dir|folder' -or
                $name -match '(?i)licen|token|serial|device|machine|challenge|password|secret|key') { continue }
            $value=$key.GetValue($name)
            if (Test-RegistryPathValue -Key $Path -Name $name -Value $value) {
                [pscustomobject]@{View=$ViewName; Key=$Path; Name=$name; Value=(Protect-PathText $value)}
            }
        }
        if ($Depth -lt 4) {
            foreach ($child in $key.GetSubKeyNames()) {
                Read-WavesRegistryBranch -Base $Base -Path ($Path+'\'+$child) -ViewName $ViewName -Depth ($Depth+1)
            }
        }
    } catch { $script:ReadErrors.Add('Registry read: '+$ViewName+' '+$Path) }
    finally { if ($null -ne $key) { $key.Dispose() } }
}

function Get-WavesRegistryPaths {
    foreach ($hive in @([Microsoft.Win32.RegistryHive]::LocalMachine,[Microsoft.Win32.RegistryHive]::CurrentUser)) {
        foreach ($view in @([Microsoft.Win32.RegistryView]::Registry64,[Microsoft.Win32.RegistryView]::Registry32)) {
            $base=$null
            try {
                $base=[Microsoft.Win32.RegistryKey]::OpenBaseKey($hive,$view)
                foreach ($key in @('Software\Waves Audio','Software\Waves')) {
                    Read-WavesRegistryBranch -Base $base -Path $key -ViewName ($hive.ToString()+'/'+$view.ToString())
                }
            } catch { $script:ReadErrors.Add('Registry unavailable: '+$hive+'/'+$view) }
            finally { if ($null -ne $base) { $base.Dispose() } }
        }
    }
}

function Get-WindowsFacts {
    try {
        $os=Get-ItemProperty -LiteralPath 'HKLM:\SOFTWARE\Microsoft\Windows NT\CurrentVersion' -ErrorAction Stop
        return [pscustomobject]@{
            Build=$os.CurrentBuildNumber; UBR=$os.UBR; DisplayVersion=$os.DisplayVersion; ReleaseId=$os.ReleaseId
            ApiVersion=[Environment]::OSVersion.VersionString
        }
    } catch { return [pscustomobject]@{Build='Unknown'; ApiVersion=[Environment]::OSVersion.VersionString} }
}

function Invoke-WavesSnapshot {
    if ($env:OS -ne 'Windows_NT') { throw 'Run on the Windows machine where Waves now works.' }
    $script:ReadErrors=New-Object 'Collections.Generic.List[string]'
    $script:FactCache=@{}
    $script:DeniedProcessCount=0
    $stamp=(Get-Date -Format 'yyyyMMdd_HHmmss')+'_'+[Guid]::NewGuid().ToString('N').Substring(0,6)
    $output=Join-Path $PSScriptRoot ('KetQua_Waves_'+$stamp)
    New-Item -ItemType Directory -Path $output -ErrorAction Stop | Out-Null
    Write-Host '[1/4] Doc module DAW dang nap. Giu plugin Waves dang mo...'
    $loaded=@(Get-LoadedWavesHosts)
    $loaded | Export-Csv -LiteralPath (Join-Path $output 'Loaded-modules.csv') -NoTypeInformation -Encoding UTF8
    Write-Host '[2/4] Lap danh sach file Waves; chi bam hash file native...'
    $roots=@(Get-WavesInstallRoots)
    $seen=@{}
    $rows=New-Object 'Collections.Generic.List[object]'
    $limitHit=$false
    foreach ($root in $roots) {
        Write-Host ('  '+(Protect-PathText $root))
        try {
            foreach ($file in Get-SafeFiles -Path $root) {
                $key=$file.FullName.ToLowerInvariant()
                if ($seen.ContainsKey($key)) { continue }
                $seen[$key]=$true
                if ($rows.Count -ge 50000) { $limitHit=$true; break }
                try { $rows.Add((Get-FileFacts -Path $file.FullName -HashNative)) }
                catch { $script:ReadErrors.Add('File: '+(Protect-PathText $file.FullName)+': '+$_.Exception.GetType().Name) }
            }
        } catch { $script:ReadErrors.Add('Root: '+(Protect-PathText $root)+': '+$_.Exception.GetType().Name) }
        if ($limitHit) { break }
    }
    $rows.ToArray() | Export-Csv -LiteralPath (Join-Path $output 'Waves-files.csv') -NoTypeInformation -Encoding UTF8
    Write-Host '[3/4] Doc mot so duong dan registry va runtime...'
    $registry=@(Get-WavesRegistryPaths)
    ConvertTo-Json -InputObject $registry -Depth 5 |
        Set-Content -LiteralPath (Join-Path $output 'Waves-registry-paths.json') -Encoding UTF8
    $runtime=New-Object 'Collections.Generic.List[object]'
    foreach ($dir in @('System32','SysWOW64')) {
        foreach ($name in @('vcruntime140.dll','vcruntime140_1.dll','msvcp140.dll','ucrtbase.dll')) {
            $path=Join-Path (Join-Path $env:SystemRoot $dir) $name
            if (Test-Path -LiteralPath $path -PathType Leaf) {
                try { $runtime.Add((Get-FileFacts -Path $path -HashNative)) }
                catch { $script:ReadErrors.Add('Runtime: '+$dir+'/'+$name) }
            }
        }
    }
    $runtime.ToArray() | Export-Csv -LiteralPath (Join-Path $output 'Runtime-files.csv') -NoTypeInformation -Encoding UTF8
    $summary=[ordered]@{
        Tool='Waves working-state metadata 1.0'; Captured=(Get-Date -Format 'o')
        Windows=(Get-WindowsFacts); PowerShell=$PSVersionTable.PSVersion.ToString()
        LoadedHostNames=@($loaded | Select-Object -ExpandProperty Host -Unique)
        LoadedModuleCount=$loaded.Count; FileCount=$rows.Count
        Roots=@($roots | ForEach-Object {Protect-PathText $_})
        RegistryPathCount=$registry.Count; DeniedProcessCount=$script:DeniedProcessCount
        ReadErrors=$script:ReadErrors.ToArray(); InventoryLimitHit=$limitHit
        Scope='Metadata after installation. No before/after history, no confirmation of Cubase loading, no complete dependency or activation diagnosis.'
    }
    ConvertTo-Json -InputObject $summary -Depth 7 |
        Set-Content -LiteralPath (Join-Path $output 'Summary.json') -Encoding UTF8
    @(
        'Mo file Summary.json de xem so lieu va loi doc.'
        'Loaded-modules.csv ghi duong dan cac module cua host da phat hien nap Waves.'
        'Neu LoadedHostNames rong: chua bat duoc module; khong ket luan Waves khong chay.'
        'Chi ghi metadata; khong nap VST, chep binary, sua registry/license, cai/go phan mem hay upload.'
        'Khong chua noi dung file license. Bo qua cac thu muc license, device, token, cache va log.'
        'Can doi chieu voi bo dong goi; danh sach nay chua chi ra nguyen nhan duy nhat.'
    ) | Set-Content -LiteralPath (Join-Path $output 'Doc_truoc.txt') -Encoding UTF8
    Write-Host '[4/4] Nen bao cao metadata...'
    $zip=$output+'.zip'
    Compress-Archive -LiteralPath $output -DestinationPath $zip -CompressionLevel Optimal -ErrorAction Stop
    Write-Host ('Da tao: '+$zip) -ForegroundColor Green
    if ($loaded.Count -eq 0) {
        Write-Host 'CHU Y: Chua bat duoc host nap Waves. Mo mot plugin Waves trong DAW roi chay lai neu can.' -ForegroundColor Yellow
    }
    if ($script:ReadErrors.Count -gt 0 -or $limitHit) {
        Write-Host 'Bao cao co muc khong doc duoc hoac gioi han; xem Summary.json.' -ForegroundColor Yellow
    }
    Write-Host 'Chi gui ZIP KetQua_Waves_*.zip; khong can gui cac file DLL/VST3.'
}

if ($MyInvocation.InvocationName -ne '.') {
    try { $ErrorActionPreference='Stop'; Invoke-WavesSnapshot }
    catch { Write-Host ('LOI: '+$_.Exception.Message) -ForegroundColor Red; exit 1 }
}

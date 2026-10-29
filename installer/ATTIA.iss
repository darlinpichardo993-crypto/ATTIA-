; ATTIA VST - Made By ORYMY  (instalador)
#define AppVer "1.0.0"

[Setup]
AppId={{5C1D9E42-3B7A-4F08-9D6E-A770A0000002}
AppName=ATTIA VST
AppVersion={#AppVer}
AppPublisher=ORYMY
VersionInfoVersion=1.0.0.0
VersionInfoCompany=ORYMY
VersionInfoDescription=ATTIA VST - Synth Effect - Instalador
VersionInfoProductName=ATTIA VST
DefaultDirName={autopf}\ORYMY\ATTIA
DefaultGroupName=ORYMY
DisableProgramGroupPage=yes
OutputDir=..\dist
OutputBaseFilename=setup
; Sin compresion: el banco de ondas (90 MB de datos reales) hace que setup.exe pese ~100 MB
Compression=none
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
PrivilegesRequired=admin
WizardStyle=modern
WizardImageFile=wizard_big.bmp
WizardSmallImageFile=wizard_small.bmp
SetupIconFile=attia.ico
UninstallDisplayIcon={app}\attia.ico
InfoBeforeFile=info_es.txt
ShowLanguageDialog=no
LanguageDetectionMethod=none
DiskSpanning=no

[Languages]
Name: "spanish"; MessagesFile: "compiler:Languages\Spanish.isl"

[Messages]
spanish.WelcomeLabel1=Bienvenido a ATTIA VST
spanish.WelcomeLabel2=Este asistente instalara [name/ver] en tu computadora.%n%nSynth Effect hecho por ORYMY para FL Studio y cualquier DAW compatible con VST3.%n%n  20.000 presets unicos%n  15.000 arpegios (escalas mayores y menores)%n  Banco Resynth de 11.000 ondas%n  Compresor y limitador integrados%n%nCierra FL Studio antes de continuar.
spanish.FinishedHeadingLabel=ATTIA VST esta listo
spanish.FinishedLabel=ATTIA se instalo correctamente.%n%nAbre FL Studio > Options > Manage plugins > Find installed plugins y busca ATTIA.

[Types]
Name: "full"; Description: "Instalacion completa"
Name: "custom"; Description: "Personalizada"; Flags: iscustom

[Components]
Name: "vst3"; Description: "ATTIA VST3 (FL Studio, Ableton, Reaper, Cubase...)"; Types: full custom; Flags: fixed
Name: "bank"; Description: "Banco de ondas Resynth: 11.000 ondas (~90 MB)"; Types: full custom; Flags: fixed
Name: "standalone"; Description: "ATTIA Standalone (aplicacion independiente)"; Types: full

[Files]
Source: "..\build\ATTIA_artefacts\Release\VST3\ATTIA.vst3\*"; DestDir: "{commoncf64}\VST3\ATTIA.vst3"; Flags: ignoreversion recursesubdirs createallsubdirs; Components: vst3
Source: "payload\attia_bank.bin"; DestDir: "{commonappdata}\ORYMY\ATTIA"; Flags: ignoreversion; Components: bank
Source: "..\build\ATTIA_artefacts\Release\Standalone\ATTIA.exe"; DestDir: "{app}"; Flags: ignoreversion; Components: standalone
Source: "attia.ico"; DestDir: "{app}"; Flags: ignoreversion

[Icons]
Name: "{autoprograms}\ORYMY\ATTIA Standalone"; Filename: "{app}\ATTIA.exe"; Components: standalone
Name: "{autoprograms}\ORYMY\Desinstalar ATTIA"; Filename: "{uninstallexe}"

[UninstallDelete]
Type: filesandordirs; Name: "{commonappdata}\ORYMY\ATTIA"

[Run]
Filename: "{app}\ATTIA.exe"; Description: "Abrir ATTIA Standalone"; Flags: nowait postinstall skipifsilent unchecked; Components: standalone

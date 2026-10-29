# ATTIA VST — Synth Effect · Made By ORYMY

Sintetizador + efectos en formato **VST3** (el formato actual que usa FL Studio; el archivo se llama `ATTIA.vst3`).

## Qué tiene
- **Resynth Osc x2**: osciladores de banco de ondas (11.000 ondas en 11 familias: Sine Bloom, Odd Glass, Formant Choir, Bowed Strings, Plucked Strings, Sub Bass, FM Metal, Organ Drawbars, Alien Birds, Warm Tines, Spectral Rooms). Cada serie de 50 ondas se transforma suavemente, así que *Wave* + *Morph* barren como un wavetable. Incluye *Warp* (distorsión de fase), octava/semitono/fine, unison hasta 5 voces.
- Filtro LP/BP/HP + envolvente + key-tracking, envolvente de amplitud, LFO (Cutoff/Pitch/Wave/Amp/Warp), vibrato.
- **20.000 presets únicos** (nombre propio cada uno) en **20 secciones de 1.000**: Ambient Pads, Evolving Atmospheres, Warm Keys, Glass Keys & Piano, Violins & Solo Strings, Orchestral Strings, Nylon Guitars, Electric & Steel Guitars, Reggaeton Plucks, Reggaeton Leads, Dembow Bass & Subs, Latin Trap Bells, Choir & Vocal Pads, Synth Leads, Brass & Winds, Bachata & Merengue Hooks, Bells & Mallets, Risers & Impacts, Glitch & Resynth FX, Dark Drones.
- **15.000 arpegios** (15 bancos x 1.000): loops de melodía (motivo + respuestas) sobre escalas mayores, menores, armónica, dórica, frigia, pentatónicas, mixolidia, lidia; patrones de dembow 3+3+2, trance, trap, stabs de acordes, bajo caminando... Raíz = nota más grave que mantengas; puedes forzar la escala (*Scale*) para pasar cualquier patrón a mayor o menor. Sincronizado al BPM del host, swing, gate, 1–4 octavas.
- **Compresor + limitador a -1 dBFS** integrados (con medidores GR / OUT), todos los presets salen con volumen controlado. Drive, chorus/phaser/flanger, delay ping-pong y reverb.

## Compilar en GitHub (sin instalar nada en tu PC)
1. Crea un repositorio vacío en GitHub.
2. En esta carpeta:
   ```
   git init
   git add .
   git commit -m "ATTIA 1.0"
   git branch -M main
   git remote add origin https://github.com/TU_USUARIO/ATTIA.git
   git push -u origin main
   ```
3. En GitHub: **Actions → Build ATTIA** (la primera vez tarda ~15 min).
4. En **Artifacts** descarga **ATTIA-setup** → `setup.exe` (~100 MB).
5. FL Studio: *Options → Manage plugins → Find installed plugins* → **ATTIA**.

Release con setup.exe adjunto: `git tag v1.0.0 && git push --tags`.

## Por qué pesa ~100 MB
`setup.exe` instala `attia_bank.bin` (90 MB): las 11.000 ondas reales que usan los osciladores, se generan en el build con `tools/gen_bank.py`. Sin compresión, el instalador pesa lo que contiene. Si el banco no se encuentra, ATTIA arranca con 16 ondas básicas y lo avisa en pantalla.

## Local (opcional)
```
pip install numpy
python tools/gen_bank.py installer/payload/attia_bank.bin
cmake -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release --target ATTIA_VST3 ATTIA_Standalone
```
Luego compila `installer/ATTIA.iss` con Inno Setup.

## Estructura
- `Source/Params.h` parámetros y generador de los 20.000 presets · `Source/PresetNames.h` nombres (editables)
- `Source/ArpData.h` generador de los 15.000 arpegios · `Source/ArpNames.h` nombres
- `Source/WaveBank.h` carga del banco · `Source/PluginProcessor.*` motor · `Source/PluginEditor.*` interfaz
- `tools/gen_bank.py`, `tools/gen_names.py` · `installer/` script, imágenes e ícono del instalador

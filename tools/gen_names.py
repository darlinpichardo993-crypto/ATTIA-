#!/usr/bin/env python3
"""Genera Source/PresetNames.h (20.000 nombres unicos) y Source/ArpNames.h (15.000 nombres unicos)."""
import random, os
ADJ = """Amber Velvet Frozen Hollow Distant Silent Golden Slow Floating Midnight Pearl Ashen Gentle Endless Drowsy Lunar Misty Crystal
Ivory Shadowed Breathing Weightless Faded Sleeping Cobalt Saffron Moonlit Hushed Opal Heavy Glacial Dreamy Thin Wide Soft Coral Dusk
Neon Solar Rising Wild Savage Shining Electric Furious Radiant Rapid Fierce Daring Mighty Atomic Blazing Sharp Cosmic Royal Rogue
Turbo Hyper Crimson Phantom Silver Iron Primal Stellar Lone Reckless Brave Vivid Scarlet Bold Mellow Dusty Smoky Honeyed Crisp
Tender Vintage Silky Rounded Bluesy Candlelit Rainy Sunday Cozy Lazy Classic Muted Sweet Hazy Plush Retro Quiet Lonely Whispering
Old Soulful Lush Tropical Caribbean Sunset Midday Nocturnal Urban Street Lucky Secret Hidden Wicked Spicy Smooth Slick Fresh Cool Warm
Hot Cold Dark Bright Loud Tiny Giant Elegant Humble Noble Ancient Modern Future Gentle Burning Frosted Glossy Gilded Painted Woven""".split()
ADJ = list(dict.fromkeys(ADJ))

N = {
 "Ambient Pads": "Cathedral Horizon Aurora Fog Lagoon Canopy Nebula Sanctuary Drift Veil Halo Tundra Monsoon Atrium Mirage Glacier Cloudbank Lullaby Eclipse Reverie Oasis Twilight Mist Harbor Dunes Cosmos Afterglow Vapor Meadow Tidepool",
 "Evolving Atmospheres": "Weather Tide Spiral Current Bloom Orbit Storm Climate Season Echoes Fractal Pulse Voyage Cycle Ripple Wander Haze Monolith Cavern Wind Plain Delta Riverbed Ether Static Breath Garden Labyrinth Gravity Frontier",
 "Warm Keys": "Parlor Stage Lounge Studio Upright Tine Reed Chapel Cabaret Motel Ballroom Jazzclub Balcony Vinyl Rooftop Cassette Diner Lobby Spinet Console Saloon Gallery Terrace Cellar Theater Quartet Ballad Nocturne Interlude Serenade Matinee",
 "Glass Keys & Piano": "Grand Piano Celeste Prism Icicle Chime Spire Mirror Crystal Window Lantern Raindrop Pane Facet Snowfall Filament Chandelier Ballroom Glimmer Etude Sonata Prelude Waltz Rondo Minuet Cadence Aria Fugue Overture Intermezzo",
 "Violins & Solo Strings": "Violin Fiddle Viola Cello Bow Sonata Caprice Solo Rosin Cadenza Largo Adagio Rhapsody Partita Concerto Gypsy Lament Vibrato Legato Tremolo Pizzicato Romance Nocturne Elegy Lullaby Tango Bolero Habanera Passion Sorrow",
 "Orchestral Strings": "Ensemble Section Hall Symphony Orchestra Tutti Swell Crescendo Cinema Score Theme Legacy Epic Saga Overture Finale Maestro Conservatory Capital Empire Dynasty Fanfare Requiem Anthem Legend Odyssey Triumph Memoir Heritage Kingdom",
 "Nylon Guitars": "Classical Flamenco Bossa Serenata Tablao Pluck Fingerstyle Arpeggio Campo Hacienda Plaza Cantina Mariachi Trova Rumba Vals Zapateado Solera Duende Cordoba Sevilla Andalucia Sierra Siesta Cuerdas Rasgueo Canto Romance Caravana",
 "Electric & Steel Guitars": "Strat Jangle Twang Chime Riff Lick Reverb Surf Cruiser Highway Amp Stack Clean Crunch Texas Nashville Memphis Delta Blues Dust Route Garage Roadhouse Slide Bottleneck Pedalboard Tremolo Spring Sundown Mustang",
 "Reggaeton Plucks": "Perreo Dembow Calle Barrio Fuego Noche Flow Gata Latido Humo Malianteo Caribe Isla Brisa Ritmo Sandunga Madrugada Chulo Corillo Trono Playa Rumba Caliente Sabor Fiesta Vibra Tigueraje Reggaeton Bellaqueo Discoteca",
 "Reggaeton Leads": "Corona Fuego Trueno Bandera Mensaje Himno Rey Reina Estrella Cometa Relampago Guaguanco Flow Hit Verso Coro Estribillo Megafono Tarima Escenario Cabina Radio Parlante Luces Pista Combo Rumbero Dominicano Boricua Latino",
 "Dembow Bass & Subs": "Subwoofer Bombo Canon Terremoto Trueno Cimiento Motor Tambor Latigo Martillo Bunker Sotano Maquina Volcan Marea Peso Tanque Bajo Cadena Hierro Mazo Pilote Fuelle Ola Bafle Grave Cemento Rugido Temblor Abismo",
 "Latin Trap Bells": "Campana Cristal Hielo Diamante Cadena Lujo Joya Dorado Plata Brillo Cartier Penthouse Yate Neon Skyline Medianoche Flexin Drip Gemas Vitrina Charm Tesoro Moneda Reloj Corona Lingote Pulsera Zafiro Rubi",
 "Choir & Vocal Pads": "Coro Aleluya Gospel Angeles Vespers Madrigal Cantata Hymnal Ave Misa Coral Salmo Plegaria Voces Sirena Eco Cielo Altar Paloma Rezo Bendicion Sinfonia Ensueno Susurro Gregoriano Letania Oracion Acapella Clarity",
 "Synth Leads": "Laser Comet Siren Rocket Falcon Flare Spear Arrow Voltage Strike Cobra Phoenix Thunder Dagger Beacon Raptor Surge Banner Spark Torch Javelin Orbit Vortex Mantis Saber Zenith Tempest Titan Hornet",
 "Brass & Winds": "Trumpet Horn Trombone Flugel Sax Clarinet Flute Oboe Piccolo Fanfare Brass Section Bugle Cornet Tuba Whistle Panpipe Quena Zampona Ocarina Reed Mouthpiece Valve Slide Embouchure Swell Salsa Merengue Bachata Charanga",
 "Bachata & Merengue Hooks": "Bachata Merengue Guitarra Requinto Tambora Guira Bongo Amargue Sentimiento Corazon Besos Mulata Cibao Santo Domingo Quisqueya Malecon Colmado Mamajuana Palmera Batey Tipico Perico Acordeon Sabor Raiz Cielito Amor Desvelo Serenata",
 "Bells & Mallets": "Bell Carillon Gong Glockenspiel Celesta Star Musicbox Windchime Triangle Bowl Tubular Lantern Tinkle Sleigh Gamelan Handbell Marimba Vibraphone Kalimba Xylophone Steelpan Belfry Steeple Temple Pagoda Fountain Stardust Mallet Cowbell Agogo",
 "Risers & Impacts": "Riser Sweep Impact Whoosh Boom Slam Hit Rumble Swell Build Drop Crash Collapse Surge Launch Ignition Blast Shockwave Detonation Rise Climb Ascent Tension Release Thunder Punch Stinger Downlifter Uplifter Reverse",
 "Glitch & Resynth FX": "Glitch Stutter Resynth Fragment Shard Circuit Static Buffer Bitcrush Scramble Signal Packet Pixel Data Cache Overflow Segfault Matrix Transmission Decoder Signal Hiss Byte Loop Warp Morph Splice Slice Tape Datamosh",
 "Dark Drones": "Drone Abyss Void Crypt Tomb Shadow Grave Eclipse Monolith Ritual Omen Dread Sepulcher Catacomb Cavern Night Sorrow Haunt Wraith Specter Gloom Shroud Obsidian Dirge Hollow Mourning Underworld Vigil Pit",
}
SECTIONS = list(N.keys())
assert len(SECTIONS) == 20

ANOUN = {
 "Major Melody Loops": "Sunrise Parade Meadow Carnival Picnic Skyline Journey Bloom Summit Anthem Horizon Garden Promise Smile Voyage Daybreak",
 "Minor Melody Loops": "Rain Memory Shadow Autumn Letter Window Echo Lament Station Midnight Thought Whisper Longing Farewell Cloud Silhouette",
 "Harmonic Minor Drama": "Duel Tragedy Opera Curtain Villain Revenge Tempest Fate Mask Cathedral Betrayal Crown Siege Verdict Sonata Requiem",
 "Dorian Groove": "Groove Swing Alley Funk Pocket Cruise Strut Shuffle Lounge Velvet Rhythm Stride Roll Vibe Soul Walk",
 "Dembow Reggaeton Arps": "Perreo Dembow Calle Barrio Fuego Noche Gata Latido Malianteo Bellaqueo Corillo Rumba Discoteca Sandunga Chulo Pista",
 "Pentatonic Hooks": "Hook Chorus Catch Mantra Jingle Riff Motif Refrain Chant Melody Tag Phrase Lick Lick Loop Tune",
 "Phrygian Dark": "Flamenco Desert Dagger Serpent Eclipse Sand Ruin Temple Scorpion Mirage Oath Veil Canyon Shrine Ember Nomad",
 "Mixolydian Pop": "Radio Chart Summer Highway Sunset Weekend Festival Neon Disco Polaroid Rooftop Cruise Boulevard Daydream Sparkle Fireworks",
 "Trance Gates": "Gate Pulse Rave Peak Lift Surge Strobe Laser Euphoria Voltage Tunnel Rush Dawn Momentum Breakdown Energy",
 "Latin Trap Arps": "Drip Flexin Cadena Penthouse Medianoche Yate Skyline Lujo Neon Diamante Humo Callejon Hielo Corona Zafiro Trapeo",
 "Ambient Cascades": "Waterfall Raindrop Stream Canopy Fountain Glacier Mist Dewdrop Lagoon Aurora Orchard Drizzle Brook Prism Starfall Tide",
 "Pluck Sequences": "Pluck Harp Marimba Kalimba Pizzicato Droplet Pebble Spring Sprout Petal Zither Clockwork Music Box Dart Tick Spark",
 "Chord Stab Rhythms": "Stab Hit Punch Shot Slam Strike Blast Clap Snap Knock Burst Jab Flash Crack Bang Thump",
 "Bass Walk Patterns": "Walk Stride Anchor Foundation Piston Tremor Engine Pulse Roll Stomp Drive March Rumble Descent Climb Tread",
 "Lydian Dream": "Dream Cloud Wonder Starlight Fantasy Lullaby Floating Daydream Fable Rainbow Orbit Bubble Nebula Carousel Fairytale Reverie",
}
ARPS = list(ANOUN.keys())
assert len(ARPS) == 15
MOVE = "Rolling Bouncing Climbing Falling Spinning Skipping Cascading Gliding Looping Dancing Twinkling Swinging Marching Rising Drifting Fluttering Pulsing Flowing".split()
TAGS = ["Wide", "Air", "Dry", "Lite", "Alt", "Mk2", "Prime", "Deluxe", "Soft", "Raw", "Live", "Duo"]

def build(groups, adjs, count_each, seed):
    rng = random.Random(seed); used = set(); out = []
    for nouns in groups:
        nouns = nouns.split(); cur = 0; guard = 0
        while cur < count_each:
            guard += 1; assert guard < 400000
            nm = rng.choice(adjs) + " " + rng.choice(nouns)
            if rng.random() < 0.2: nm += " " + rng.choice(TAGS)
            if nm in used: continue
            used.add(nm); out.append(nm); cur += 1
    return out

def write(path, arr, name, count, comment):
    assert len(arr) == count and len(set(arr)) == count
    with open(path, "w") as f:
        f.write("#pragma once\n// %s\nnamespace attia {\ninline const char* const %s[%d] = {\n" % (comment, name, count))
        for i in range(0, count, 8):
            f.write("    " + ", ".join('"%s"' % x.replace("ñ", "n").replace("á", "a") for x in arr[i:i + 8]) + ",\n")
        f.write("};\n}\n")

here = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "Source")
pn = build([N[s] for s in SECTIONS], ADJ, 1000, 77)
write(os.path.join(here, "PresetNames.h"), pn, "kPresetNames", 20000, "20.000 nombres de preset (1000 por seccion)")
an = build([ANOUN[s] for s in ARPS], ADJ + MOVE * 3, 1000, 78)
write(os.path.join(here, "ArpNames.h"), an, "kArpNames", 15000, "15.000 nombres de arpegio (1000 por banco)")
print(pn[0], "|", pn[8000], "|", pn[19999], "||", an[0], "|", an[4000], "|", an[14999])

# Wauvio

A C++17 (or newer) audio synthesis engine and music framework. What started as a single synthesis header grew into three distinct layers that build on each other.

* `wauvio.hpp` - the actual audio engine. Oscillators, FM synthesis, filters, envelopes, reverb/chorus/delay/distortion/EQ, mixing, WAV export, playback. This is where all the samples are generated.
* `wauvio_ext/` - a higher-level layer on top of the existing engine. Instead of hand-wiring oscillators and envelopes, and manually writing instruments every time you want to produce a sound, you get 300+ pre-made instruments, ready to be instantiated and played, plus notes/chords/melodies/tracks/arrangements to compose with.
* `wauvio_midi/` - loads a standard MIDI binary and turns it into playable Wauvio audio on its own. Simply point to a `*.mid` file, and let it do the rest.

Each layer only ends up depending on the one below it. `wauvio.hpp` never gets touched by the other two (abstractions); if you're already using it directly, absolutely nothing would change for you.

## In this repository

```
wauvio.hpp
wauvio_ext.hpp
wauvio_midi.hpp
wauvio_ext/
wauvio_midi/
```
Yeah, that's it. Examples and demos will be accessible [here](https://github.com/artdoesstuff/Wauvio/tree/wauvio-test-branch/worthy) (old branch), but as it stands, only the framework itself will remain here. However, if the examples/demos are too overwhelming at a glance, you can start writing it yourself almost immediately (it's like 3 lines, see below).

## Requirements

C++17 (or newer), and that's literally it. I specifically ensured not to include any external dependencies, nothing to link, nothing to install. Everything you'd need is inside the headers.

## Usage

Clone this repository into its own folder
```bash
git clone https://github.com/artdoesstuff/Wauvio.git
```
then copy (or move) the files into your project, and then just include what you need:
```cpp
#include "wauvio.hpp"       // needed no matter what
#include "wauvio_ext.hpp"   // instruments / notes / tracks
#include "wauvio_midi.hpp"  // only if you're loading MIDI files
```

Compile with C++17 (or newer). There are no extra steps beyond compiling your own `*.cpp` file.

(Run `git pull` inside the cloned folder any time later to grab updates).

## Getting started
### The simplest thing possible
```cpp
#include "wauvio.hpp"
#include "wauvio_ext.hpp"

int main() {
    auto piano = wauvio::instruments::AcousticGrandPiano();
    auto note  = piano.play(60, 1.0); // middle C for a second

    wauvio::save_wav_stereo(note.audio, "note.wav");
}
```

### Putting a few instruments together
```cpp
#include "wauvio.hpp"
#include "wauvio_ext.hpp"

using namespace wauvio::audio;

int main() {
    auto piano  = wauvio::instruments::AcousticGrandPiano();
    auto violin = wauvio::instruments::SoloViolin();
    violin.reverb(0.5f, 0.2f);

    Arrangement song;
    song.add(piano,  Melody{ Note(60, 0.5), Note(64, 0.5), Note(67, 1.0) });
    song.add(violin, Melody{ Note(67, 1.0), Note(71, 1.0) }, 0.25);

    wauvio::save_wav_stereo(song.render(), "song.wav");
}
```

### Or simply give it a MIDI file
```cpp
#include "wauvio.hpp"
#include "wauvio_ext.hpp"
#include "wauvio_midi.hpp"

int main() {
    auto music = wauvio::track::load_midi("song.mid");
    wauvio::play(music);
}
```

This one is probably the best for most people. It parses the file, works out the tempo (including tempo changes mid-file), figures out which instrument each track/channel should be, and mixes them all together. You don't need to create an `Instrument` or `Track` for this by hand; it's genuinely just two lines.

# Wauvio Library Extension (`wauvio_ext`)

There are 300+ instrument names available (A bit over 300 actual classes, plus aliases for things that are the same under a different name: e.g. `Bongo`/`Bongos`, `Gong`/`TamTam`; and as a result, you aren't really stuck if you don't remember what name _we_ specifically picked). Rough grouping:

- **Keyboards**: grand/upright/felt/tack/prepared piano, Rhodes,
  Wurlitzer, Clavinet, harpsichord, celesta, Mellotron, a handful of
  organs, accordion and its relatives.
- **Strings**: solo and section violin/viola/cello/bass, Baroque violin
  and cello, viola da gamba, Hardanger fiddle, nyckelharpa, chamber and
  symphonic string sections.
- **Guitars & bass**: the usual acoustic/nylon/steel/electric family
  (clean, overdriven, distorted, muted, power chords, harmonics), plus
  mandolin, banjo, ukulele, harp, oud, bouzouki, balalaika, resonator
  guitar, and the full bass family.
- **Woodwinds & saxophones**: pretty much the whole concert range,
  piccolo through contrabass.
- **Brass**: piccolo trumpet through tuba and sousaphone, plus some
  historical stuff like natural horn, alphorn, serpent, ophicleide.
- **World / folk**: erhu, guzheng, koto, shamisen, sitar, duduk,
  shakuhachi, kalimba, handpan, uilleann pipes, hurdy-gurdy, and more.
- **Percussion**: tuned percussion (including a properly unstable-sounding
  Waterphone), full kit/orchestral percussion, hand percussion, and 10
  ready-made drum kits with GM-style note mappings.
- **Vocals**: solo voices by range, various choir sizes, vowel presets,
  whisper/breath.
- **Experimental**: musical saw, theremin, Ondes Martenot, tape-style
  textures, that kind of thing. Good for weird ambient stuff.
- **Synths**: the usual lead/pad/bass/pluck/bell/arp/drone patches, built
  from Wauvio's own oscillators, no samples involved.

They're all real, usable classes:
```cpp
auto flute      = wauvio::instruments::Flute();
auto waterphone = wauvio::instruments::Waterphone();
auto theremin   = wauvio::instruments::Theremin();
```

### Notes, chords, dynamics, and articulations

```cpp
Note n(60, 0.5, Dynamics::mf, Articulation::Staccato);
auto c_major = wauvio::audio::scale::major(60);
```

Dynamics go from `ppp` to `fff`. Articulations cover the basics: `Sustain`, `Staccato`, `Pizzicato`, `Tremolo`, `Spiccato`, `SulPonticello`, `ColLegno`, `Harmonic`, `PalmMute`, `Muted`, plus a few extras like `Whisper` and `Breath` for vocals; and each instrument only lists the ones that actually make sense for it. They change the actual, audible sound, instead of just the label.

Notes also carry `expression`, `pitch_bend_semitones`, and a `glide_from_midi`/`glide_time` pair for portamento. These mostly get set automatically when you load a MIDI file, but nothing is stopping you from setting them yourself.

### Samples

Every instrument makes sound out of the box using synthesised fallback; you don't need any sampled audio files to get started; however, if you do, you can drop them in later without changing how you call the instrument:
```cpp
wauvio::instruments::AcousticGrandPiano piano;
piano.load_sample_file("piano_C4.wav", 60, 48, 72);
```
Underneath that, there's a real sample engine — key zones, velocity layers, round robin, loop points, release samples, pitch shifting — that stuff.

### Effects

`reverb()`, `chorus()`, `delay()`, `eq()`, `distortion()` on any
instrument, plus reverb/limiting at the `Arrangement` (master bus) level.

# Wauvio MIDI (`wauvio_midi`)

The entire point of this layer (and its creation) is that you shouldn't have to manually map every track in a MIDI file to an instrument just to hear it played back.
`load_midi()`:

1. Parses the actual binary file: header/track chunks, variable-length quantities, running status, all of it. No external MIDI library; it's all in here.
2. Builds a tempo map. If the file changes tempo halfway through, that gets respected instead of averaged away.
3. Splits things up by track, channel, _and_ whatever GM program is active at the time. This matters more than it sounds like it should, because some MIDI exports switch GM program mid-track just to signal a change in articulation, and if you only look at channel, you'd miss that.
4. Picks an instrument for each part automatically, using a General MIDI program table that covers all 128 programs plus the standard percussion key map. If a track never sends a program change at all (happens), it'll attempt to guess from the track name before giving up and using piano as the default.
5. Applies velocity, sustain pedal, portamento, pitch bend, and expression/volume, so it's not just "play this note at this fixed volume".
6. Hands you back something you can render or play.

If you don't like how a particular automatic choice may sound, you may override it:
```cpp
wauvio::track::LoadOptions opts;
opts.resolver.override_program(40, [] {
    return std::make_shared<wauvio::instruments::HardangerFiddle>();
});
auto music = wauvio::track::load_midi("music.mid", opts);

music.set_instrument<wauvio::instruments::SoloViolin>(0);
```
Overrides work at the level of the specific program, a specific percussion note, a whole channel, or just swap out a part's instrument after the fact.

Bad input gets an error instead of crashing: missing file, broken header, truncated data, corrupt variable-length values, invalid running status; those things throw a `wauvio::midi::MidiParseError` with a message that tells you exactly what went wrong.

## Some extra notes

- Nothing lower in the stack knows or cares about anything higher up.
  `wauvio.hpp` has no idea `wauvio_ext` exists.
- Sampled and synthesised instruments are interchangeable — same
  interface, same `play()` call, so you can swap one for the other (or
  upgrade from synthesised to sampled) without rewriting anything.
- "Automatic" doesn't mean "no say in the matter." Every decision the MIDI
  loader makes on your behalf can be overridden.

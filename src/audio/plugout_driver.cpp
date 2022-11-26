#include "plugout_driver.h"
#include <iostream>

static const plugout_endian RequestedEndian = PLUGOUT_ENDIAN_AUTOSELECT;
static const long DefaultBufferSize = 512L;
static const long DefaultSampleRate = 44100L;
static const gbs_opus::plugout_type DefaultPlugoutType = gbs_opus::plugout_type::SDL;

namespace gbs_opus
{
    extern const struct output_plugin plugout_app;
}

extern const struct output_plugin plugout_midi;
extern const struct output_plugin plugout_alsa;
extern const struct output_plugin plugout_altmidi;
extern const struct output_plugin plugout_devdsp;
extern const struct output_plugin plugout_dsound;
extern const struct output_plugin plugout_iodumper;
extern const struct output_plugin plugout_nas;
extern const struct output_plugin plugout_pulse;
extern const struct output_plugin plugout_stdout;
extern const struct output_plugin plugout_vgm;
extern const struct output_plugin plugout_wav;

gbs_opus::plugout_driver::plugout_driver() :
    sound_open(), sound_skip(), sound_pause(), sound_io(),
    sound_step(), sound_write(), sound_close(),
    m_endian(), m_sample_rate(),
    m_buffer_size(), m_req_buffersize(DefaultBufferSize),
    m_req_samplerate(DefaultSampleRate),
    m_plugout_type(DefaultPlugoutType),
    m_plugout()
{
    // Set endian
    if (RequestedEndian == PLUGOUT_ENDIAN_AUTOSELECT)
        m_endian = PLUGOUT_ENDIAN_NATIVE;
    else
        m_endian = RequestedEndian;
}

bool gbs_opus::plugout_driver::load()
{
    const output_plugin *p;

    // Choose plugout
    switch(m_plugout_type)
    {
        case plugout_type::APP:
            p = &plugout_app;
            break;
#ifdef PLUGOUT_MIDI
        case plugout_type::MIDI:
            p = &plugout_midi;
            break;
#endif
#ifdef PLUGOUT_ALSA
        case plugout_type::ALSA:
            p = &plugout_alsa;
            break;
#endif
#ifdef PLUGOUT_ALTMIDI
        case plugout_type::ALTMIDI:
            p = &plugout_altmidi;
            break;
#endif
#ifdef PLUGOUT_DEVDSP
        case plugout_type::DEVDSP:
            p = &plugout_devdsp;
            break;
#endif
#ifdef PLUGOUT_DSOUND
        case plugout_type::DSOUND:
            p = &plugout_dsound;
            break;
#endif
#ifdef PLUGOUT_IODUMPER
        case plugout_type::IODUMPER:
            p = &plugout_iodumper;
            break;
#endif
#ifdef PLUGOUT_NAS
        case plugout_type::NAS:
            p = &plugout_nas;
            break;
#endif
#ifdef PLUGOUT_PULSE
        case plugout_type::PULSE:
            p = &plugout_pulse;
            break;
#endif
#ifdef PLUGOUT_STDOUT
        case plugout_type::STDOUT:
            p = &plugout_stdout;
            break;
#endif
#ifdef PLUGOUT_VGM
        case plugout_type::VGM:
            p = &plugout_vgm;
            break;
#endif
#ifdef PLUGOUT_WAV
        case plugout_type::WAV:
            p = &plugout_wav;
            break;
#endif
        default:
            std::cerr << "Error: plugout is either unsupported or of an unknown type\n";
            return false;
    }

    // Setup callbacks
    sound_open = p->open;
    sound_close = p->close;
    sound_skip = p->skip;
    sound_pause = p->pause;
    sound_io = p->io;
    sound_step = p->step;
    sound_write = p->write;

    m_plugout = *p; // get copy of plugout
    // Sanitize name and desc
    if (!m_plugout.name) m_plugout.name = "";
    if (!m_plugout.description) m_plugout.description = "";

    return true;
}

long gbs_opus::plugout_driver::call_open() {
    long result = 0;
    long bytes = m_req_buffersize * (sizeof(int16_t) * 2);
    long rate = m_req_samplerate; // TODO: not modified by plugout, take out if not necessary?

    if (sound_open)
    {
        result = sound_open(&m_endian, rate, &bytes);
        if (RequestedEndian != PLUGOUT_ENDIAN_AUTOSELECT && m_endian != RequestedEndian)
        {
            std::cerr << "Error: Unsupported endianness for output plugin \"" <<
                      name() << "\"\n";
            call_close();
            return -1L;
        }
        if (bytes != m_req_buffersize)
        {
            std::cerr << "Warning: requested sample buffer size does not match the actual size:\n"
                         "Requsted " << m_req_buffersize << ", but got " << bytes / (sizeof(int16_t) * 2) << '\n';
        }
    }

    m_buffer_size = bytes;
    m_sample_rate = rate;
    return result;
}






#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <dos.h>
#include <string.h>

#include "CONFIG.H"
#include "LINEAR.H"
#include "PTRAP.H"
#include "VMPU.H"
#include "PLATFORM.H"

#if 1
void* tsfimpl_malloc(size_t size)
{
    __dpmi_meminfo info;
    info.address = 0;
    info.size = (size + 4 + 4095) & ~4095;

    size += 4;
    if (__dpmi_allocate_linear_memory( &info, 1 ) == -1 )
        return NULL;

    (*(size_t*)NearPtr(info.address)) = size;
    return NearPtr((unsigned int)(((uint8_t*)info.address) + sizeof(size_t)));
}

void tsfimpl_free(void* ptr) {}

void* tsfimpl_realloc(void *ptr, size_t size)
{
    void* newptr;
    size_t oldsize;
    if (ptr == 0)
	return tsfimpl_malloc(size);

    oldsize = *((size_t*)(ptr) - 1);
    newptr = tsfimpl_malloc(size);

    if (!newptr) return NULL;

    if (size <= oldsize)
        memcpy(newptr, ptr, size);
    else
        memcpy(newptr, ptr, oldsize);

    return newptr;
}

#define TSF_MALLOC tsfimpl_malloc
#define TSF_FREE tsfimpl_free
#define TSF_REALLOC tsfimpl_realloc
#endif
#define TSF_IMPLEMENTATION
#include "TSF.H"

/* 0x330: data port
 * 0x331: read: status port
 *       write: command port
 * status port:
 * bit 7: 0=data ready to read
 * bit 6: 0=data ready to write (both command and data)
 * command port:
 *  ff: reset, then ACK (FE) from data port
 *  3f: set to UART mode
 */

static bool bReset = false;

static const int midi_lengths[8] = { 3, 3, 3, 3, 2, 2, 3, 1 };
static unsigned char midi_buffer[32];
static unsigned char midi_ptr = 0;
static unsigned char midi_status_byte = 0x80;
static unsigned char midi_mpu_status = 0x80;

extern tsf* tsfrenderer;

static void VMPU_Write(uint16_t port, uint8_t value)
////////////////////////////////////////////////////
{
        dbgprintf(("VMPU_Write(%X)=%X\n", port, value ));
        if ( port == 0x331 ) {
        	if ( value == 0x3f ) {
                	midi_mpu_status &= ~0x80;
                }
                if ( value == 0xff ) {
                        bReset = true;
                        midi_ptr = 0;
                        midi_mpu_status &= ~0x80;
                }
        } else {
                if (!bReset) {
                        {
				if (midi_status_byte == 0xF0 && value != 0xF7)
					return;
				if (midi_status_byte == 0xF0 && value == 0xF7) {
					midi_status_byte = 0x80;
					midi_buffer[0] = 0x80;
                                        midi_ptr = 0;
					return;
				}
                                if ((value & 0xF0) < 0x80 && midi_ptr == 0) {
                                        midi_buffer[0] = midi_status_byte;
                                        midi_ptr = 1;
				}
				
                                midi_buffer[midi_ptr++] = value;
                                midi_status_byte = midi_buffer[0];

				if (midi_buffer[0] == 0xF0)
					return;

                                if (midi_ptr >= midi_lengths[(midi_buffer[0] >> 4) - 0x8]) {
                                        midi_ptr = 0;
                                        if (tsfrenderer) {
                                                //asm("cli");
                                                switch (midi_status_byte & 0xF0)
                                                {
                                                        case 0x80:
                                                                tsf_channel_note_off(tsfrenderer, midi_buffer[0] & 0xf, midi_buffer[1]);
                                                                break;
                                                        case 0x90:
                                                                tsf_channel_note_on(tsfrenderer, midi_buffer[0] & 0xf, midi_buffer[1], midi_buffer[2] / 127.0f);
                                                                break;
                                                        case 0xE0:
                                                                tsf_channel_set_pitchwheel(tsfrenderer, midi_buffer[0] & 0xf, (midi_buffer[1] & 0x7f) | ((midi_buffer[2] & 0x7f) << 7));
                                                                break;
                                                        case 0xC0:
                                                                tsf_channel_set_presetnumber(tsfrenderer, midi_buffer[0] & 0xf, midi_buffer[1], (midi_buffer[0] & 0xf) == 0x9);
                                                                break;
                                                        case 0xB0:
                                                                tsf_channel_midi_control(tsfrenderer, midi_buffer[0] & 0xf, midi_buffer[1], midi_buffer[2]);
                                                                break;
                                                        case 0xF0:
                                                                if (midi_status_byte == 0xFF) {
                                                                	int channel = 0;
                                                                        for (channel = 0; channel < 16; channel++) {
                                                                        	tsf_channel_midi_control(tsfrenderer, channel, 120, 0);
                                                                                tsf_channel_midi_control(tsfrenderer, channel, 121, 0);
                                                                        }
                                                                }
                                                                break;
                                                }
                                                //asm("sti");
                                        }
                                }
                        }
                }
        }
    return;
}

static uint8_t VMPU_Read(uint16_t port)
///////////////////////////////////////
{
        dbgprintf(("VMPU_Read(%X)\n", port ));
        if ( port == 0x330 ) {
                midi_mpu_status |= 0x80;
                if ( bReset ) {
                        bReset = false;
                        return 0xfe;
                }
                return 0xfe; // Always return Active Sensing.
        } else {
                return midi_mpu_status;
        }
}

/* SB-MIDI data written with DSP cmd 0x38 */

void VMPU_SBMidi_RawWrite( uint8_t value )
//////////////////////////////////////////
{
}

uint32_t VMPU_MPU(uint32_t port, uint32_t val, uint32_t out)
////////////////////////////////////////////////////////////
{
    return out ? (VMPU_Write(port, val), val) : ( val &= ~0xff, val |= VMPU_Read(port) );
}

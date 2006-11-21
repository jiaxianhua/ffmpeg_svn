/*
 * Creative Voice File muxer.
 * Copyright (c) 2006  Aurelien Jacobs <aurel@gnuage.org>
 *
 * This file is part of FFmpeg.
 *
 * FFmpeg is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * FFmpeg is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with FFmpeg; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "avformat.h"
#include "riff.h"    /* for CodecTag */
#include "voc.h"


static const unsigned char voc_magic[] = "Creative Voice File\x1A";

static const CodecTag voc_codec_tags[] = {
    {CODEC_ID_PCM_U8,        0x00},
    {CODEC_ID_ADPCM_SBPRO_4, 0x01},
    {CODEC_ID_ADPCM_SBPRO_3, 0x02},
    {CODEC_ID_ADPCM_SBPRO_2, 0x03},
    {CODEC_ID_PCM_S16LE,     0x04},
    {CODEC_ID_PCM_ALAW,      0x06},
    {CODEC_ID_PCM_MULAW,     0x07},
    {CODEC_ID_ADPCM_CT,    0x0200},
    {0, 0},
};


typedef struct voc_enc_context {
    int param_written;
} voc_enc_context_t;

static int voc_write_header(AVFormatContext *s)
{
    ByteIOContext *pb = &s->pb;
    const int header_size = 26;
    const int version = 0x0114;

    if (s->nb_streams != 1
        || s->streams[0]->codec->codec_type != CODEC_TYPE_AUDIO)
        return AVERROR_NOTSUPP;

    put_buffer(pb, voc_magic, sizeof(voc_magic) - 1);
    put_le16(pb, header_size);
    put_le16(pb, version);
    put_le16(pb, ~version + 0x1234);

    return 0;
}

static int voc_write_packet(AVFormatContext *s, AVPacket *pkt)
{
    voc_enc_context_t *voc = s->priv_data;
    AVCodecContext *enc = s->streams[0]->codec;
    ByteIOContext *pb = &s->pb;

    if (!voc->param_written) {
        int format = codec_get_tag(voc_codec_tags, enc->codec_id);

        if (format > 0xFF) {
            put_byte(pb, VOC_TYPE_NEW_VOICE_DATA);
            put_le24(pb, pkt->size + 12);
            put_le32(pb, enc->sample_rate);
            put_byte(pb, enc->bits_per_sample);
            put_byte(pb, enc->channels);
            put_le16(pb, format);
            put_le32(pb, 0);
        } else {
            if (s->streams[0]->codec->channels > 1) {
                put_byte(pb, VOC_TYPE_EXTENDED);
                put_le24(pb, 4);
                put_le16(pb, 65536-256000000/(enc->sample_rate*enc->channels));
                put_byte(pb, format);
                put_byte(pb, enc->channels - 1);
            }
            put_byte(pb, VOC_TYPE_VOICE_DATA);
            put_le24(pb, pkt->size + 2);
            put_byte(pb, 256 - 1000000 / enc->sample_rate);
            put_byte(pb, format);
        }
        voc->param_written = 1;
    } else {
        put_byte(pb, VOC_TYPE_VOICE_DATA_CONT);
        put_le24(pb, pkt->size);
    }

    put_buffer(pb, pkt->data, pkt->size);
    return 0;
}

static int voc_write_trailer(AVFormatContext *s)
{
    put_byte(&s->pb, 0);
    return 0;
}

AVOutputFormat voc_muxer = {
    "voc",
    "Creative Voice File format",
    "audio/x-voc",
    "voc",
    sizeof(voc_enc_context_t),
    CODEC_ID_PCM_U8,
    CODEC_ID_NONE,
    voc_write_header,
    voc_write_packet,
    voc_write_trailer,
};

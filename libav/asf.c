/*
 * ASF compatible encoder and decoder.
 * Copyright (c) 2000, 2001 Fabrice Bellard.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#include "avformat.h"
#include "avi.h"
#include "mpegaudio.h"

#define PACKET_SIZE 3200
#define PACKET_HEADER_SIZE 12
#define FRAME_HEADER_SIZE 17

typedef struct {
    int num;
    int seq;
    /* use for reading */
    AVPacket pkt;
    int frag_offset;
    int timestamp;
    INT64 duration;

    int ds_span;		/* descrambling  */
    int ds_packet_size;
    int ds_chunk_size;
    int ds_data_size;
    int ds_silence_data;

} ASFStream;

typedef struct {
    UINT32 v1;
    UINT16 v2;
    UINT16 v3;
    UINT8 v4[8];
} GUID;

typedef struct __attribute__((packed)) {
    GUID guid;			// generated by client computer
    uint64_t file_size;		// in bytes
                                // invalid if broadcasting
    uint64_t create_time;	// time of creation, in 100-nanosecond units since 1.1.1601
                                // invalid if broadcasting
    uint64_t packets_count;	// how many packets are there in the file
                                // invalid if broadcasting
    uint64_t play_time;		// play time, in 100-nanosecond units
                                // invalid if broadcasting
    uint64_t send_time;		// time to send file, in 100-nanosecond units
                                // invalid if broadcasting (could be ignored)
    uint32_t preroll;		// timestamp of the first packet, in milliseconds
    				// if nonzero - substract from time
    uint32_t ignore;            // preroll is 64bit - but let's just ignore it
    uint32_t flags;		// 0x01 - broadcast
    				// 0x02 - seekable
                                // rest is reserved should be 0
    uint32_t min_pktsize;	// size of a data packet
                                // invalid if broadcasting
    uint32_t max_pktsize;	// shall be the same as for min_pktsize
                                // invalid if broadcasting
    uint32_t max_bitrate;	// bandwith of stream in bps
    				// should be the sum of bitrates of the
                                // individual media streams
} ASFMainHeader;


typedef struct {
    int seqno;
    int packet_size;
    int is_streamed;
    int asfid2avid[128];        /* conversion table from asf ID 2 AVStream ID */
    ASFStream streams[128];	/* it's max number and it's not that big */
    /* non streamed additonnal info */
    INT64 nb_packets;
    INT64 duration; /* in 100ns units */
    /* packet filling */
    int packet_size_left;
    int packet_timestamp_start;
    int packet_timestamp_end;
    int packet_nb_frames;
    UINT8 packet_buf[PACKET_SIZE];
    ByteIOContext pb;
    /* only for reading */
    uint64_t data_offset; /* begining of the first data packet */

    ASFMainHeader hdr;

    int packet_flags;
    int packet_property;
    int packet_timestamp;
    int packet_segsizetype;
    int packet_segments;
    int packet_seq;
    int packet_replic_size;
    int packet_key_frame;
    int packet_padsize;
    int packet_frag_offset;
    int packet_frag_size;
    int packet_frag_timestamp;
    int packet_multi_size;
    int packet_obj_size;
    int packet_time_delta;
    int packet_time_start;

    int stream_index;
    ASFStream* asf_st; /* currently decoded stream */
} ASFContext;

static const GUID asf_header = {
    0x75B22630, 0x668E, 0x11CF, { 0xA6, 0xD9, 0x00, 0xAA, 0x00, 0x62, 0xCE, 0x6C },
};

static const GUID file_header = {
    0x8CABDCA1, 0xA947, 0x11CF, { 0x8E, 0xE4, 0x00, 0xC0, 0x0C, 0x20, 0x53, 0x65 },
};

static const GUID stream_header = {
    0xB7DC0791, 0xA9B7, 0x11CF, { 0x8E, 0xE6, 0x00, 0xC0, 0x0C, 0x20, 0x53, 0x65 },
};

static const GUID audio_stream = {
    0xF8699E40, 0x5B4D, 0x11CF, { 0xA8, 0xFD, 0x00, 0x80, 0x5F, 0x5C, 0x44, 0x2B },
};

static const GUID audio_conceal_none = {
    // 0x49f1a440, 0x4ece, 0x11d0, { 0xa3, 0xac, 0x00, 0xa0, 0xc9, 0x03, 0x48, 0xf6 },
    // New value lifted from avifile
    0x20fb5700, 0x5b55, 0x11cf, { 0xa8, 0xfd, 0x00, 0x80, 0x5f, 0x5c, 0x44, 0x2b },
};

static const GUID video_stream = {
    0xBC19EFC0, 0x5B4D, 0x11CF, { 0xA8, 0xFD, 0x00, 0x80, 0x5F, 0x5C, 0x44, 0x2B },
};

static const GUID video_conceal_none = {
    0x20FB5700, 0x5B55, 0x11CF, { 0xA8, 0xFD, 0x00, 0x80, 0x5F, 0x5C, 0x44, 0x2B },
};


static const GUID comment_header = {
    0x75b22633, 0x668e, 0x11cf, { 0xa6, 0xd9, 0x00, 0xaa, 0x00, 0x62, 0xce, 0x6c },
};

static const GUID codec_comment_header = {
    0x86D15240, 0x311D, 0x11D0, { 0xA3, 0xA4, 0x00, 0xA0, 0xC9, 0x03, 0x48, 0xF6 },
};
static const GUID codec_comment1_header = {
    0x86d15241, 0x311d, 0x11d0, { 0xa3, 0xa4, 0x00, 0xa0, 0xc9, 0x03, 0x48, 0xf6 },
};

static const GUID data_header = {
    0x75b22636, 0x668e, 0x11cf, { 0xa6, 0xd9, 0x00, 0xaa, 0x00, 0x62, 0xce, 0x6c },
};

static const GUID index_guid = {
    0x33000890, 0xe5b1, 0x11cf, { 0x89, 0xf4, 0x00, 0xa0, 0xc9, 0x03, 0x49, 0xcb },
};

static const GUID head1_guid = {
    0x5fbf03b5, 0xa92e, 0x11cf, { 0x8e, 0xe3, 0x00, 0xc0, 0x0c, 0x20, 0x53, 0x65 },
};

static const GUID head2_guid = {
    0xabd3d211, 0xa9ba, 0x11cf, { 0x8e, 0xe6, 0x00, 0xc0, 0x0c, 0x20, 0x53, 0x65 },
};

/* I am not a number !!! This GUID is the one found on the PC used to
   generate the stream */
static const GUID my_guid = {
    0, 0, 0, { 0, 0, 0, 0, 0, 0, 0, 0 },
};

static void put_guid(ByteIOContext *s, const GUID *g)
{
    int i;

    put_le32(s, g->v1);
    put_le16(s, g->v2);
    put_le16(s, g->v3);
    for(i=0;i<8;i++)
        put_byte(s, g->v4[i]);
}

static void put_str16(ByteIOContext *s, const char *tag)
{
    int c;

    put_le16(s,strlen(tag) + 1);
    for(;;) {
        c = (UINT8)*tag++;
        put_le16(s, c);
        if (c == '\0')
            break;
    }
}

static void put_str16_nolen(ByteIOContext *s, const char *tag)
{
    int c;

    for(;;) {
        c = (UINT8)*tag++;
        put_le16(s, c);
        if (c == '\0')
            break;
    }
}

static INT64 put_header(ByteIOContext *pb, const GUID *g)
{
    INT64 pos;

    pos = url_ftell(pb);
    put_guid(pb, g);
    put_le64(pb, 24);
    return pos;
}

/* update header size */
static void end_header(ByteIOContext *pb, INT64 pos)
{
    INT64 pos1;

    pos1 = url_ftell(pb);
    url_fseek(pb, pos + 16, SEEK_SET);
    put_le64(pb, pos1 - pos);
    url_fseek(pb, pos1, SEEK_SET);
}

/* write an asf chunk (only used in streaming case) */
static void put_chunk(AVFormatContext *s, int type, int payload_length, int flags)
{
    ASFContext *asf = s->priv_data;
    ByteIOContext *pb = &s->pb;
    int length;

    length = payload_length + 8;
    put_le16(pb, type);
    put_le16(pb, length);
    put_le32(pb, asf->seqno);
    put_le16(pb, flags); /* unknown bytes */
    put_le16(pb, length);
    asf->seqno++;
}

/* convert from unix to windows time */
static INT64 unix_to_file_time(int ti)
{
    INT64 t;

    t = ti * INT64_C(10000000);
    t += INT64_C(116444736000000000);
    return t;
}

/* write the header (used two times if non streamed) */
static int asf_write_header1(AVFormatContext *s, INT64 file_size, INT64 data_chunk_size)
{
    ASFContext *asf = s->priv_data;
    ByteIOContext *pb = &s->pb;
    int header_size, n, extra_size, extra_size2, wav_extra_size, file_time;
    int has_title;
    AVCodecContext *enc;
    INT64 header_offset, cur_pos, hpos;
    int bit_rate;

    has_title = (s->title[0] || s->author[0] || s->copyright[0] || s->comment[0]);

    bit_rate = 0;
    for(n=0;n<s->nb_streams;n++) {
        enc = &s->streams[n]->codec;

        bit_rate += enc->bit_rate;
    }

    if (asf->is_streamed) {
        put_chunk(s, 0x4824, 0, 0xc00); /* start of stream (length will be patched later) */
    }

    put_guid(pb, &asf_header);
    put_le64(pb, -1); /* header length, will be patched after */
    put_le32(pb, 3 + has_title + s->nb_streams); /* number of chunks in header */
    put_byte(pb, 1); /* ??? */
    put_byte(pb, 2); /* ??? */

    /* file header */
    header_offset = url_ftell(pb);
    hpos = put_header(pb, &file_header);
    put_guid(pb, &my_guid);
    put_le64(pb, file_size);
    file_time = 0;
    put_le64(pb, unix_to_file_time(file_time));
    put_le64(pb, asf->nb_packets); /* number of packets */
    put_le64(pb, asf->duration); /* end time stamp (in 100ns units) */
    put_le64(pb, asf->duration); /* duration (in 100ns units) */
    put_le32(pb, 0); /* start time stamp */
    put_le32(pb, 0); /* ??? */
    put_le32(pb, asf->is_streamed ? 1 : 0); /* ??? */
    put_le32(pb, asf->packet_size); /* packet size */
    put_le32(pb, asf->packet_size); /* packet size */
    put_le32(pb, bit_rate); /* Nominal data rate in bps */
    end_header(pb, hpos);

    /* unknown headers */
    hpos = put_header(pb, &head1_guid);
    put_guid(pb, &head2_guid);
    put_le32(pb, 6);
    put_le16(pb, 0);
    end_header(pb, hpos);

    /* title and other infos */
    if (has_title) {
        hpos = put_header(pb, &comment_header);
        put_le16(pb, 2 * (strlen(s->title) + 1));
        put_le16(pb, 2 * (strlen(s->author) + 1));
        put_le16(pb, 2 * (strlen(s->copyright) + 1));
        put_le16(pb, 2 * (strlen(s->comment) + 1));
        put_le16(pb, 0);
        put_str16_nolen(pb, s->title);
        put_str16_nolen(pb, s->author);
        put_str16_nolen(pb, s->copyright);
        put_str16_nolen(pb, s->comment);
        end_header(pb, hpos);
    }

    /* stream headers */
    for(n=0;n<s->nb_streams;n++) {
        INT64 es_pos;
        //        ASFStream *stream = &asf->streams[n];

        enc = &s->streams[n]->codec;
        asf->streams[n].num = n + 1;
        asf->streams[n].seq = 0;

        switch(enc->codec_type) {
        case CODEC_TYPE_AUDIO:
            wav_extra_size = 0;
            extra_size = 18 + wav_extra_size;
            extra_size2 = 0;
            break;
        default:
        case CODEC_TYPE_VIDEO:
            wav_extra_size = 0;
            extra_size = 0x33;
            extra_size2 = 0;
            break;
        }

        hpos = put_header(pb, &stream_header);
        if (enc->codec_type == CODEC_TYPE_AUDIO) {
            put_guid(pb, &audio_stream);
            put_guid(pb, &audio_conceal_none);
        } else {
            put_guid(pb, &video_stream);
            put_guid(pb, &video_conceal_none);
        }
        put_le64(pb, 0); /* ??? */
        es_pos = url_ftell(pb);
        put_le32(pb, extra_size); /* wav header len */
        put_le32(pb, extra_size2); /* additional data len */
        put_le16(pb, n + 1); /* stream number */
        put_le32(pb, 0); /* ??? */

        if (enc->codec_type == CODEC_TYPE_AUDIO) {
            /* WAVEFORMATEX header */
            int wavsize = put_wav_header(pb, enc);

            if (wavsize < 0)
                return -1;
            if (wavsize != extra_size) {
                cur_pos = url_ftell(pb);
                url_fseek(pb, es_pos, SEEK_SET);
                put_le32(pb, wavsize); /* wav header len */
                url_fseek(pb, cur_pos, SEEK_SET);
            }
        } else {
            put_le32(pb, enc->width);
            put_le32(pb, enc->height);
            put_byte(pb, 2); /* ??? */
            put_le16(pb, 40); /* size */

            /* BITMAPINFOHEADER header */
            put_bmp_header(pb, enc, codec_bmp_tags, 1);
        }
        end_header(pb, hpos);
    }

    /* media comments */

    hpos = put_header(pb, &codec_comment_header);
    put_guid(pb, &codec_comment1_header);
    put_le32(pb, s->nb_streams);
    for(n=0;n<s->nb_streams;n++) {
        AVCodec *p;

        enc = &s->streams[n]->codec;
        p = avcodec_find_encoder(enc->codec_id);

        put_le16(pb, asf->streams[n].num);
        put_str16(pb, p ? p->name : enc->codec_name);
        put_le16(pb, 0); /* no parameters */
        /* id */
        if (enc->codec_type == CODEC_TYPE_AUDIO) {
            put_le16(pb, 2);
            put_le16(pb, codec_get_tag(codec_wav_tags, enc->codec_id));
        } else {
            put_le16(pb, 4);
            put_le32(pb, codec_get_tag(codec_bmp_tags, enc->codec_id));
        }
    }
    end_header(pb, hpos);

    /* patch the header size fields */

    cur_pos = url_ftell(pb);
    header_size = cur_pos - header_offset;
    if (asf->is_streamed) {
        header_size += 8 + 30 + 50;

        url_fseek(pb, header_offset - 10 - 30, SEEK_SET);
        put_le16(pb, header_size);
        url_fseek(pb, header_offset - 2 - 30, SEEK_SET);
        put_le16(pb, header_size);

        header_size -= 8 + 30 + 50;
    }
    header_size += 24 + 6;
    url_fseek(pb, header_offset - 14, SEEK_SET);
    put_le64(pb, header_size);
    url_fseek(pb, cur_pos, SEEK_SET);

    /* movie chunk, followed by packets of packet_size */
    asf->data_offset = cur_pos;
    put_guid(pb, &data_header);
    put_le64(pb, data_chunk_size);
    put_guid(pb, &my_guid);
    put_le64(pb, asf->nb_packets); /* nb packets */
    put_byte(pb, 1); /* ??? */
    put_byte(pb, 1); /* ??? */
    return 0;
}

static int asf_write_header(AVFormatContext *s)
{
    ASFContext *asf = s->priv_data;

    av_set_pts_info(s, 32, 1, 1000); /* 32 bit pts in ms */

    asf->packet_size = PACKET_SIZE;
    asf->nb_packets = 0;

    if (asf_write_header1(s, 0, 50) < 0) {
        //av_free(asf);
        return -1;
    }

    put_flush_packet(&s->pb);

    asf->packet_nb_frames = 0;
    asf->packet_timestamp_start = -1;
    asf->packet_timestamp_end = -1;
    asf->packet_size_left = asf->packet_size - PACKET_HEADER_SIZE;
    init_put_byte(&asf->pb, asf->packet_buf, asf->packet_size, 1,
                  NULL, NULL, NULL, NULL);

    return 0;
}

static int asf_write_stream_header(AVFormatContext *s)
{
    ASFContext *asf = s->priv_data;

    asf->is_streamed = 1;

    return asf_write_header(s);
}

/* write a fixed size packet */
static int put_packet(AVFormatContext *s,
                       unsigned int timestamp, unsigned int duration,
                       int nb_frames, int padsize)
{
    ASFContext *asf = s->priv_data;
    ByteIOContext *pb = &s->pb;
    int flags;

    if (asf->is_streamed) {
        put_chunk(s, 0x4424, asf->packet_size, 0);
    }

    put_byte(pb, 0x82);
    put_le16(pb, 0);

    flags = 0x01; /* nb segments present */
    if (padsize > 0) {
        if (padsize < 256)
            flags |= 0x08;
        else
            flags |= 0x10;
    }
    put_byte(pb, flags); /* flags */
    put_byte(pb, 0x5d);
    if (flags & 0x10)
        put_le16(pb, padsize - 2);
    if (flags & 0x08)
        put_byte(pb, padsize - 1);
    put_le32(pb, timestamp);
    put_le16(pb, duration);
    put_byte(pb, nb_frames | 0x80);

    return PACKET_HEADER_SIZE + ((flags & 0x18) >> 3);
}

static void flush_packet(AVFormatContext *s)
{
    ASFContext *asf = s->priv_data;
    int hdr_size, ptr;

    hdr_size = put_packet(s, asf->packet_timestamp_start,
               asf->packet_timestamp_end - asf->packet_timestamp_start,
               asf->packet_nb_frames, asf->packet_size_left);

    /* Clear out the padding bytes */
    ptr = asf->packet_size - hdr_size - asf->packet_size_left;
    memset(asf->packet_buf + ptr, 0, asf->packet_size_left);

    put_buffer(&s->pb, asf->packet_buf, asf->packet_size - hdr_size);

    put_flush_packet(&s->pb);
    asf->nb_packets++;
    asf->packet_nb_frames = 0;
    asf->packet_timestamp_start = -1;
    asf->packet_timestamp_end = -1;
    asf->packet_size_left = asf->packet_size - PACKET_HEADER_SIZE;
    init_put_byte(&asf->pb, asf->packet_buf, asf->packet_size, 1,
                  NULL, NULL, NULL, NULL);
}

static void put_frame_header(AVFormatContext *s, ASFStream *stream, int timestamp,
                             int payload_size, int frag_offset, int frag_len)
{
    ASFContext *asf = s->priv_data;
    ByteIOContext *pb = &asf->pb;
    int val;

    val = stream->num;
    if (s->streams[val - 1]->codec.key_frame /* && frag_offset == 0 */)
        val |= 0x80;
    put_byte(pb, val);
    put_byte(pb, stream->seq);
    put_le32(pb, frag_offset); /* fragment offset */
    put_byte(pb, 0x08); /* flags */
    put_le32(pb, payload_size);
    put_le32(pb, timestamp);
    put_le16(pb, frag_len);
}


/* Output a frame. We suppose that payload_size <= PACKET_SIZE.

   It is there that you understand that the ASF format is really
   crap. They have misread the MPEG Systems spec !
 */
static void put_frame(AVFormatContext *s, ASFStream *stream, int timestamp,
                      UINT8 *buf, int payload_size)
{
    ASFContext *asf = s->priv_data;
    int frag_pos, frag_len, frag_len1;

    frag_pos = 0;
    while (frag_pos < payload_size) {
        frag_len = payload_size - frag_pos;
        frag_len1 = asf->packet_size_left - FRAME_HEADER_SIZE;
        if (frag_len1 > 0) {
            if (frag_len > frag_len1)
                frag_len = frag_len1;
            put_frame_header(s, stream, timestamp, payload_size, frag_pos, frag_len);
            put_buffer(&asf->pb, buf, frag_len);
            asf->packet_size_left -= (frag_len + FRAME_HEADER_SIZE);
            asf->packet_timestamp_end = timestamp;
            if (asf->packet_timestamp_start == -1)
                asf->packet_timestamp_start = timestamp;
            asf->packet_nb_frames++;
        } else {
            frag_len = 0;
        }
        frag_pos += frag_len;
        buf += frag_len;
        /* output the frame if filled */
        if (asf->packet_size_left <= FRAME_HEADER_SIZE)
            flush_packet(s);
    }
    stream->seq++;
}


static int asf_write_packet(AVFormatContext *s, int stream_index,
                            UINT8 *buf, int size, int timestamp)
{
    ASFContext *asf = s->priv_data;
    ASFStream *stream;
    INT64 duration;
    AVCodecContext *codec;

    codec = &s->streams[stream_index]->codec;
    stream = &asf->streams[stream_index];

    if (codec->codec_type == CODEC_TYPE_AUDIO) {
        duration = (codec->frame_number * codec->frame_size * INT64_C(10000000)) /
            codec->sample_rate;
    } else {
        duration = codec->frame_number *
            ((INT64_C(10000000) * FRAME_RATE_BASE) / codec->frame_rate);
    }
    if (duration > asf->duration)
        asf->duration = duration;

    put_frame(s, stream, timestamp, buf, size);
    return 0;
}

static int asf_write_trailer(AVFormatContext *s)
{
    ASFContext *asf = s->priv_data;
    INT64 file_size;

    /* flush the current packet */
    if (asf->pb.buf_ptr > asf->pb.buffer)
        flush_packet(s);

    if (asf->is_streamed) {
        put_chunk(s, 0x4524, 0, 0); /* end of stream */
    } else {
        /* rewrite an updated header */
        file_size = url_ftell(&s->pb);
        url_fseek(&s->pb, 0, SEEK_SET);
        asf_write_header1(s, file_size, file_size - asf->data_offset);
    }

    put_flush_packet(&s->pb);
    return 0;
}

/**********************************/
/* decoding */

//#define DEBUG

#ifdef DEBUG
static void print_guid(const GUID *g)
{
    int i;
    printf("0x%08x, 0x%04x, 0x%04x, {", g->v1, g->v2, g->v3);
    for(i=0;i<8;i++)
        printf(" 0x%02x,", g->v4[i]);
    printf("}\n");
}
#endif

static void get_guid(ByteIOContext *s, GUID *g)
{
    int i;

    g->v1 = get_le32(s);
    g->v2 = get_le16(s);
    g->v3 = get_le16(s);
    for(i=0;i<8;i++)
        g->v4[i] = get_byte(s);
}

#if 0
static void get_str16(ByteIOContext *pb, char *buf, int buf_size)
{
    int len, c;
    char *q;

    len = get_le16(pb);
    q = buf;
    while (len > 0) {
        c = get_le16(pb);
        if ((q - buf) < buf_size - 1)
            *q++ = c;
        len--;
    }
    *q = '\0';
}
#endif

static void get_str16_nolen(ByteIOContext *pb, int len, char *buf, int buf_size)
{
    int c;
    char *q;

    q = buf;
    while (len > 0) {
        c = get_le16(pb);
        if ((q - buf) < buf_size - 1)
            *q++ = c;
        len-=2;
    }
    *q = '\0';
}

static int asf_probe(AVProbeData *pd)
{
    GUID g;
    const unsigned char *p;
    int i;

    /* check file header */
    if (pd->buf_size <= 32)
        return 0;
    p = pd->buf;
    g.v1 = p[0] | (p[1] << 8) | (p[2] << 16) | (p[3] << 24);
    p += 4;
    g.v2 = p[0] | (p[1] << 8);
    p += 2;
    g.v3 = p[0] | (p[1] << 8);
    p += 2;
    for(i=0;i<8;i++)
        g.v4[i] = *p++;

    if (!memcmp(&g, &asf_header, sizeof(GUID)))
        return AVPROBE_SCORE_MAX;
    else
        return 0;
}

static int asf_read_header(AVFormatContext *s, AVFormatParameters *ap)
{
    ASFContext *asf = s->priv_data;
    GUID g;
    ByteIOContext *pb = &s->pb;
    AVStream *st;
    ASFStream *asf_st;
    int size, i;
    INT64 gsize;

    av_set_pts_info(s, 32, 1, 1000); /* 32 bit pts in ms */

    get_guid(pb, &g);
    if (memcmp(&g, &asf_header, sizeof(GUID)))
        goto fail;
    get_le64(pb);
    get_le32(pb);
    get_byte(pb);
    get_byte(pb);
    memset(&asf->asfid2avid, -1, sizeof(asf->asfid2avid));
    for(;;) {
        get_guid(pb, &g);
        gsize = get_le64(pb);
#ifdef DEBUG
        printf("%08Lx: ", url_ftell(pb) - 24);
        print_guid(&g);
        printf("  size=0x%Lx\n", gsize);
#endif
        if (gsize < 24)
            goto fail;
        if (!memcmp(&g, &file_header, sizeof(GUID))) {
            get_guid(pb, &asf->hdr.guid);
	    asf->hdr.file_size		= get_le64(pb);
	    asf->hdr.create_time	= get_le64(pb);
	    asf->hdr.packets_count	= get_le64(pb);
	    asf->hdr.play_time		= get_le64(pb);
	    asf->hdr.send_time		= get_le64(pb);
	    asf->hdr.preroll		= get_le32(pb);
	    asf->hdr.ignore		= get_le32(pb);
	    asf->hdr.flags		= get_le32(pb);
	    asf->hdr.min_pktsize	= get_le32(pb);
	    asf->hdr.max_pktsize	= get_le32(pb);
	    asf->hdr.max_bitrate	= get_le32(pb);
	    asf->packet_size = asf->hdr.max_pktsize;
            asf->nb_packets = asf->hdr.packets_count;
        } else if (!memcmp(&g, &stream_header, sizeof(GUID))) {
            int type, total_size;
            unsigned int tag1;
            INT64 pos1, pos2;

            pos1 = url_ftell(pb);

            st = av_mallocz(sizeof(AVStream));
            if (!st)
                goto fail;
            s->streams[s->nb_streams] = st;
            asf_st = av_mallocz(sizeof(ASFStream));
            if (!asf_st)
                goto fail;
            st->priv_data = asf_st;
	    st->time_length = (asf->hdr.send_time - asf->hdr.preroll) / 10; // us
            get_guid(pb, &g);
            if (!memcmp(&g, &audio_stream, sizeof(GUID))) {
                type = CODEC_TYPE_AUDIO;
            } else if (!memcmp(&g, &video_stream, sizeof(GUID))) {
                type = CODEC_TYPE_VIDEO;
            } else {
                goto fail;
            }
            get_guid(pb, &g);
            total_size = get_le64(pb);
            get_le32(pb);
            get_le32(pb);
	    st->id = get_le16(pb) & 0x7f; /* stream id */
            // mapping of asf ID to AV stream ID;
            asf->asfid2avid[st->id] = s->nb_streams++;

            get_le32(pb);
	    st->codec.codec_type = type;
            st->codec.frame_rate = 1000000; // us
            if (type == CODEC_TYPE_AUDIO) {
                get_wav_header(pb, &st->codec, 1);
		/* We have to init the frame size at some point .... */
		pos2 = url_ftell(pb);
		if (gsize > (pos2 + 8 - pos1 + 24)) {
		    asf_st->ds_span = get_byte(pb);
		    asf_st->ds_packet_size = get_le16(pb);
		    asf_st->ds_chunk_size = get_le16(pb);
		    asf_st->ds_data_size = get_le16(pb);
		    asf_st->ds_silence_data = get_byte(pb);
		}
		//printf("Descrambling: ps:%d cs:%d ds:%d s:%d  sd:%d\n",
		//       asf_st->ds_packet_size, asf_st->ds_chunk_size,
		//       asf_st->ds_data_size, asf_st->ds_span, asf_st->ds_silence_data);
		if (asf_st->ds_span > 1) {
		    if (!asf_st->ds_chunk_size
			|| (asf_st->ds_packet_size/asf_st->ds_chunk_size <= 1))
			asf_st->ds_span = 0; // disable descrambling
		}
                switch (st->codec.codec_id) {
                case CODEC_ID_MP3LAME:
                    st->codec.frame_size = MPA_FRAME_SIZE;
                    break;
                case CODEC_ID_PCM_S16LE:
                case CODEC_ID_PCM_S16BE:
                case CODEC_ID_PCM_U16LE:
                case CODEC_ID_PCM_U16BE:
                case CODEC_ID_PCM_S8:
                case CODEC_ID_PCM_U8:
                case CODEC_ID_PCM_ALAW:
                case CODEC_ID_PCM_MULAW:
                    st->codec.frame_size = 1;
                    break;
                default:
                    /* This is probably wrong, but it prevents a crash later */
                    st->codec.frame_size = 1;
                    break;
                }
            } else {
		get_le32(pb);
                get_le32(pb);
                get_byte(pb);
                size = get_le16(pb); /* size */
                get_le32(pb); /* size */
                st->codec.width = get_le32(pb);
		st->codec.height = get_le32(pb);
                /* not available for asf */
                get_le16(pb); /* panes */
                get_le16(pb); /* depth */
                tag1 = get_le32(pb);
		url_fskip(pb, 20);
		if (size > 40) {
		    st->codec.extradata_size = size - 40;
		    st->codec.extradata = av_mallocz(st->codec.extradata_size);
		    get_buffer(pb, st->codec.extradata, st->codec.extradata_size);
		}
                st->codec.codec_tag = tag1;
		st->codec.codec_id = codec_get_id(codec_bmp_tags, tag1);
            }
            pos2 = url_ftell(pb);
            url_fskip(pb, gsize - (pos2 - pos1 + 24));
        } else if (!memcmp(&g, &data_header, sizeof(GUID))) {
            break;
        } else if (!memcmp(&g, &comment_header, sizeof(GUID))) {
            int len1, len2, len3, len4, len5;

            len1 = get_le16(pb);
            len2 = get_le16(pb);
            len3 = get_le16(pb);
            len4 = get_le16(pb);
            len5 = get_le16(pb);
            get_str16_nolen(pb, len1, s->title, sizeof(s->title));
            get_str16_nolen(pb, len2, s->author, sizeof(s->author));
            get_str16_nolen(pb, len3, s->copyright, sizeof(s->copyright));
            get_str16_nolen(pb, len4, s->comment, sizeof(s->comment));
	    url_fskip(pb, len5);
#if 0
        } else if (!memcmp(&g, &head1_guid, sizeof(GUID))) {
            int v1, v2;
            get_guid(pb, &g);
            v1 = get_le32(pb);
            v2 = get_le16(pb);
        } else if (!memcmp(&g, &codec_comment_header, sizeof(GUID))) {
            int len, v1, n, num;
            char str[256], *q;
            char tag[16];

            get_guid(pb, &g);
            print_guid(&g);

            n = get_le32(pb);
            for(i=0;i<n;i++) {
                num = get_le16(pb); /* stream number */
                get_str16(pb, str, sizeof(str));
                get_str16(pb, str, sizeof(str));
                len = get_le16(pb);
                q = tag;
                while (len > 0) {
                    v1 = get_byte(pb);
                    if ((q - tag) < sizeof(tag) - 1)
                        *q++ = v1;
                    len--;
                }
                *q = '\0';
            }
#endif
        } else if (url_feof(pb)) {
            goto fail;
        } else {
            url_fseek(pb, gsize - 24, SEEK_CUR);
        }
    }
    get_guid(pb, &g);
    get_le64(pb);
    get_byte(pb);
    get_byte(pb);
    if (url_feof(pb))
        goto fail;
    asf->data_offset = url_ftell(pb);
    asf->packet_size_left = 0;

    return 0;

 fail:
     for(i=0;i<s->nb_streams;i++) {
        AVStream *st = s->streams[i];
	if (st) {
	    av_free(st->priv_data);
            av_free(st->codec.extradata);
	}
        av_free(st);
    }
    return -1;
}

#define DO_2BITS(bits, var, defval) \
    switch (bits & 3) \
    { \
    case 3: var = get_le32(pb); rsize += 4; break; \
    case 2: var = get_le16(pb); rsize += 2; break; \
    case 1: var = get_byte(pb); rsize++; break; \
    default: var = defval; break; \
    }

static int asf_get_packet(AVFormatContext *s)
{
    ASFContext *asf = s->priv_data;
    ByteIOContext *pb = &s->pb;
    uint32_t packet_length, padsize;
    int rsize = 11;
    int c = get_byte(pb);
    if (c != 0x82) {
        if (!url_feof(pb))
	    printf("ff asf bad header %x  at:%Ld\n", c, url_ftell(pb));
	return -EIO;
    }
    if ((c & 0x0f) == 2) { // always true for now
	if (get_le16(pb) != 0) {
            if (!url_feof(pb))
		printf("ff asf bad non zero\n");
	    return -EIO;
	}
    }

    asf->packet_flags = get_byte(pb);
    asf->packet_property = get_byte(pb);

    DO_2BITS(asf->packet_flags >> 5, packet_length, asf->packet_size);
    DO_2BITS(asf->packet_flags >> 1, padsize, 0); // sequence ignored
    DO_2BITS(asf->packet_flags >> 3, padsize, 0); // padding length

    asf->packet_timestamp = get_le32(pb);
    get_le16(pb); /* duration */
    // rsize has at least 11 bytes which have to be present

    if (asf->packet_flags & 0x01) {
	asf->packet_segsizetype = get_byte(pb); rsize++;
        asf->packet_segments = asf->packet_segsizetype & 0x3f;
    } else {
	asf->packet_segments = 1;
        asf->packet_segsizetype = 0x80;
    }
    asf->packet_size_left = packet_length - padsize - rsize;
    asf->packet_padsize = padsize;
#ifdef DEBUG
    printf("packet: size=%d padsize=%d  left=%d\n", asf->packet_size, asf->packet_padsize, asf->packet_size_left);
#endif
    return 0;
}

static int asf_read_packet(AVFormatContext *s, AVPacket *pkt)
{
    ASFContext *asf = s->priv_data;
    ASFStream *asf_st = 0;
    ByteIOContext *pb = &s->pb;
    //static int pc = 0;
    for (;;) {
	int rsize = 0;
	if (asf->packet_size_left < FRAME_HEADER_SIZE
	    || asf->packet_segments < 1) {
	    //asf->packet_size_left <= asf->packet_padsize) {
	    int ret = asf->packet_size_left + asf->packet_padsize;
	    //printf("PacketLeftSize:%d  Pad:%d Pos:%Ld\n", asf->packet_size_left, asf->packet_padsize, url_ftell(pb));
	    /* fail safe */
	    url_fskip(pb, ret);
	    ret = asf_get_packet(s);
	    //printf("READ ASF PACKET  %d   r:%d   c:%d\n", ret, asf->packet_size_left, pc++);
	    if (ret < 0 || url_feof(pb))
		return -EIO;
            asf->packet_time_start = 0;
            continue;
	}
	if (asf->packet_time_start == 0) {
	    /* read frame header */
            int num = get_byte(pb);
	    asf->packet_segments--;
	    rsize++;
	    asf->packet_key_frame = (num & 0x80) >> 7;
	    asf->stream_index = asf->asfid2avid[num & 0x7f];
	    // sequence should be ignored!
	    DO_2BITS(asf->packet_property >> 4, asf->packet_seq, 0);
	    DO_2BITS(asf->packet_property >> 2, asf->packet_frag_offset, 0);
	    DO_2BITS(asf->packet_property, asf->packet_replic_size, 0);

	    if (asf->packet_replic_size > 1) {
                // it should be always at least 8 bytes - FIXME validate
		asf->packet_obj_size = get_le32(pb);
		asf->packet_frag_timestamp = get_le32(pb); // timestamp
		rsize += asf->packet_replic_size; // FIXME - check validity
	    } else {
		// multipacket - frag_offset is beginig timestamp
		asf->packet_time_start = asf->packet_frag_offset;
                asf->packet_frag_offset = 0;
		asf->packet_frag_timestamp = asf->packet_timestamp;

		if (asf->packet_replic_size == 1) {
		    asf->packet_time_delta = get_byte(pb);
		    rsize++;
		}
	    }
	    if (asf->packet_flags & 0x01) {
		DO_2BITS(asf->packet_segsizetype >> 6, asf->packet_frag_size, 0); // 0 is illegal
		//printf("Fragsize %d\n", asf->packet_frag_size);
	    } else {
		asf->packet_frag_size = asf->packet_size_left - rsize;
		//printf("Using rest  %d %d %d\n", asf->packet_frag_size, asf->packet_size_left, rsize);
	    }
	    if (asf->packet_replic_size == 1) {
		asf->packet_multi_size = asf->packet_frag_size;
		if (asf->packet_multi_size > asf->packet_size_left) {
		    asf->packet_segments = 0;
                    continue;
		}
	    }
#undef DO_2BITS
	    asf->packet_size_left -= rsize;
	    //printf("___objsize____  %d   %d    rs:%d\n", asf->packet_obj_size, asf->packet_frag_offset, rsize);

	    if (asf->stream_index < 0) {
                asf->packet_time_start = 0;
		/* unhandled packet (should not happen) */
		url_fskip(pb, asf->packet_frag_size);
		asf->packet_size_left -= asf->packet_frag_size;
		printf("ff asf skip %d  %d\n", asf->packet_frag_size, num & 0x7f);
                continue;
	    }
	    asf->asf_st = s->streams[asf->stream_index]->priv_data;
	}
	asf_st = asf->asf_st;

	if ((asf->packet_frag_offset != asf_st->frag_offset
	     || (asf->packet_frag_offset
		 && asf->packet_seq != asf_st->seq)) // seq should be ignored
	   ) {
	    /* cannot continue current packet: free it */
	    // FIXME better check if packet was already allocated
	    printf("ff asf parser skips: %d - %d     o:%d - %d    %d %d   fl:%d\n",
		   asf_st->pkt.size,
		   asf->packet_obj_size,
		   asf->packet_frag_offset, asf_st->frag_offset,
		   asf->packet_seq, asf_st->seq, asf->packet_frag_size);
	    if (asf_st->pkt.size)
		av_free_packet(&asf_st->pkt);
	    asf_st->frag_offset = 0;
	    if (asf->packet_frag_offset != 0) {
		url_fskip(pb, asf->packet_frag_size);
		printf("ff asf parser skiping %db\n", asf->packet_frag_size);
		asf->packet_size_left -= asf->packet_frag_size;
		continue;
	    }
	}
	if (asf->packet_replic_size == 1) {
	    // frag_offset is here used as the begining timestamp
	    asf->packet_frag_timestamp = asf->packet_time_start;
	    asf->packet_time_start += asf->packet_time_delta;
	    asf->packet_obj_size = asf->packet_frag_size = get_byte(pb);
	    asf->packet_size_left--;
            asf->packet_multi_size--;
	    if (asf->packet_multi_size < asf->packet_obj_size)
	    {
		asf->packet_time_start = 0;
		url_fskip(pb, asf->packet_multi_size);
		asf->packet_size_left -= asf->packet_multi_size;
                continue;
	    }
	    asf->packet_multi_size -= asf->packet_obj_size;
	    //printf("COMPRESS size  %d  %d  %d   ms:%d\n", asf->packet_obj_size, asf->packet_frag_timestamp, asf->packet_size_left, asf->packet_multi_size);
	}
	if (asf_st->frag_offset == 0) {
	    /* new packet */
	    av_new_packet(&asf_st->pkt, asf->packet_obj_size);
	    asf_st->seq = asf->packet_seq;
	    asf_st->pkt.pts = asf->packet_frag_timestamp - asf->hdr.preroll;
	    asf_st->pkt.stream_index = asf->stream_index;
	    if (asf->packet_key_frame)
		asf_st->pkt.flags |= PKT_FLAG_KEY;
	}

	/* read data */
	//printf("READ PACKET s:%d  os:%d  o:%d,%d  l:%d   DATA:%p\n",
	//       asf->packet_size, asf_st->pkt.size, asf->packet_frag_offset,
	//       asf_st->frag_offset, asf->packet_frag_size, asf_st->pkt.data);
	asf->packet_size_left -= asf->packet_frag_size;
	if (asf->packet_size_left < 0)
            continue;
	get_buffer(pb, asf_st->pkt.data + asf->packet_frag_offset,
		   asf->packet_frag_size);
	asf_st->frag_offset += asf->packet_frag_size;
	/* test if whole packet is read */
	if (asf_st->frag_offset == asf_st->pkt.size) {
	    /* return packet */
	    if (asf_st->ds_span > 1) {
		/* packet descrambling */
		char* newdata = av_malloc(asf_st->pkt.size);
		if (newdata) {
		    int offset = 0;
		    while (offset < asf_st->pkt.size) {
			int off = offset / asf_st->ds_chunk_size;
			int row = off / asf_st->ds_span;
			int col = off % asf_st->ds_span;
			int idx = row + col * asf_st->ds_packet_size / asf_st->ds_chunk_size;
			//printf("off:%d  row:%d  col:%d  idx:%d\n", off, row, col, idx);
			memcpy(newdata + offset,
			       asf_st->pkt.data + idx * asf_st->ds_chunk_size,
			       asf_st->ds_chunk_size);
			offset += asf_st->ds_chunk_size;
		    }
		    av_free(asf_st->pkt.data);
		    asf_st->pkt.data = newdata;
		}
	    }
	    asf_st->frag_offset = 0;
	    memcpy(pkt, &asf_st->pkt, sizeof(AVPacket));
	    //printf("packet %d %d\n", asf_st->pkt.size, asf->packet_frag_size);
	    asf_st->pkt.size = 0;
	    asf_st->pkt.data = 0;
	    break; // packet completed
	}
    }
    return 0;
}

static int asf_read_close(AVFormatContext *s)
{
    int i;

    for(i=0;i<s->nb_streams;i++) {
	AVStream *st = s->streams[i];
	av_free(st->priv_data);
	av_free(st->codec.extradata);
    }
    return 0;
}

static int asf_read_seek(AVFormatContext *s, int64_t pts)
{
    printf("SEEK TO %Ld", pts);
    return -1;
}

AVInputFormat asf_iformat = {
    "asf",
    "asf format",
    sizeof(ASFContext),
    asf_probe,
    asf_read_header,
    asf_read_packet,
    asf_read_close,
    asf_read_seek,
};

AVOutputFormat asf_oformat = {
    "asf",
    "asf format",
    "application/octet-stream",
    "asf,wmv",
    sizeof(ASFContext),
#ifdef CONFIG_MP3LAME
    CODEC_ID_MP3LAME,
#else
    CODEC_ID_MP2,
#endif
    CODEC_ID_MSMPEG4,
    asf_write_header,
    asf_write_packet,
    asf_write_trailer,
};

AVOutputFormat asf_stream_oformat = {
    "asf_stream",
    "asf format",
    "application/octet-stream",
    "asf,wmv",
    sizeof(ASFContext),
#ifdef CONFIG_MP3LAME
    CODEC_ID_MP3LAME,
#else
    CODEC_ID_MP2,
#endif
    CODEC_ID_MSMPEG4,
    asf_write_stream_header,
    asf_write_packet,
    asf_write_trailer,
};

int asf_init(void)
{
    av_register_input_format(&asf_iformat);
    av_register_output_format(&asf_oformat);
    av_register_output_format(&asf_stream_oformat);
    return 0;
}

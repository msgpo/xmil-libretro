#include	"compiler.h"

#if !defined(DISABLE_SOUND)

#include	"wavefile.h"
#include	"dosio.h"
#include	"soundmng.h"
#include	"z80core.h"
#include	"pccore.h"
#include	"sound.h"
#include	"sndcsec.h"
/* #include	"getsnd.h" */


	SOUNDCFG	soundcfg;


#define	STREAM_CBMAX	4

typedef struct {
	void	*hdl;
	SOUNDCB	cbfn;
} CBTBL;

typedef struct {
	SINT32	*buffer;
	SINT32	*ptr;
	UINT	samples;
	UINT	reserve;
	UINT	remain;
#if defined(SUPPORT_WAVEREC)
	WAVEWR	rec;
#endif
	CBTBL	*cbreg;
	CBTBL	cb[STREAM_CBMAX];
} SNDSTREAM;

static	SNDSTREAM	sndstream;


static void streamreset(void) {

	SNDCSEC_ENTER;
	sndstream.ptr = sndstream.buffer;
	sndstream.remain = sndstream.samples + sndstream.reserve;
	sndstream.cbreg = sndstream.cb;
	SNDCSEC_LEAVE;
}

static void streamprepare(UINT samples) {

	CBTBL	*cb;
	UINT	count;

	count = min(sndstream.remain, samples);
	if (count) {
		ZeroMemory(sndstream.ptr, count * 2 * sizeof(SINT32));
		cb = sndstream.cb;
		while(cb < sndstream.cbreg) {
			cb->cbfn(cb->hdl, sndstream.ptr, count);
			cb++;
		}
		sndstream.ptr += count * 2;
		sndstream.remain -= count;
	}
}


#if defined(SUPPORT_WAVEREC)
/* wave rec */

BRESULT sound_recstart(const OEMCHAR *filename) {

	WAVEWR	rec;

	sound_recstop();
	if (sndstream.buffer == NULL) {
		return(FAILURE);
	}
	rec = wavewr_open(filename, soundcfg.rate, 16, 2);
	sndstream.rec = rec;
	if (rec) {
		return(SUCCESS);
	}
	return(FAILURE);
}

void sound_recstop(void) {

	WAVEWR	rec;

	rec = sndstream.rec;
	sndstream.rec = NULL;
	wavewr_close(rec);
}

static void streamfilewrite(UINT samples) {

	CBTBL	*cb;
	UINT	count;
	SINT32	buf32[2*512];
	BYTE	buf[2*2*512];
	UINT	r;
	UINT	i;
	SINT32	samp;

	while(samples) {
		count = min(samples, 512);
		ZeroMemory(buf32, count * 2 * sizeof(SINT32));
		cb = sndstream.cb;
		while(cb < sndstream.cbreg) {
			cb->cbfn(cb->hdl, buf32, count);
			cb++;
		}
		r = min(sndstream.remain, count);
		if (r) {
			CopyMemory(sndstream.ptr, buf32, r * 2 * sizeof(SINT32));
			sndstream.ptr += r * 2;
			sndstream.remain -= r;
		}
		for (i=0; i<count*2; i++) {
			samp = buf32[i];
			if (samp > 32767) {
				samp = 32767;
			}
			else if (samp < -32768) {
				samp = -32768;
			}
			/* little endianなので satuation_s16は使えない */
			buf[i*2+0] = (UINT8)samp;
			buf[i*2+1] = (UINT8)(samp >> 8);
		}
		wavewr_write(sndstream.rec, buf, count * 4);
		samples -= count;
	}
}

static void filltailsample(UINT count) {

	SINT32	*ptr;
	UINT	orgsize;
	SINT32	sampl;
	SINT32	sampr;

	count = min(sndstream.remain, count);
	if (count) {
		ptr = sndstream.ptr;
		orgsize = (ptr - sndstream.buffer) / 2;
		if (orgsize == 0) {
			sampl = 0;
			sampr = 0;
		}
		else {
			sampl = *(ptr - 2);
			sampr = *(ptr - 1);
		}
		sndstream.ptr += count * 2;
		sndstream.remain -= count;
		do {
			ptr[0] = sampl;
			ptr[1] = sampr;
			ptr += 2;
		} while(--count);
	}
}
#endif


/* ---- */

BRESULT sound_create(UINT rate, UINT ms) {

	UINT	samples;
	UINT	reserve;

	ZeroMemory(&sndstream, sizeof(sndstream));
	samples = soundmng_create(rate, ms);
	if (samples == 0) {
		goto scre_err1;
	}
	soundmng_reset();

	soundcfg.rate = rate;

#if defined(SOUNDRESERVE)
	reserve = rate * SOUNDRESERVE / 1000;
#else
	reserve = 0;
#endif
#ifdef __LIBRETRO__
reserve=0;
samples=735;
#endif

	sndstream.buffer = (SINT32 *)_MALLOC((samples + reserve) * 2 
												* sizeof(SINT32), "stream");
	if (sndstream.buffer == NULL) {
		goto scre_err2;
	}
	sndstream.samples = samples;
	sndstream.reserve = reserve;

	SNDCSEC_INIT;
	streamreset();
	return(SUCCESS);

scre_err2:
	soundmng_destroy();

scre_err1:
	return(FAILURE);
}

void sound_destroy(void) {

	if (sndstream.buffer) {
#if defined(SUPPORT_WAVEREC)
		sound_recstop();
#endif
		soundmng_stop();
		streamreset();
		soundmng_destroy();
		SNDCSEC_TERM;
		_MFREE(sndstream.buffer);
		sndstream.buffer = NULL;
	}
}

void sound_reset(void) {

	if (sndstream.buffer) {
		soundmng_reset();
		streamreset();
		soundcfg.lastclock = CPU_CLOCK;
	}
}

void sound_changeclock(void) {

	UINT32	clock;
	UINT	hz;
	UINT	hzmax;

	if (sndstream.buffer == NULL) {
		return;
	}

	/* とりあえず 25で割り切れる。 */
	clock = pccore.realclock / 25;
	hz = soundcfg.rate / 25;

	/* で、クロック数に合せて調整。(64bit演算しろよな的) */
	hzmax = (1 << (32 - 8)) / (clock >> 8);
	while(hzmax < hz) {
		clock = (clock + 1) >> 1;
		hz = (hz + 1) >> 1;
	}
	TRACEOUT(("hzbase/clockbase = %d/%d", hz, clock));
	soundcfg.hzbase = hz;
	soundcfg.clockbase = clock;
	soundcfg.minclock = 2 * clock / hz;
	soundcfg.lastclock = CPU_CLOCK;
}

void sound_streamregist(void *hdl, SOUNDCB cbfn) {

	if (sndstream.buffer) {
		if ((cbfn) &&
			(sndstream.cbreg < (sndstream.cb + STREAM_CBMAX))) {
			sndstream.cbreg->hdl = hdl;
			sndstream.cbreg->cbfn = cbfn;
			sndstream.cbreg++;
		}
	}
}


/* ---- */

void sound_sync(void) {

	UINT32	length;

	if (sndstream.buffer == NULL) {
		return;
	}
	length = CPU_CLOCK + CPU_BASECLOCK - CPU_REMCLOCK - soundcfg.lastclock;
	if (length < soundcfg.minclock) {
		return;
	}
	length = (length * soundcfg.hzbase) / soundcfg.clockbase;
	if (length == 0) {
		return;
	}
	SNDCSEC_ENTER;
#if defined(SUPPORT_WAVEREC)
	if (sndstream.rec) {
		streamfilewrite(length);
	}
	else
#endif
	streamprepare(length);
	soundcfg.lastclock += length * soundcfg.clockbase / soundcfg.hzbase;
	SNDCSEC_LEAVE;

	soundcfg.writecount += length;
	if (soundcfg.writecount >= 100) {
		soundcfg.writecount = 0;
		soundmng_sync();
	}
}


static volatile int locks = 0;

const SINT32 *sound_pcmlock(void) {

const SINT32 *ret;

	if (locks) {
		/* TRACEOUT(("sound pcm lock: already locked")); */
		return(NULL);
	}
	locks++;
	ret = sndstream.buffer;
	if (ret) {
		SNDCSEC_ENTER;
		if (sndstream.remain > sndstream.reserve)
#if defined(SUPPORT_WAVEREC)
			if (sndstream.rec) {
				filltailsample(sndstream.remain - sndstream.reserve);
			}
			else
#endif
		{
			streamprepare(sndstream.remain - sndstream.reserve);
			soundcfg.lastclock = CPU_CLOCK + CPU_BASECLOCK - CPU_REMCLOCK;
		}
	}
	else {
		locks--;
	}
	return(ret);
}

void sound_pcmunlock(const SINT32 *hdl) {

	int		leng;

	if (hdl) {
		leng = sndstream.reserve - sndstream.remain;
		if (leng > 0) {
			CopyMemory(sndstream.buffer,
						sndstream.buffer + (sndstream.samples * 2),
												leng * 2 * sizeof(SINT32));
		}
		sndstream.ptr = sndstream.buffer + (leng * 2);
		sndstream.remain = sndstream.samples + sndstream.reserve - leng;
		/* sndstream.remain += sndstream.samples; */
		SNDCSEC_LEAVE;
		locks--;
	}
}

#endif



#ifdef __cplusplus
extern "C" {
#endif

#define	PAL_NORMAL		0x00
#define PAL_HIGHRESO	0x01

#define PAL_4096H		0x00
#define PAL_4096L		0x01

#define	PAL_4096BANK	0x01
#define PAL_64x2		0x02
#define PAL_4096FULL	0x04
#define	PAL_4096		0x10


#define	SCRN_BANK0			0x00
#define	SCRN_BANK1			0x80
#define	SCRN_DRAW4096		0x40

#define	SCRN64_MASK				0x0f
#define	SCRN64_INVALID			0
#define	SCRN64_320x200			1
#define	SCRN64_L320x200x2		2
#define	SCRN64_L640x200			3
#define	SCRN64_H320x400			4
#define	SCRN64_320x200x4096		5

#define	SCRN64_320x100			6
#define	SCRN64_320x100x2		7
#define	SCRN64_L640x100			8
#define	SCRN64_H320x200			9
#define	SCRN64_320x100x4096		10


#define	SCRN_24KHZ		0x01	// 0:15KHz		1:24KHz
#define	SCRN_200LINE	0x02	// 0:400line	1:200line
#define	SCRN_TEXTYx2	0x04	// 0:�m�[�}��	1:�c�Ɋg��
#define	SCRN_DISPVRAM	0x08	// 0:�\			1:��
#define	SCRN_ACCESSVRAM	0x10	// 0:�\			1:��
#define	SCRN_PCGMODE	0x20	// 0:�݊�		1:����
#define	SCRN_CPUFONT	0x40	// 0:8���X�^	1:16���X�^
#define	SCRN_UNDERLINE	0x80	// 0:�Ȃ�		1:����

#define	SCRN_DISPCHANGE	(SCRN_24KHZ | SCRN_200LINE |					\
									SCRN_TEXTYx2 | SCRN_UNDERLINE)


typedef struct {
	UINT8	PAL_B;
	UINT8	PAL_R;
	UINT8	PAL_G;
	UINT8	PLY;
	UINT8	TEXT_PAL[8];
	UINT8	SCRN_BITS;
	UINT8	CRTC_NUM;

	WORD	DISP_PAGE;
	WORD	FNT_XL;
	WORD	FNT_YL;
	BYTE	TXT_XL;
	BYTE	TXT_YL;
	BYTE	TXT_YS;
	WORD	GRP_XL;
	WORD	GRP_YL;
	BYTE	CPU_BANK;
	BYTE	CRT_BANK;

	WORD	CRT_YL;
	WORD	CRT_VS;
	WORD	CRT_VL;

	BYTE	TXT_VL;
	BYTE	TXT_VLA;

	WORD	TXT_TOP;
	WORD	fnty;

	UINT8	BLACKPAL;
	UINT8	EXTPALMODE;
	UINT8	EXTGRPHPAL;
	UINT8	ZPRY;
	UINT8	lastpal;
} CRTCSTAT;

typedef struct {
	UINT8	*gram;				// curvram
	UINT	updatemask;			// updatemsk
	UINT8	updatebit;			// curupdt
} CRTCEXT;

typedef struct {
	CRTCSTAT	s;
	CRTCEXT		e;
} CRTC;


extern	BYTE	crtc_TEXTPAL[8];
extern	WORD	crtc_GRPHPAL[2][64];
extern	WORD	crtc_PAL4096[4096];


//**********************************************************************

void vrambank_patch(void);

void IOOUTCALL crtc_o(UINT port, REG8 value);			// x1_crtc_w

void IOOUTCALL scrn_o(UINT port, REG8 value);			// x1_scrn_w
REG8 IOINPCALL scrn_i(UINT port);						// x1_scrn_r

void IOOUTCALL ply_o(UINT port, REG8 value);			// x1_ply_w
REG8 IOINPCALL ply_i(UINT port);						// x1_ply_r

void IOOUTCALL palette_o(UINT port, REG8 value);		// x1_palet_w
REG8 IOINPCALL palette_i(UINT port);					// x1_palet_r

void IOOUTCALL extpal_o(UINT port, REG8 value);			// x1_extpal_w
REG8 IOINPCALL extpal_i(UINT port);						// x1_extpal_r

void IOOUTCALL extgrphpal_o(UINT port, REG8 value);		// x1_extgrphpal_w
REG8 IOINPCALL extgrphpal_i(UINT port);					// x1_extgrphpal_r

void IOOUTCALL exttextpal_o(UINT port, REG8 value);		// x1_exttextpal_w
REG8 IOINPCALL exttextpal_i(UINT port);					// x1_exttextpal_r

void IOOUTCALL exttextdisp_o(UINT port, REG8 value);	// x1_exttextdisp_w
REG8 IOINPCALL exttextdisp_i(UINT port);				// x1_exttextdisp_r

void IOOUTCALL blackctrl_o(UINT port, REG8 value);		// x1_blackctrl_w
REG8 IOINPCALL blackctrl_i(UINT port);					// x1_blackctrl_r

void crtc_initialize(void);
void crtc_reset(void);

#ifdef __cplusplus
}
#endif

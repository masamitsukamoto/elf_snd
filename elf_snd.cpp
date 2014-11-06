//#define ELFSND_WAVE_WRITE

//20030712 for Syabu-san
//20010529

#include <windows.h>
#include <string.h>

#include	"x68sound.h"
#include	"sound_iocs.h"
#include	"stdio.h"

#include "resource.h"

/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
/***************************************************************************/


#include	"snd_data.c"


USHORT snd_mask = 0x1ff;
const UCHAR* tone_table;    //= TONE_TABLE;
const UCHAR* adpcm_data_adr;	//= ADPCM_DATA_ADR;
int elf_pcm_shift = 2; // ELLE=2, Shan=1, TNSN=, DGK3=

// X68Sound_Free() を呼び出してからプログラムを終了するルーチン
void	exit_program(int ret) {
    X68Sound_Free();	// X68Sound.dllの終了
    exit(ret);
}

/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
class M68REG_DATA
{
public:
    union {
	unsigned l;
	unsigned short w;
	unsigned char b;
    };
} d0,d1,d2,d3,d4,d5,d6,d7;


//UCHAR *a0,*cmpi,*a2,*a3,*a4,*a5,*a6,*a7;
const UCHAR *a0,*a1;
UCHAR *a3;

class C_SND_TBL
{
private:

public:
    unsigned short hdr1;
    unsigned short hdr2;
    unsigned short tempo;

    const unsigned char* trk[10];
    C_SND_TBL(){};
    C_SND_TBL(USHORT h1, USHORT h2, USHORT h3,
	      const UCHAR *t1, const UCHAR *t2, const UCHAR *t3, const UCHAR *t4,
	      const UCHAR *t5, const UCHAR *t6, const UCHAR *t7, const UCHAR *t8,
	      const UCHAR *t9) {
	hdr1 = h1;
	hdr2 = h2;
	tempo = h3;
	trk[0] = t1;
	trk[1] = t2;
	trk[2] = t3;
	trk[3] = t4;
	trk[4] = t5;
	trk[5] = t6;
	trk[6] = t7;
	trk[7] = t8;
	trk[8] = t9;
	trk[9] = NULL;
    }
    C_SND_TBL(USHORT h1, const UCHAR *t7, const UCHAR *t8, const UCHAR *t9) {
	static const UCHAR _ff000000[] = {0xff,0x00,0x00,0x00};

	hdr1 = 0x0000; //h1;
	hdr2 = 0x0000;
	tempo = 0x00ca;
	trk[6] = (t7 == NULL) ? _ff000000 : t7;
	trk[7] = (t8 == NULL) ? _ff000000 : t8;
	trk[8] = (t9 == NULL) ? _ff000000 : t9;
	trk[9] = NULL;

	trk[0] = _ff000000;
	trk[1] = _ff000000;
	trk[2] = _ff000000;
	trk[3] = _ff000000;
	trk[4] = _ff000000;
	trk[5] = _ff000000;

    }
};

class C_WorkAll 
{
public:

    UCHAR be5;

    UCHAR c40;	// Current Trk
    UCHAR c41;
    UCHAR c43;
    UCHAR c44;
    UCHAR c45;
    USHORT c46;
    UCHAR c48;
    UCHAR c49;
    UCHAR c4a;
	
    UCHAR c4e[0x20];
    UCHAR c6e[0x08];
    UCHAR c76[0x6b];
    UCHAR ce2[0x66];

    USHORT  I1318;

	void init(){
		c40 = 0x00;	//
		c41 = 0x00;
				
		c43 = 0x01;
		c44 = 0x63;
		c45 = 0x00;
		c46 = 0x0000;
		c48 = 0x00;
		c49 = 0x00;
		c4a = 0x00;
	}
    C_WorkAll(){ init(); }
} work;
class C_WorkTrk
{

public:
    const UCHAR*	I00;	// $00-$03
    UCHAR	I04;	// $04
    UCHAR	I05;	// $05
    UCHAR	I06;	// $06
    UCHAR	I07;	// $07
    UCHAR	I08;	// $08
    UCHAR	I09;	// $09
    UCHAR	I0A;	// $0A
    UCHAR	I0B;	// $0B
    USHORT	I0C;	// $0C-0D
    // $$0E-$0F
    UCHAR	I10;	// $10
    UCHAR	I11;	// $11
    const UCHAR*	I12;	// $12-15
    const UCHAR*	I16;	// $16-19
    const UCHAR*	I1A;	// $1A-1D
    UCHAR	I1E;	// $1E
    UCHAR	I1F;	// $1F
    UCHAR	I20;	// $20
    UCHAR	I21;	// $21
    UCHAR	I22;	// $22
    UCHAR	I23;	// $23
    UCHAR	I24;	// $24
    UCHAR	I25;	// $25 : PAN
    USHORT	I26;	// $26-$27
    USHORT	I28;	// $28-29$
    // $2A-$2F
    // ADPCM


    //
    const UCHAR* ptr_st;

    void init() {
	I00 = NULL;	// $00-$03
	I04 = 0x00;	// $04
	I05 = 0x01;	// $05
	I06 = 0x0e;	// $06
	I07 = 0x00;	// $07
	I08 = 0x01;	// $08
	I09 = 0x18;	// $09
	I0A = 0x00;	// $0A
	I0B = 0x00;	// $0B
	I0C = 0x0000;	// $0C-0D
	// $$0E-$0F
	I10 = 0x00;	// $10
	I11 = 0x00;	// $11
	I12 = NULL;	// $12-15
	I16 = NULL;	// $16-19
	I1A = NULL;	// $1A-1D
	I1E = 0x00;	// $1E
	I1F = 0x00;	// $1F
	I20 = 0x00;	// $20
	I21 = 0x00;	// $21
	I22 = 0x00;	// $22
	I23 = 0x00;	// $23
	I24 = 0x00;	// $24
	I25 = 0x00;	// $25:PAN
	I26 = 0x0000;	// $26-$27
	I28 = 0x0000;	// $28-29$
	// $2A-$2F

	ptr_st = NULL;
    }

	C_WorkTrk(){ init();}
} work_trk[10];

C_WorkAll *a6;



struct ElfSoundList {
    C_SND_TBL *snd;
    char *name;
};

/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
/***************************************************************************/

void L0002e6();

void L0003d4();
void L0003dc();
void L0003f0(C_WorkTrk *a2);
void L000400(C_WorkTrk *a2);

void L0005f4(C_WorkTrk *a2);
void L0005fa(C_WorkTrk *a2);
void L000602(C_WorkTrk *a2);
void L000602__(C_WorkTrk *a2);

/*
void L000794(UCHAR *a0, C_WorkTrk *a2);
void L0007d8(UCHAR *a0, C_WorkTrk *a2);
void L000802(UCHAR *a0, C_WorkTrk *a2);
void L00087a(UCHAR *a0, C_WorkTrk *a2);
void L0008b6(UCHAR *a0, C_WorkTrk *a2);
*/
void L000794(C_WorkTrk *a2);
void L0007d8(C_WorkTrk *a2);
void L000802(C_WorkTrk *a2);
void L00087a(C_WorkTrk *a2);
void L0008b6(C_WorkTrk *a2);

void L000868(C_WorkTrk *a2);
void L0008a6(C_WorkTrk *a2);
void L0006a4();


/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
/***************************************************************************/

void OPM_Write(UCHAR d1, UCHAR d2)
{
    while (X68Sound_OpmPeek() & 0x80);
    X68Sound_OpmReg(d1);
    while (X68Sound_OpmPeek() & 0x80);
    X68Sound_OpmPoke(d2);

}

void proc_trap()
{
//L00015c:
    d0.w = 0x00;
    d0.b = d2.b;

    switch (d1.b)
    {

    case 0x00:
	// 再生
	break;
    case 0x01:
	// 停止
	break;
    case 0x02:
	// フェードアウト
	//L0009a4(d2.b);
	break;
    case 0x03:
	// 
	break;
    case 0x04:
	//
	break;
    }
}


/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
/***************************************************************************/

void L0001b0(const C_SND_TBL snd)
{
    int	i;	

    L0002e6();	//初期化
	
	
    //サウンドテーブル
    // a0 <-

    if (snd.hdr1 == 0x0001) goto L000254;

    //
    work.c46 = snd.hdr2;

    // テンポ
    d2.b = (unsigned char)((snd.tempo*0x60+0x32)/0x64);
//    printf("Timer-B = $%02x // 1024*(256-%d) /4000 (ms) // T = %d\n", d2.b, d2.b, 78125/(8*(256-d2.b)));
    OPM_Write(0x12, d2.b);

    //
    work.c41 = 0x00;
    work.c45 = 0x00;
    work.c48 = 0x00;
    work.c49 = 0x00;
    work.c44 = 0x63;


    for (i=0;i<9 ;i++) {
	work_trk[i].I00 = snd.trk[i];	
	work_trk[i].I04 = 0x00;
	work_trk[i].I05 = 0x00;
	work_trk[i].I1E = 0x00;
	work_trk[i].I22 = 0x00;
	work_trk[i].I23 = 0x00;
	work_trk[i].I11 = 0x00;
	work_trk[i].I25 = 0xc0;

	work_trk[i].ptr_st = snd.trk[i];

	if ( *(work_trk[i].I00) == 0xff) {
	    work_trk[i].I05 = 0x01;		// データなし
	}
	
	if (i ==8 ) {
	    work_trk[i].I24 = 0x03;
	    work_trk[i].I25 = 0x03;
	}
    }

    OPM_Write(0x0f, 00);

    L0003d4();
    return;


/*
L000a60:FM Ch.1
L000a90:FM Ch.2
L000ac0:FM Ch.3
L000af0:FM Ch.4
L000b20:FM Ch.5
L000b50:FM Ch.6
L000b80:FM Ch.7
L000bb0:FM Ch.8
L000be0:ADPCM
L000c10:ADPCM
*/
L000254:;
/*	movem.l	d0-d2/a0-a3,-(a7)
	lea.l	(L000b80,pc),a2
	tst.b	($0005,a2)
	beq.s	L00026a
	moveq.l	#$0e,d1
	moveq.l	#$00,d2
	bsr.w	L0003dc
*/
    //C_WorkTrk* a2 = &work_trk[6];
    if (work_trk[6].I05 == 0x00) goto L00026a;
//    d1.b = 0x0e;
//    d2.b = 0x00;
//    L0003dc();
    OPM_Write(0x0e, 0x00);

L00026a:;
/*	addq.l	#1,d2
	tst.b	($0035,a2)
	beq.s	L000276
	bsr.w	L0003dc
*/
//    d2.b++;
    if (work_trk[7].I05 == 0x00) goto L000276;
//    L0003dc();
    OPM_Write(0x0e, 0x01);

L000276:;
/*	movem.l	(a7)+,d0-d2/a0-a3

	move.w	#$0003,d2
	lea.l	(L000b80,pc),a2
L000282:
	cmp.b	#$01,d2
	beq.s	L0002b6

	movea.l	(a0),a1
	cmpi.b	#$ff,(a1)
	beq.s	L0002b4
	move.l	(a0),(a2)
	clr.b	($0004,a2)
	clr.b	($0005,a2)
	clr.b	($001e,a2)
	clr.l	($0012,a2)
	clr.b	($0022,a2)
	clr.b	($0023,a2)
	clr.b	($0011,a2)
	move.b	($1318,a6),($0025,a2)
L0002b4:
	addq.l	#4,a0
L0002b6:
	adda.l	#$00000030,a2
	dbra.w	d2,L000282
*/
    for (i=6;i<9;i++) {
	if ( *(snd.trk[i]) != 0xff) {
	    if (i != 8) {
		work_trk[i].I00 = snd.trk[i];
    		work_trk[i].I04 = 0x00;
		work_trk[i].I05 = 0x00;
		work_trk[i].I1E = 0x00;
		work_trk[i].I22 = 0x00;
		work_trk[i].I23 = 0x00;
		work_trk[i].I11 = 0x00;
		work_trk[i].I25 = (UCHAR)((work.I1318) & 0xff);
	    } else {
		work_trk[9].I00 = snd.trk[i];
    		work_trk[9].I04 = 0x00;
		work_trk[9].I05 = 0x00;
		work_trk[9].I1E = 0x00;
		work_trk[9].I22 = 0x00;
		work_trk[9].I23 = 0x00;
		work_trk[9].I11 = 0x00;
		work_trk[9].I25 = (UCHAR)((work.I1318) & 0xff);
	    }
		
	    work_trk[(i!=8)?i:i+1].ptr_st = snd.trk[i];
	}
    }

/*
	suba.l	#$00000030,a2
	move.b	#$03,($0024,a2)
	move.b	($1319,a6),($0025,a2)
	tst.b	($0005,a2)
	bne.s	L0002e0
	moveq.l	#$67,d0
	moveq.l	#$00,d1
	bsr.w	L0003f0
L0002e0:
	rts
*/
    // a2 : work_trk[9]のポインタ
    C_WorkTrk* a2;
    a2 = &work_trk[9];
    a2->I24 = 0x03;
    a2->I25 = (UCHAR)(((work.I1318)>>7) & 0xff);
    if ( a2->I05 == 0x00) {
	d0.l = 0x67;
	d1.l = 0x00;
	L0003f0(a2);
    }
    return;    
/*
L0002e2:
	bsr.s	L0002e6
	rts
*/
    L0002e6();
    return;
}


void L0002e6()
{
/*	L0002e6:
	movem.l	d0-d7/a0-a6,-(a7)
	moveq.l	#$07,d0
	moveq.l	#$08,d1
	moveq.l	#$00,d2
	L0002f0:
	bsr.w	L0003dc
	addq.w	#1,d2
	dbra.w	d0,L0002f0
*/
    d1.b = 0x08;
    d2.b = 0x00;
    for (int i=0;i<9;i++) {
	L0003dc();
	d2.b++;
    }

/*	moveq.l	#$67,d0
	moveq.l	#$00,d1
	trap	#15
*/
    _iocs_adpcmmod((int) 0x00 );
/*
	lea.l	(L000a60,pc),a2
	moveq.l	#$08,d2
L000306:
	move.b	#$01,($0005,a2)
	adda.w	#$0030,a2
	dbra.w	d2,L000306
	movem.l	(a7)+,d0-d7/a0-a6
	rts
*/
	//a2 = L000a60;
    for (int i=0;i<9;i++) {
	work_trk[i].I05 = 0x01;
    }
}


/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
void L00031a()
{
    int i;
    a6 = &work;
    UCHAR *a4;
    C_WorkTrk *a2;

/*	lea.l	(L000000,pc),a6
	lea.l	($00e90003),a5
*/

/*	tst.b	($0c41,a6)
	beq.s	L00037a
	tst.b	($0c44,a6)
	bne.s	L000376
*/

    if (a6->c41 == 0x00) goto L00037a;
    if (a6->c44 != 0x00) goto L000376;
    

/*	addq.b	#1,($0c49,a6)
	move.b	($0c43,a6),($0c44,a6)
*/
    a6->c49++;
    a6->c44 = a6->c43;

/*	move.b	($0c45,a6),d0
  addq.b	#1,d0
  move.b	d0,($0c45,a6)
  cmpi.b	#$07,d0
  bne.s	L000350
  move.b	#$01,($0be5,a6)
*/
    d0.b = a6->c45;
    d0.b++;
    a6->c45 = d0.b;
    if (d0.b  != 0x07) goto L000350;
    a6->be5 = 0x01;


 L000350:;
/*	cmpi.b	#$0e,d0
	bne.s	L00037a
*/
    if (d0.b != 0x0e) goto L00037a;

/*	movem.l	d0/a0,-(a7)
	lea.l	(L000a65,pc),a0
	moveq.l	#$08,d0

	L000360:;
	move.b	#$01,(a0)
	adda.w	#$0030,a0
	dbra.w	d0,L000360
	movem.l	(a7)+,d0/a0
*/
    for (i=0;i<9;i++) {
	work_trk[i].I05 = 0x01;
    }

/*	clr.b	($0c41,a6)
	bra.s	L00037a
*/
    a6->c41 = 0x00;
    goto L00037a;


 L000376:;
/*	subq.b	#1,($0c44,a6)
 */
    a6->c44--;

 L00037a:;
/*	bsr.s	L0003d4
	lea.l	(L000c40,pc),a4
	clr.b	(a4)
	clr.b	($0c4a,a6)
*/
    L0003d4();
    a4 = &(a6->c40);

    *a4 = 0x00;
    a6->c4a = 0x00;
/*	lea.l	(L000a60,pc),a2
	bsr.s	L000400
*/
				//TRK 0
    a2 = &work_trk[0];
    L000400(a2);
/*	addq.b	#1,(a4)
	lea.l	(L000a90,pc),a2
	bsr.s	L000400
*/
    work.c40++;	//TRK 1
    a2 = &work_trk[1];
    L000400(a2);

/*	addq.b	#1,(a4)
	lea.l	(L000ac0,pc),a2
	bsr.s	L000400
*/
    work.c40++;	//TRK 2
    L000400(&work_trk[2]);

/*	addq.b	#1,(a4)
	lea.l	(L000af0,pc),a2
	bsr.s	L000400
*/
    work.c40++;	//TRK 3
    L000400(&work_trk[3]);

/*	addq.b	#1,(a4)
	lea.l	(L000b20,pc),a2
	bsr.s	L000400
*/
    work.c40++;	//TRK 4
    L000400(&work_trk[4]);

/*	addq.b	#1,(a4)
	lea.l	(L000b50,pc),a2
	bsr.s	L000400
*/
    work.c40++;	//TRK 5
    L000400(&work_trk[5]);

/*	addq.b	#1,(a4)
	lea.l	(L000b80,pc),a2
	bsr.s	L000400
*/
    work.c40++;	//TRK 6
    L000400(&work_trk[6]);

/*	addq.b	#1,(a4)
	lea.l	(L000bb0,pc),a2
	bsr.s	L000400
*/
    work.c40++;	//TRK 7
    L000400(&work_trk[7]);

/*	bsr.w	L0006a4
	move.b	#$01,($0c4a,a6)
	bsr.w	L0006a4
*/
//ADPCM

    L0006a4();
    work.c4a = 0x01;
    L0006a4();

//L0003d2:
}


/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
/*
L0003d4:
	moveq.l	#$14,d1
	moveq.l	#$3a,d2
	bra.w	L0003dc
L0003dc:
	btst.b	#$07,(a5)
	bne.s	L0003dc
	move.b	d1,(-$0002,a5)
L0003e6:
	btst.b	#$07,(a5)
	bne.s	L0003e6
	move.b	d2,(a5)
	rts
*/
void L0003d4()
{
	OPM_Write(0x14,0x3a);
}
void L0003dc()
{
	OPM_Write(d1.b,d2.b);
}
void L0003f0(C_WorkTrk *a2)
{
/*
L0003f0:
	tst.b	($0c4a,a6)
	bne.s	L0003fc
	tst.b	($0035,a2)
	beq.s	L0003fe
L0003fc:
	trap	#15
L0003fe:
	rts
*/
	// d0.b = 0x60 or 0x67
	if (work.c4a != 0x00) goto L0003fc;
//	if (a2->I35 != 0x00)) return;
	if (work_trk[9].I05 == 0x00) return;
L0003fc:
	// trap #15

	switch (d0.b) {
	case 0x60:
//		printf("%08x. %04x, %08x\n", (ULONG)(a1 -L00e568) /0x0c00, d1.w, d2.l);
//		if (d1.b > 03) d1.b = 0x03;

		if ((snd_mask & 0x100) == 0x00) break;
		_iocs_adpcmout((void*)a1, (int) d1.w , (int)d2.l);
		break;
	case 0x67:
		_iocs_adpcmmod((int)d1.b);
		break;
	default:
		break;
	}
}


/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
void L000400(C_WorkTrk *a2)
{
	//UCHAR *a0;
L000400:
/*
	tst.b	($0005,a2)
	beq.s	L000408
*/
	if (a2->I05 == 0) goto L000408;
/*
	rts
*/
	return;

L000408:
/*
	move.b	($0004,a2),d0
	cmp.b	($0008,a2),d0
	bne.s	L00041e
*/
	if ((d0.b = a2->I04) != a2->I08) goto L00041e;
/*	tst.b	($001e,a2)
	bne.s	L00041e
*/
	if (a2->I1E != 0x00 ) goto L00041e;

/*	moveq.l	#$08,d1
	move.b	(a4),d2
	bsr.s	L0003dc
*/
	d1.l = 0x08;
	d2.b = work.c40;
	L0003dc();
	//OPM_Write(0x08, work.c40 /*a4*/);

L00041e:
/*
	move.b	($0004,a2),d0
	bne.w	L00054c
	movea.l	(a2),a0
*/
//	if (work.c40 == 5) printf("%02x, ", a2->I04);
	if ((d0.b = a2->I04) != 0x00) goto L00054c;
//	if (work.c40 == 5) printf("\n %03x( %02x ):", (USHORT)(a2->I00 - a2->ptr_st), *a2->I00);
	
	a0 = a2->I00;
L000428:
/*
	clr.b	($0010,a2)
	move.b	($000a,a2),($000b,a2)
*/
	// if (work.c40 == 5) printf("%04x : %02x,%02x\n ",(int)(a0-a2->ptr_st), *a0, *(a0+1));
	a2->I10 = 0x00;
	a2->I0B = a2->I0A;
/*	move.b	(a0),d0
	and.b	#$f8,d0
	cmp.b	#$f8,d0
	bne.w	L0004e0
	move.b	(a0),d0
	cmp.b	#$ff,d0
	bne.s	L00046e
*/
	d0.b = *a0;
	if ((d0.b = d0.b & 0xf8) != 0xf8) goto L0004e0;
	
	// 0xf8～0xfeだったら
	if ((d0.b = *a0) !=0xff) goto L00046e;
	
	// 0xffだったら
/*	addq.l	#1,a0
	move.l	a0,d6
	btst.l	#$00,d6
	beq.s	L000454
	addq.l	#1,a0
*/
	a0++;
	d6.l = (ULONG)a0;

	if (((UCHAR)(a0 - a2->ptr_st) & 0x01) == 0x00) goto L000454;
	//if ((d6.l & 0x01) == 0x00) goto L000454;
	a0++;
L000454:;
/* 
	move.w	(a0),d6
	tst.w	d6
	beq.s	L000466
*/
	if ((d6.b = *a0) == 0x00) goto L000466;

/*	movea.l	(a0),a0
	bsr.w	L0001ba
	addq.l	#4,a7
	bra.w	L0003d2   // rts
*/
	/***********************/
	/***********************/
	/***********************/
	/***********************/
	/***********************/
	/***********************/
//	printf("ERROR\n");
	return;

L000466:;
/*
	move.b	#$01,($0005,a2)
	rts
*/
	a2->I05 = 0x01;
	return;

L00046e:;
/*
	cmp.b	#$fe,d0
	bne.s	L000480
	move.b	($0001,a0),d0
	bsr.w	L0005fa
	addq.l	#2,a0
	bra.s	L000428
L000480:
	cmp.b	#$fd,d0
	bne.s	L000496
	move.b	($0001,a0),d0
	and.b	#$0f,d0
	bsr.w	L000602
	addq.l	#2,a0
	bra.s	L000428
L000496:
	cmp.b	#$fc,d0
	bne.s	L0004a2
	bsr.w	L000794
	bra.s	L000428
L0004a2:
	cmp.b	#$fb,d0
	bne.s	L0004b0
	bsr.w	L0007d8
	bra.w	L000428
L0004b0:
	cmp.b	#$fa,d0
	bne.s	L0004be
	bsr.w	L000802
	bra.w	L000428
L0004be:
	cmp.b	#$f9,d0
	bne.s	L0004cc
	bsr.w	L00087a
	bra.w	L000428
L0004cc:
	cmp.b	#$f8,d0
	bne.s	L0004da
	bsr.w	L0008b6
	bra.w	L000428
L0004da:
	addq.w	#2,d6
	bra.w	L000428
*/

	//printf("%02x ", d0.b);
	switch (d0.b)
	{
	case 0xfe:
		d0.b =*(a0+1);
		// bsr.w	L0005fa
		L0005fa(a2);
		a0+=2;
		break;
	case 0xfd:
		d0.b =*(a0+1) & 0x0f;
		L000602(a2);
		a0+=2;
		break;
	case 0xfc:
		L000794(a2);
		break;
	case 0xfb:
		L0007d8(a2);
		break;
	case 0xfa:
		L000802(a2);
		break;
	case 0xf9:
		L00087a(a2);
		break;
	case 0xf8:
		L0008b6(a2);
		break;
	default:
		d6.b+=2;
	}

	goto L000428;




L0004e0:;
/*	move.b	(a0),d0
	and.w	#$007f,d0
	cmp.b	#$70,d0
	beq.s	L00052a
	add.b	($0022,a2),d0
	move.w	d0,d2
	lsl.w	#8,d0
	moveq.l	#$14,d1
	add.b	($0023,a2),d1
	or.w	d1,d0
	move.w	d0,($0026,a2)
	tst.b	($0011,a2)
	bne.s	L00050e
	move.w	d0,($0028,a2)
	clr.w	($0026,a2)
*/
	d0.b = *a0;
//	printf("%02x, ", d0.b);
	d0.w &= 0x007f;
	if (d0.b == 0x70) {
//		if (work.c40 == TEST_TRK) printf("%03x (  R ", (USHORT)(a0-a2->ptr_st));
		goto L00052a;
	}
	d0.b += a2->I22;
	d2.w = d0.w;
	d0.w <<=8;
	d1.l = 0x00000014;
 	d1.b+=a2->I23;
	d0.w |= d1.w;
	a2->I26 = d0.w;
	if (a2->I11 != 0x00) {
		goto L00050e;
	}
	a2->I28 = d0.w;
	a2->I26 = 0x0000;

L00050e:;
/*
	lea.l	(L000c76,pc),a3
	move.b	(a3,d2.w),d2
	moveq.l	#$28,d1
	add.b	(a4),d1
	bsr.w	L0003dc
	moveq.l	#$08,d1
	move.b	(a4),d2
	or.b	#$78,d2
	bsr.w	L0003dc
*/

	a3 = L000c76;
	d2.b = *(a3+d2.w);
	d1.b = 0x28 + work.c40;
	L0003dc();
	{
		char *note[] = {
			"D+",	// 0x00
			"E ",	// 0x01
			"F ",	// 0x02
			"/**/",	// 0x03
			"F+",	// 0x04
			"G ",	// 0x05
			"G+",	// 0x06
			"/**/",	// 0x07
			"A ",	// 0x08
			"A+",	// 0x09
			"B ",	// 0x0a
			"/**/",	// 0x0b
			"C ",	// 0x0c
			"C+",	// 0x0d
			"D ",	// 0x0e
			"/**/",	// 0x0f
		};

//		if (work.c40 == TEST_TRK) printf("%03x (o%d%s",(USHORT)(a0-a2->ptr_st), (((d2.b)&0x7f)>>4) + ((((d2.b) &0x0f) >=0x0c) ? 1 :0) , note[(d2.b) &0x0f ]);

	}
	//printf("($%02x,$%02x), ", d1.b, d2.b);//##MT
	d1.b = 0x08;
	d2.b = work.c40 | 0x78;

	if (((snd_mask >> work.c40) & 0x01) == 0) {
	    d2.b = work.c40;
	}


	L0003dc();

L00052a:;
/*	btst.b	#$07,(a0)
	bne.s	L000536
	move.b	($0009,a2),d0
	bra.s	L000540
*/
	if ((*a0 & 0x80) != 0x00)  goto L000536;
	d0.b = a2->I09;
	goto L000540;
L000536:
/*	move.b	($0001,a0),d0
	move.b	d0,($0009,a2)
	addq.l	#1,a0
*/
	a2->I09 = d0.b = *(a0+1);
	a0++;
L000540:
/*	move.b	d0,($0004,a2)
	addq.l	#1,a0
	move.l	a0,(a2)
	bra.w	L000400
*/
	a2->I04 = d0.b;
//	if (work.c40 == TEST_TRK) printf(",%03d) @%2d  v%2d\n", d0.b,a2->I24,a2->I06);	//##MT
	a0++;
	a2->I00 = a0;
	goto L000400;

/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
/***************************************************************************/

L00054c:;
/*	subq.b	#1,($0004,a2)
	move.w	($0026,a2),d1
	beq.s	L000586
*/
 	a2->I04--;
	if ((d1.w = a2->I26) == 0x0000) goto L000586;
/*	move.w	($0028,a2),d0
	move.b	($0011,a2),d2
	ext.w	d2
	lsl.w	#2,d2
	move.w	d0,d3
	sub.w	d1,d3
	bcs.s	L00056a
	neg.w	d2
*/

	d0.w = a2->I28;
	d2.b = a2->I11;
	d2.w = (d2.b & 0x80) ? (0xff00 |(USHORT)d2.b) : (USHORT)d2.b;

	d2.w <<= 2;
	d3.w = d0.w;
	if (d1.w>d3.w){
		d3.w -= d1.w; goto L00056a;
	} else {
		d3.w -= d1.w;
		d2.w = -d2.w;
	}
	

L00056a:
/*
	add.w	d2,d0
	move.w	d0,d4
	sub.w	d1,d4
	eor.w	d4,d3
	btst.l	#$0f,d3
	bne.s	L00057e
	move.w	d0,($0028,a2)
	bra.s	L000586
*/
	d0.w += d2.w;
	d4.w = d0.w;
	d4.w -= d1.w;
	d3.w ^=d4.w;
	if ((d3.w & 0x8000) != 0x00) goto L00057e;
	a2->I28 = d0.w;
	goto L000586;


/*
	if (work.c40 == 3) {
		if (max_d0 < (int)(short)d0.w) max_d0 = (int)(short)d0.w;
		if (min_d0 > (int)(short)d0.w) min_d0 = (int)(short)d0.w;
//		printf("%04d (%3d, %3d), %3d:%02x\n",(int)(short)d0.w,max_d0,min_d0,iii,a2->I10_); // 
		printf("%04x, %3d:%02x - %04x\n",a2->I28, (int)(short)d0.w, a2->I10_, a2->I0C); // 
	}
*/	


L00057e:;
/*	move.w	d1,($0028,a2)
	clr.w	($0026,a2)
*/
	a2->I28 = d1.w;
	a2->I26 = 0x00;



L000586:;
/*
	move.b	($000b,a2),d0
	beq.s	L000594
	subq.b	#1,($000b,a2)
	clr.w	d0
	bra.s	L0005c0
*/
	if ((d0.b = a2->I0B) == 0x00) goto L000594;
	a2->I0B--;
	d0.w = 0x00;
	goto L0005c0;

L000594:
/*	clr.w	d0
	move.b	($0010,a2),d0
	add.w	($000c,a2),d0
	addq.b	#1,($0010,a2)
	move.b	(a6,d0.w),d0
	cmp.b	#$81,d0
	bne.s	L0005b4
	subq.b	#1,($0010,a2)
	clr.w	d0
	bra.s	L0005c0
*/
	d0.w = 0x00;
	d0.b = a2->I10 ;
	d0.w += a2->I0C;

	a2->I10++;

	//d0.b = *(a6+d0.w);
	//d0.b = *(music_dmp + d0.w);

	//d0.b = *(L000ce2 + d0.w-0xce2);
	//printf("(%04x,%04x,%04x), \n", min_d0, max_d0, d0.w);
	if (d0.w >= 0xd00 && d0.w <= 0xd26) {
		d0.b = *(L000d00+ d0.b );
	} else {
		d0.b = 0x00;
	}
	
	//	d0.b = *(L000d00+ d0.b );
//	if (work.c40 == 3) printf("%03x:%02x, ", d7.w, d0.b);
	//if (work.c40 == 0x03) printf("%04x, ", d0.w);

	if (d0.b != 0x81) goto L0005b4;
	a2->I10--;
	d0.w = 0x00;
	goto L0005c0;


L0005b4:
/*	cmp.b	#$80,d0
	bne.s	L0005c0
	clr.b	($0010,a2)
	bra.s	L000594
*/
	if (d0.b != 0x80) goto L0005c0;
	a2->I10 = 0x00;
	d0.w = 0x0000;
	goto L000594;


L0005c0:
/*	ext.w	d0
	asl.w	#2,d0
	add.w	d0,($0028,a2)
	move.w	($0028,a2),d0
	bge.s	L0005d0
	clr.w	d0
*/
//	d0.w = (SHORT)(d0.b);

/*	
	if (work.c40 == 3 && d0.w != 0x0000) {
//		printf("%04x / %04x, %04x (%02x) ",a2->I0C, a2->I10, d0.w, *(L000d00+d0.b));
//		printf("%04x / %04x, %04x (%02x) ",a,b,c,d);
	}
*/
	d0.w = (d0.b >= 0x80) ? (0xff00 |(USHORT)d0.b) : (USHORT)d0.b;
	d0.w <<=2;

//	d0.w = 0 ;

	a2->I28 += d0.w;
	if (((d0.w = a2->I28) & 0x8000) == 0x00) goto L0005d0;	
	d0.w = 0x00;

L0005d0:
/*	move.w	d0,d2
	lsr.w	#8,d2
	lea.l	(L000c76,pc),a3
	move.b	(a3,d2.w),d2
	moveq.l	#$28,d1
	add.b	(a4),d1
	bsr.w	L0003dc
	move.w	d0,d2
	addq.w	#8,d1
	bsr.w	L0003dc
	tst.b	($0c44,a6)
	beq.s	L0005f4
	rts
*/

	d2.w = d0.w;
	d2.w >>=8;
	a3 = L000c76;
	d2.b = *(a3+d2.w);
	d1.b = 0x28 + work.c40;
	L0003dc();
	d2.b = d0.b;
	d1.b += 8;
	L0003dc();
//	if(work.c40 == 0) OPM_Write(0x30 + work.c40, 0x00);
//	else OPM_Write(0x30 + work.c40, 0x00);

	if (work.c44 == 0x00) L0005f4(a2);

}


/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
void L0005f4(C_WorkTrk *a2)
{
//L0005f4:;
/*
	move.b	($0006,a2),d0
	bra.s	L000602
*/
	d0.b = a2->I06;
	L000602(a2);
}

void L0005fa(C_WorkTrk *a2)
{
//L0005fa:;
/*
	move.b	d0,($0024,a2)
	move.b	($0006,a2),d0
*/
	a2->I24 = d0.b;
	d0.b = a2->I06;
	L000602(a2);
}
//void L000602(C_WorkTrk *a2){}
void L000602(C_WorkTrk *a2)
{
/*
L000602:
	move.b	d0,($0006,a2)
	lea.l	(L00131e,pc),a1
	clr.w	d5
	move.b	($0024,a2),d5
	mulu.w	#$0019,d5
	adda.w	d5,a1
	move.b	($0018,a1),($0007,a2)
	move.b	(a4),d1
	add.b	#$40,d1
	moveq.l	#$03,d3

*/
	a2->I06 = d0.b;
	a1 = tone_table;//L00131e;// 音色
	d5.w = 0x0000;
	d5.b = a2->I24;
	d5.w *= 0x19;
	a1 += d5.w;
	a2->I07 = *(a1+0x18);
	d1.b = work.c40;
	d1.b += 0x40;
	d3.l = 0x03;

	for (int i=0;i<4;i++) {
/*
L000624:
	move.b	(a1)+,d2
	bsr.w	L0003dc
	addq.b	#8,d1
	dbra.w	d3,L000624
*/
		d2.b = *a1++;
		//L0003dc();
		OPM_Write(d1.b , d2.b);
		d1.b += 8;
	}


/*	move.b	($0006,a2),d0
	and.w	#$000f,d0
	sub.b	($0c49,a6),d0
	bge.s	L000640
	clr.b	d0
*/
	d0.b = a2->I06;
	d0.w &= 0x000f;
	if (((signed short)d0.b - (signed short)work.c49) >= 0x00) {
		d0.b -= work.c49;
		goto L000640;
	}
	d0.w = 0x00;

L000640:
/*
	lea.l	(L000c4e,pc),a3
	cmpi.b	#$40,($0024,a2)
	bne.s	L000650
	adda.w	#$0010,a3
*/
	a3 = L000c4e;
	if (a2->I24 == 0x40) {
		a3 +=0x10;
	}
  
/*
L000650:
	move.b	(a3,d0.w),d5
	move.b	($0007,a2),d0
	and.w	#$0007,d0
	lea.l	(L000c6e,pc),a3
	move.b	(a3,d0.w),d4
*/

	d5.b = *(a3+d0.w);
	d0.b = a2->I07;
	d0.w &= 0x0007;
	a3 = L000c6e;
	d4.b = *(a3+d0.w);



/*  
	moveq.l	#$03,d3
L000666:
	ror.w	#1,d4
	bcc.s	L000678
	moveq.l	#$7f,d2
	sub.b	(a1)+,d2
	add.b	d5,d2
	tst.b	d2
	bge.s	L000676
	moveq.l	#$7f,d2
L000676:
	bra.s	L00067a
L000678:
	move.b	(a1)+,d2
L00067a:
	bsr.w	L0003dc
	addq.w	#8,d1
	dbra.w	d3,L000666
*/

	for (int i=0;i<4;i++) {
		if ((d4.w & 0x01) == 0x01) {
			d4.w >>= 1;
			d2.l = 0x7f;
			d2.b -= *a1++;
			d2.b += d5.b;
			if (d2.b >= 0x80) {
				d2.b = 0x7f;				
			}
		} else {
			d4.w >>= 1;
			d2.b = *a1++;
		}
		//L0003dc()
		OPM_Write(d1.b,d2.b);
		//printf("($%02x,$%02x), ",d1.b,d2.b);
		d1.b += 8;
	}

/*	move.w	#$000f,d3
L000688:
	move.b	(a1)+,d2
	bsr.w	L0003dc
	addq.b	#8,d1
	dbra.w	d3,L000688
*/
	for (int i=0;i<0x10;i++) {
		d2.b = *a1++;
		//L0003dc();
		OPM_Write(d1.b,d2.b);
		d1.b+=8;
	}
			
/*	add.b	#$20,d1
	move.b	(a1),d2
	or.b	($0025,a2),d2
	bsr.w	L0003dc
	rts

*/

	//d1.b += 0x20;
	d1.b = 0x20 + work.c40;
	d2.b = *a1;
	d2.b |= a2->I25;
	//L0003dc();
	OPM_Write(d1.b,d2.b);

/*
	///////////////
	// TEST CODE //
	///////////////
	int vol_no = a2->I06 ;
	int tone_no = a2->I24;
	int vol_mask = L000c6e[ a2->I07 & 0x07 ];
	int vol = L000c4e[vol_no];

	a1 = L00131e + tone_no * 0x19;
	d1.b = work.c40 + 0x40;
	for (i=0;i<4;i++) {
		OPM_Write(d1.b, *a1++);
		d1.b += 0x08;
	}
	OPM_Write(d1.b, (vol_mask & 0x01) ? 0x7f - *a1 + vol : *a1);
	a1++;
	d1.b += 0x08;
	OPM_Write(d1.b, (vol_mask & 0x02) ? 0x7f - *a1 + vol : *a1);
	a1++;
	d1.b += 0x08;
	OPM_Write(d1.b, (vol_mask & 0x04) ? 0x7f - *a1 + vol : *a1);
	a1++;
	d1.b += 0x08;
	OPM_Write(d1.b, (vol_mask & 0x08) ? 0x7f - *a1 + vol : *a1);
	a1++;
	d1.b += 0x08;

	for (i=0;i<0x10;i++) {
		OPM_Write(d1.b, *a1++);
		d1.b += 0x08;
	}
	d1.b += 0x20;
	d1.b = 0x20 + work.c40;
	d2.b = *a1;
	d2.b |= a2->I25;
	OPM_Write(d1.b,d2.b);

*/
}


/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
void L0006a4()
{
	C_WorkTrk *a2;
/*
L0006a4:
	lea.l	(L000be0),a2
	tst.b	($0c4a,a6)
	beq.s	L0006b6
	adda.l	#$00000030,a2
*/
	a2 = &work_trk[8];
	if (work.c4a == 0x00) goto L0006b6;
	a2 = &work_trk[9];

L0006b6:;
/*
	tst.b	($0005,a2)
	beq.s	L0006be
	rts
*/
	if (a2->I05 != 0x00) return;

/*
L0006be:
	move.b	($0004,a2),d0
	cmp.b	($0008,a2),d0
	bne.s	L0006d0
	moveq.l	#$67,d0
	moveq.l	#$00,d1
	bsr.w	L0003f0
*/
	if ((d0.b = a2->I04) == a2->I08) {
		d0.l = 0x67;
		d1.l = 0x00;
		L0003f0(a2);
	}

/*
L0006d0:
	move.b	($0004,a2),d0
	bne.w	L00078e
	movea.l	(a2),a0
*/

	if ((d0.b = a2->I04) != 0x00) goto L00078e;

	a0 = a2->I00;

L0006da:;
/*	move.b	(a0),d0
	and.b	#$f8,d0
	cmp.b	#$f8,d0
	bne.s	L000736
*/
	//printf("PCM:%04x(%02x,%02x), ", (SHORT)(a0-L_E6),*a0,*(a0+1));
	if ((d0.b = *a0 & 0xf8) != 0xf8) goto L000736;

/*
	move.b	(a0),d0
	cmp.b	#$ff,d0
	bne.s	L000702
*/
//$f8-$feだったらL000702
	if ((d0.b = *a0) != 0xff) goto L000702;

//$ffだったら 
/*
	addq.l	#1,a0
	move.l	a0,d6
	btst.l	#$00,d6
	beq.s	L0006fa
	addq.l	#1,a0

L0006fa:
	move.b	#$01,($0005,a2)
	rts
*/
	a0++;

	//if ((ULONG)a0 & 0x01) a0++; //ポインタが奇数だったら偶数に
	if (((UCHAR)(a0 - a2->ptr_st) & 0x01) ) a0++;

	a2->I05 = 0x01;
	return;

L000702:
/*	cmp.b	#$fc,d0
	bne.s	L00070e
	bsr.w	L000794
	bra.s	L0006da
L00070e:
	cmp.b	#$fa,d0
	bne.s	L00071a
	bsr.w	L000868
	bra.s	L0006da
L00071a:
	cmp.b	#$f9,d0
	bne.s	L000726
	bsr.w	L0008a6
	bra.s	L0006da
L000726:
	cmp.b	#$f8,d0
	bne.s	L000732
	bsr.w	L0008b6
	bra.s	L0006da
L000732:
	addq.l	#2,a0
	bra.s	L0006da
*/
	switch (d0.b) {
	case 0xfc:
		L000794(a2);		
		break;
	case 0xfa:
		L000868(a2);
		break;
	case 0xf9:
		L0008a6(a2);
		break;
	case 0xf8:
		L0008b6(a2);
		break;
	default:
		a0+=2;
		break;
	}
	goto L0006da;


L000736:
/*	move.b	(a0),d0
	and.b	#$7f,d0
	cmp.b	#$70,d0
	beq.s	L00076c
*/
	if ((d0.b = *a0 & 0x7f) == 0x70) goto L00076c;
/*	clr.l	d1
	move.b	d0,d1
	move.b	d0,d7
	moveq.l	#$60,d0
	mulu.w	#$0c00,d1
	lea.l	(L00e568),a1
	adda.l	d1,a1
	move.w	($0024,a2),d1
	move.l	#$00000c00,d2
	cmp.b	#$0c,d7
	blt.s	L000768
	lsl.l	#2,d2
*/
	//d1.l = 0;
	d7.b = d1.b = d0.b;
//	a1 = L00e568 + d0.b * 0x0c00;
	a1 = adpcm_data_adr + d0.b * 0x0c00;
	d0.b = 0x60;
	d2.l = 0xc00;
	d1.w = (a2->I24 *0x100 + a2->I25);
	if (d7.b >= 0x0c) {
		d2.l <<= elf_pcm_shift;
	}

/*
L000768:
	bsr.w	L0003f0
*/
	//printf("PCM:%04x(%02x), ", (SHORT)(a0-L_E6), d7.b);
	L0003f0(a2);
L00076c:
/*	btst.b	#$07,(a0)
	bne.s	L000778
	move.b	($0009,a2),d0
	bra.s	L000782
*/
	if ((*a0 & 0x80) != 0x00) goto L000778;
	d0.b = a2->I09;
	goto L000782;
L000778:
/*	move.b	($0001,a0),d0
	move.b	d0,($0009,a2)
	addq.l	#1,a0
*/
	a2->I09 = d0.b = *(a0+1);
	a0++;
L000782:
/*	move.b	d0,($0004,a2)
	addq.l	#1,a0
	move.l	a0,(a2)
	bra.w	L0006b6
*/
	a2->I04 = d0.b;
	a0++;
	a2->I00 = a0;
	goto L0006b6;

L00078e:;
/*	subq.b	#1,($0004,a2)
	rts
*/
	a2->I04--;
}
/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
/***************************************************************************/

void L000794(C_WorkTrk *a2)
{
/*
L000794:
	cmpi.b	#$ff,($0001,a0)
	bne.s	L0007a4
	clr.b	($0011,a2)
	addq.l	#2,a0
	rts
*/
	if ( *(a0+1) == 0xff) {
		a2->I11 = 0x00;
		a0+=2;
		return;
	}
/*
L0007a4:
	cmpi.b	#$81,($0001,a0)
	bcs.s	L0007bc
	move.b	($0001,a0),d0
	and.b	#$7f,d0
	move.b	d0,($0011,a2)
	addq.l	#2,a0
	rts
*/
	if (*(a0+1) > 0x81) {
		a2->I11 = *(a0+1) & 0x7f;
		a0+=2;
		return;
	}
/*
L0007bc:
	move.b	($0001,a0),d0
	and.b	#$80,d0
	move.b	d0,($001e,a2)
	move.b	($0001,a0),d0
	and.b	#$7f,d0
	move.b	d0,($0008,a2)
	addq.l	#2,a0
	rts
*/
	d0.b = *(a0+1);
	a2->I1E = d0.b & 0x80;
	a2->I08 = d0.b & 0x7f;
	a0+=2;
	return;
}
void L0007d8(C_WorkTrk *a2)
{
/*
L0007d8:
	move.b	($0001,a0),d0
	ror.w	#3,d0
	and.w	#$001e,d0
	lea.l	(L000ce2,pc),a1
	move.w	(a1,d0.w),($000c,a2)
*/
	d0.b = *(a0+1);
	d0.w = (d0.w >> 3) & 0x001e;
	a1 = L000ce2;
	a2->I0C = *(a1+d0.w)*0x100 + *(a1+d0.w+1);

/*	move.b	($0001,a0),d0
	rol.w	#1,d0
	and.b	#$1e,d0
	beq.s	L0007fa
	subq.b	#1,d0
*/
	d0.b = *(a0+1);
	d0.w <<=1;
	d0.b &= 0x1e;
	if (d0.b == 0x00) goto L0007fa;
	d0.w--;


L0007fa:
/*	move.b	d0,($000a,a2)
	addq.l	#2,a0
	rts
*/
	a2->I0A= (UCHAR)d0.w;
	a0+=2;
	
}
void L000802(C_WorkTrk *a2)
{
/*
L000802:
	move.b	($0001,a0),d0
	cmp.b	#$01,d0
	beq.s	L000832
	cmpi.b	#$06,(a4)
	beq.s	L00083a
	cmpi.b	#$07,(a4)
	beq.s	L000854
*/
	d0.b = *(a0+1);
	if (d0.b == 0x01) goto L000832;
	if (work.c40 == 0x06) goto L00083a;
	if (work.c40 == 0x07) goto L000854;

/*	and.b	#$c0,d0
	move.b	d0,($0025,a2)
	or.b	($0007,a2),d0
	moveq.l	#$20,d1
	add.b	(a4),d1
	move.b	d0,d2
	bsr.w	L0003dc
	addq.l	#2,a0
	rts
*/
	d0.b &= 0xc0;
	a2->I25 = d0.b;
	d0.b |= a2->I07;
	d1.l = 0x20+work.c40;
	d2.b = d0.b;
	L0003dc();
	a0+=2;
	return;

L000832:
/*	addq.b	#1,($0c48,a6)
	addq.l	#2,a0
	rts
*/
	work.c48 = 0x01;
	a0+=2;
	return;

L00083a:
/*	moveq.l	#$12,d1
	clr.w	d2
	move.b	d0,d2
	mulu.w	#$0060,d2
	add.w	#$0032,d2
	divu.w	#$0064,d2
	bsr.w	L0003dc
	addq.l	#2,a0
	rts
*/
	d1.l = 0x12;
	d2.w = 0x0000;
	d2.w *= 0x60;
	d2.w += 0x32;
	d2.w /= 0x64;
	L0003dc();
	a0+=2;
	return;

L000854:
/*	moveq.l	#$0f,d1
	eori.w	#$001f,d0
	move.b	d0,d2
	or.w	#$0080,d2
	bsr.w	L0003dc
	addq.l	#2,a0
	rts
*/
	d1.b = 0x0f;
	d0.w ^= 0x1f;
	d2.b = d0.b;
	d2.b |= 0x80;
	L0003dc();
	a0+=2;
	return;
}

//ADPCM - $fa
void L000868(C_WorkTrk *a2)
{
/*
	L000868:
	move.b	($0001,a0),d0
	ror.w	#6,d0
	and.b	#$03,d0
	move.b	d0,($0025,a2)
	addq.l	#2,a0
	rts
*/
	a2->I25 = (*(a0+1) >> 6) & 0x03;
	a0+=2;


}

void L00087a(C_WorkTrk *a2)
{
/*
L00087a:
	move.b	($0001,a0),d0
	and.b	#$8f,d0
	bclr.l	#$07,d0
	beq.s	L00088c
	or.b	#$f0,d0
*/
	d0.b = *(a0+1);
	d0.b &= 0x8f;
	if ((d0.b & 0x80) == 0x00) {
		d0.l &= ~80;
		goto L00088c;
	}
	d0.l &= ~80;
	d0.b = d0.b | 0xf0;

L00088c:;
/*	move.b	d0,($0022,a2)
	move.b	($0001,a0),d0
	and.w	#$0070,d0
	mulu.w	#$0003,d0
	lsr.w	#2,d0
	move.b	d0,($0023,a2)
	addq.l	#2,a0
	rts
*/
	a2->I22 = d0.b;
	d0.b = *(a0+1);
	d0.w = (USHORT)d0.b & 0x0070;
	d0.w *=3;
	d0.w >>=2;
	a2->I23 = d0.b;
	a0+=2;
}

//ADPCM - $f9
void L0008a6(C_WorkTrk *a2)
{
/*
L0008a6:
	move.b	($0001,a0),d0
	and.b	#$07,d0
	move.b	d0,($0024,a2)
	addq.l	#2,a0
	rts
*/
	a2->I24 = *(a0+1) & 0x07;
	a0+=2;
}


//$f8
void L0008b6(C_WorkTrk *a2)
{
/*L0008b6:
	move.b	($0001,a0),d0
	move.b	d0,d1
	addq.l	#2,a0
	ror.w	#5,d0
	and.w	#$0007,d0
	bne.s	L0008cc
	move.l	a0,($0012,a2)
	rts
*/
	d0.b = *(a0+1);
	d1.b = d0.b;
	a0+=2;
	if ((d0.b = (d0.b >> 5) & 0x07) == 0x00) {
		a2->I12 = a0;	
		return;
	}

/*
L0008cc:
	cmp.b	#$01,d0
	bne.s	L0008e0
	move.l	a0,($0016,a2)
	clr.b	($0020,a2)
	clr.b	($0021,a2)
	rts
*/
	if (d0.b == 0x01) {
		a2->I16 = a0;
		a2->I20 = 0x00;
		a2->I21 = 0x00;
		return;
	}


/*
L0008e0:
	cmp.b	#$02,d0
	bne.s	L000920
	move.b	($0020,a2),d0
	bne.s	L0008f4
	move.b	#$01,($0020,a2)
	rts
*/
	if (d0.b != 0x02) goto L000920;

	if ((d0.b = a2->I20) == 0x00) {
		a2->I20 = 0x01;
		return;
	}

L0008f4:
/*	
	move.b	(a0),d0
	move.b	d0,d1
	move.b	($0001,a0),d0
	move.w	d0,d2
	addq.l	#2,a0
	cmp.b	#$ff,d1
	beq.s	L00091c
	cmp.b	#$f8,d1
	beq.s	L000916
	and.b	#$80,d1
	beq.s	L0008f4
	subq.l	#1,a0
	bra.s	L0008f4
*/
	d0.b = *a0;
	d1.b = d0.b;
	d0.b = *(a0+1);
	d2.w = d0.w;
	a0 +=2;

	if (d1.b == 0xff) goto L00091c;
	if (d1.b == 0xf8) goto L000916;
	if ((d1.b &= 0x80) == 0x00) goto L0008f4;
	a0--;
	goto L0008f4;

L000916:
/*	cmp.b	#$60,d2
	bne.s	L0008f4
*/
	if (d2.b != 0x60) goto L0008f4;
L00091c:
/*	subq.l	#2,a0
	rts
*/
	a0-=2;
	return;


L000920:;
/*
L000920:
	cmp.b	#$03,d0
	bne.s	L00095e
	move.b	($0021,a2),d0
	bne.s	L000934
	move.b	#$01,($0021,a2)
	rts
*/
	if (d0.b != 0x03) goto L00095e;
	if ((d0.b = a2->I21) = 0x00) goto L000934;
	a2->I21 = 0x01;
	return;

L000934:
/*
	move.b	(a0),d0
	move.b	d0,d1
	move.b	($0001,a0),d0
	move.w	d0,d2
	addq.l	#2,a0
	cmp.b	#$ff,d1
	beq.s	L00091c
	cmp.b	#$f8,d1
	beq.s	L000956
	and.b	#$80,d1
	beq.s	L000934
	subq.l	#1,a0
	bra.s	L000934
*/
	d1.b = d0.b = *a0;
	d0.b = *(a0+1);
	d2.w = d0.b;
	a0+=2;
	if (d1.b == 0xff) goto L00091c;
	if (d1.b == 0xf8) goto L000956;
	if ((d1.b &= 0x80)  == 0x00) goto L000934;
	a0--;
	goto L000934;

L000956:
/*	
	cmp.b	#$80,d2
	bne.s	L000934
	bra.s	L00091c
*/
	if (d2.b != 0x80) goto L000934;
	goto L00091c;

L00095e:;
/*
	cmp.b	#$04,d0
	bne.s	L000966
	rts
*/
	if (d0.b != 0x04) goto L000966;
	return;


L000966:;
/*
	cmp.b	#$05,d0
	bne.s	L00097e
	and.b	#$01,d1
	bne.s	L000978
	movea.l	($0016,a2),a0
	rts
*/
	if (d0.b != 0x05) goto L00097e;
	if ((d1.b &= 0x01) != 0x00) goto L000978;
	a0 = a2->I16;
	return;


L000978:
/*	movea.l	($0012,a2),a0
	rts
*/
	a0 = a2->I12;
	return;

L00097e:;
/*	cmp.b	#$06,d0
	bne.s	L000994
	move.l	a0,($001a,a2)
	and.w	#$001f,d1
	addq.w	#1,d1
	move.b	d1,($001f,a2)
	rts
*/
	if (d0.b !=0x06) goto L000994;
	a2->I1A = a0;
	d1.w &= 0x1f;
	d1.w++;
	a2->I1F = d1.b;
	return;

L000994:;
/*
	subq.b	#1,($001f,a2)
	bne.s	L00099c
	rts
*/
		a2->I1F--;
		if ((a2->I1F) != 0x00) goto L00099c;
		return;
L00099c:;
/*	movea.l	($001a,a2),a0
	rts
*/
		a0 = a2->I1A;
		return;
}

/*
L0009a2:
	.dc.b	$74,$12
*/
void L0009a4(UCHAR d2)
{
/*L0009a4:
	move.b	d2,($0c43,a6)
	move.b	#$01,($0c41,a6)
	move.b	($0c43,a6),($0c44,a6)
	rts
*/
	work.c43 = d2;
	work.c41 = 0x01;
	work.c44 = work.c43;
}


/*
L0009b6:
	lea.l	(L000a65,pc),a2
	clr.b	d0
	move.b	(a2),d0
	rts

L0009c0:
	clr.b	d0
	move.b	($0c48,a6),d0
	rts
*/



/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
void CALLBACK L00014e()
{

	L00031a();
	//printf(".");
}



/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
/***************************************************************************/


//////////////////////////////
//ELLE
#include    "elle_fm.c"
#include    "elle_pcm.c"
#include    "elle_lst.c"

//////////////////////////////
//シャングリラ
#include    "shan_fm.c"
#include    "shan_pcm.c"
#include    "shan_lst.c"

//////////////////////////////
//天神乱魔
#include    "tnsn_fm.c"
#include    "tnsn_fm2.c"
#include    "tnsn_pcm.c"
#include    "tnsn_lst.c"

//////////////////////////////
//DragonKnight3
#include    "dgk3_fm.c"
#include    "dgk3_tone.c"
#include    "dgk3_fm2.c"
#include    "dgk3_pcm.c"
#include    "dgk3_lst.c"

//////////////////////////////

static bool flg_pause = FALSE;
static int sel_no = 0;
static bool flg_disp_mask = TRUE;

class C_ELF_SND_DATA{
public:
    const char *game_title;
    const ElfSoundList *snd_lst;
    const UCHAR *tone_table;
    const UCHAR *adpcm_data_ptr;
    int snd_no;
    int snd_no_max;
    int pcm_shift;

    C_ELF_SND_DATA(const char *t, const ElfSoundList lst[],const UCHAR *tone, const UCHAR *pcm, int pcm_sft = 2)
    {
	game_title  = t;
	snd_lst	    = lst;
	tone_table  = tone;
	adpcm_data_ptr  = pcm;
	pcm_shift = pcm_sft;
	snd_no = 0;

        int i = 0;
	while( snd_lst[i].snd != NULL ) i++;
	snd_no_max = i-1;
    }
    void play(void){::elf_pcm_shift = pcm_shift; ::L0001b0( *(snd_lst+snd_no)->snd );}
    void stop(void){::L0002e6();}
    void fade_out(void){::L0009a4(0x14);}
    void next(void){if (++snd_no > snd_no_max) snd_no = 0;}
    void prev(void){if (--snd_no < 0) snd_no = snd_no_max;}
    const char* get_title(void){ return snd_lst[snd_no].name;}
    int get_tempo(){return snd_lst[snd_no].snd->tempo;}
    bool pause(){
	if (::_iocs_opmintst(::L00014e) == NULL ) {
	    ::L0003d4();
	    return FALSE;
	} else {
	    _iocs_opmintst(NULL);
	    return TRUE;
	}
    }

};

//#if 1
C_ELF_SND_DATA elle_data("ELLE / (c)1991 elf"	    ,elle_lst, elle_L00131e, elle_L00e568, 2);
C_ELF_SND_DATA shan_data("SHANGRLIA / (c)1991 elf"  ,shan_lst, shan_L000fd4, shan_L00981e, 1);
C_ELF_SND_DATA tnsn_data("天神乱魔 / (c)1992 elf"    ,tnsn_lst, tnsn_L001580, tnsn_L022c36);
C_ELF_SND_DATA dgk3_data("Dragon Knight 3 / (c)1991 elf",	dgk3_lst, dgk3_L00142a, dgk3_L015f20);
/*
#else
C_ELF_SND_DATA elle_data("ELLE (一部) / (c)1991 elf"	    ,elle_lst_, elle_L00131e, elle_L00e568);
C_ELF_SND_DATA shan_data("SHANGRLIA (一部) / (c)1991 elf"  ,shan_lst_, shan_L000fd4, shan_L00981e);
C_ELF_SND_DATA tnsn_data("天神乱魔 (一部) / (c)1992 elf"    ,tnsn_lst_, tnsn_L001580, tnsn_L022c36);
C_ELF_SND_DATA dgk3_data("Dragon Knight 3 (一部) / (c)1991 elf",	dgk3_lst_, dgk3_L00142a, dgk3_L015f20);
#endif

C_ELF_SND_DATA elle_data("ELLE / (c)1991 elf"	    ,elle_lst, elle_L00131e, elle_L00e568, 2);
C_ELF_SND_DATA shan_data("SHANGRLIA / (c)1991 elf"  ,shan_lst, shan_L000fd4, shan_L00981e, 1);
C_ELF_SND_DATA tnsn_data("天神乱魔 (一部) / (c)1992 elf"    ,tnsn_lst_, tnsn_L001580, tnsn_L022c36);
C_ELF_SND_DATA dgk3_data("Dragon Knight 3 (一部) / (c)1991 elf",	dgk3_lst_, dgk3_L00142a, dgk3_L015f20);
*/
C_ELF_SND_DATA* elf_data[] = {
	&elle_data,
	&shan_data,
	&tnsn_data,
	&dgk3_data,
	NULL,
};

static C_ELF_SND_DATA *current_data = elf_data[0];

int SampleRate[] = {22050, 44100, 48000};
int sample_rate = 1;

/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
/***************************************************************************/

/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
void elfsnd_write_file(int sel_no, int mus_no, char *filename,int sec, int rate)
{
	static short buf[48000][2];	// 生成したPCMデータを格納するバッファ
	static unsigned char	WAVEHEADER[0x2C] = {	// 44.1kHz16ビットステレオPCM
		0x52,0x49,0x46,0x46,0x00,0x00,0x00,0x00, 0x57,0x41,0x56,0x45,0x66,0x6D,0x74,0x20,
		0x10,0x00,0x00,0x00,0x01,0x00,0x02,0x00, 0x44,0xAC,0x00,0x00,0x10,0xB1,0x02,0x00,
		0x04,0x00,0x10,0x00,0x64,0x61,0x74,0x61, 0x00,0x00,0x00,0x00,
	};

	FILE *fp;

	current_data = elf_data[sel_no];
	tone_table = current_data->tone_table;
	adpcm_data_adr = current_data->adpcm_data_ptr;
	current_data->snd_no = 0;
	sample_rate = 1;
    X68Sound_Samprate(SampleRate[sample_rate]);

	for(int i = 0;i <mus_no;i++) current_data->next();

	if ((fp=fopen(filename, "wb")) == NULL) {
		printf("%s : write open error.\n",filename);
		exit_program(1);
	}
	*(int*)&WAVEHEADER[0x18] = SampleRate[sample_rate];
	*(int*)&WAVEHEADER[0x28] = SampleRate[sample_rate]*4 * sec/100;		// データ長(byte) 2秒
	*(int*)&WAVEHEADER[0x04] = *(int*)&WAVEHEADER[0x28] + 0x24;		// ckSize
	fwrite(WAVEHEADER, 1, 0x2C, fp);

    current_data->play();

	for (int j=0;j<sec;j++) {
		X68Sound_GetPcm(buf, SampleRate[sample_rate]/100);	// 44100/44100 秒のPCMデータを生成
		fwrite(buf, 4, SampleRate[sample_rate]/100, fp);
	}
	current_data->stop();
	fclose(fp);	
}
/***************************************************************************/
/***************************************************************************/
/***************************************************************************/


LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
BOOL InitApp(HINSTANCE, LPCSTR);
BOOL InitInstance(HINSTANCE, LPCSTR, int);
HFONT SetMyFont(HDC, LPCTSTR, int, int);
HFONT SetMyFont_bold(HDC, LPCTSTR, int, int);

HWND hParent;  //親ウィンドウハンドルの保存
HACCEL hAccel; //アクセラレーターテーブルハンドル

int WINAPI WinMain(HINSTANCE hCurInst, HINSTANCE hPrevInst,
                   LPSTR lpsCmdLine, int nCmdShow)
{
#ifndef	ELFSND_WAVE_WRITE
    MSG msg;
#endif	//ELFSND_WAVE_WRITE
    char szClassName[] = "elf_snd";        //ウィンドウクラス

    //if (*lpsCmdLine == NULL) exit(-1);
/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
    int ret;
    ret = X68Sound_Load();	// X68Sound.dll を読み込む
    if (ret != 0) {			// 戻り値が0以外は失敗
	if (ret == X68SNDERR_DLL) {
//	    printf("X68Sound.dll が見つかりません\n");
	} else if (ret == X68SNDERR_FUNC) {
//	    printf("X68Sound.dll のバージョンが違うようです\n");
	} else {
//	    printf("謎のエラー\n");
	}
	exit(1);
    }

#ifndef	ELFSND_WAVE_WRITE
    ret=X68Sound_Start( SampleRate[sample_rate], 1, 1, 5, 5, 200, 1.0);
	
	if (ret != 0) {										// 戻り値が0以外は失敗
	if (ret == X68SNDERR_PCMOUT) {
//	    printf("X68Sound : PCMサウンドを出力できません。\n");
	} else if (ret == X68SNDERR_TIMER) {
//	    printf("X68Sound : マルチメディアタイマーが利用できません。\n");
	} else if (ret == X68SNDERR_MEMORY) {
//	    printf("X68Sound : メモリ確保に失敗しました。\n");
	} else {
//	    printf("謎のエラー\n");
	}
//	printf("X68Sound : ErrorCode = $%X\n", X68Sound_ErrorCode());
	exit(1);
    }
    sound_iocs_init();	// IOCS を初期化して使えるようにする
    _iocs_opmintst(L00014e);

/***************************************************************************/
/***************************************************************************/
/***************************************************************************/
    if (!hPrevInst) {
        if (!InitApp(hCurInst, szClassName)) {
	    exit_program(0);
	    return FALSE;
	}
    }
    if (!InitInstance(hCurInst, szClassName, nCmdShow)) {
        exit_program(0);
	return FALSE;
    }

    
/*
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
*/
    hAccel = LoadAccelerators(hCurInst, "MYACCEL");
    while (GetMessage(&msg, NULL, 0, 0)) {
        if (!TranslateAccelerator(hParent, hAccel, &msg)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
    exit_program(0);
    return msg.wParam;    
#else
// ***************************************************************************
// ***************************************************************************
// ***************************************************************************
    ret=X68Sound_StartPcm( SampleRate[sample_rate] );
    if (ret != 0) {										// 戻り値が0以外は失敗
	if (ret == X68SNDERR_PCMOUT) {
//	    printf("X68Sound : PCMサウンドを出力できません。\n");
	} else if (ret == X68SNDERR_TIMER) {
//	    printf("X68Sound : マルチメディアタイマーが利用できません。\n");
	} else if (ret == X68SNDERR_MEMORY) {
//	    printf("X68Sound : メモリ確保に失敗しました。\n");
	} else {
//	    printf("謎のエラー\n");
	}
//	printf("X68Sound : ErrorCode = $%X\n", X68Sound_ErrorCode());
	exit(1);
    }
    sound_iocs_init();	// IOCS を初期化して使えるようにする
    _iocs_opmintst(L00014e);

//	elfsnd_write_file(0, 4, "elle-04.wav", 3890, 1);
//	elfsnd_write_file(0, 4, "elle04.wav", 3900, 1);
	int shantbl[] = {
		3400,
		3277,
		6600,//??
		3000,
		4500,
		3300,//??
		4200,
		3700,
		3800,
		4500,
		3000,
		4700
		};
		

	char filename[256];
	{
		int sel_game, music_no, millisec;
		sscanf(lpsCmdLine, "%d %d %d %s",&sel_game,&music_no,&millisec, filename);

		printf("Parameter error : %d %d %d %s\n",sel_game, music_no, millisec,filename);

		if (millisec <(100 *60*60) && millisec > 0) {
			elfsnd_write_file(sel_game, music_no, filename, millisec, 1);
		} else {
			elfsnd_write_file(0/*ELLE*/, 10/*MusNo.*/, "__ERROR__.wav"/*filename*/, /*4323*/10, 1/*44100Hz*/);	
		}
	} 


//	for(int i=0;i<12;i++) 
	{
		int i= 1;
//		sprintf(filename, "shan%02d.wav",i);
//		elfsnd_write_file(1, i, filename, shantbl[i]/*100*45*/, 1);
	}

//	elfsnd_write_file(0/*ELLE*/, 10/*MusNo.*/, "ELLE-J.wav"/*filename*/, 4323, 1/*44100Hz*/);	
/*
	for (int i=0;i<9;i++)
	{
		snd_mask = 1<<i;
		sprintf(filename, "elle01-%d.wav",i);
		elfsnd_write_file(0, 1, filename, 4485, 1);
		//sprintf(filename, "elle04-%d.wav",i);
		//elfsnd_write_file(0, 4, filename, 3893, 1);
	}
*/
		
	exit_program(0);
    return 0;
// ***************************************************************************
// ***************************************************************************
// ***************************************************************************
#endif
}


//ウィンドウ・クラスの登録

BOOL InitApp(HINSTANCE hInst, LPCSTR szClassName)
{
    WNDCLASS wc;
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WndProc;    //プロシージャ名
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = hInst;        //インスタンス
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
//    wc.hIcon = LoadIcon(hInst, "IDI_ELLE");
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
    wc.lpszMenuName = NULL;    //メニュー名
    wc.lpszClassName = (LPCSTR)szClassName;
    return (RegisterClass(&wc));
}

//ウィンドウの生成

BOOL InitInstance(HINSTANCE hInst, LPCSTR szClassName, int nCmdShow)
{
    HWND hWnd;

    hWnd = CreateWindow(szClassName,
            //"ミュージックドライバー　for　ＥＬＬＥ　Ｘ６８０００",    //タイトルバーにこの名前が表示されます
            "Elf MusicDriver for X68K / WIN32:Masami. 20010529",
//            "Elf MusicDriver for X68K / WIN32:Masami. for Syabu-san(20030712)",

	    //WS_OVERLAPPEDWINDOW    ,    //ウィンドウの種類
	    WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
            CW_USEDEFAULT,    //Ｘ座標
            CW_USEDEFAULT,    //Ｙ座標
            440,//CW_USEDEFAULT,    //幅
            60,//CW_USEDEFAULT,    //高さ
            NULL,    //親ウィンドウのハンドル、親を作るときはNULL
            NULL,    //メニューハンドル、クラスメニューを使うときはNULL
            hInst,    //インスタンスハンドル
            NULL);
    if (!hWnd)
        return FALSE;
    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);
    hParent = hWnd; //ウィンドウハンドルの保存

/*
    HMENU hMenu = GetSystemMenu(hWnd, FALSE);
    for (int  i = 0; i <= 5; i++)
        DeleteMenu(hMenu, 0, MF_BYPOSITION);
    AppendMenu(hMenu, MF_STRING, IDM_ELLE, "項目の追加だよ");
    DrawMenuBar(hWnd);
*/  


    return TRUE;
}


//ウィンドウプロシージャ

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp)
{
    static char str[2048];
    static char msg_str[256];
    int id/*, wh*/;
    HDC hdc;
    PAINTSTRUCT ps;

    HFONT hFont,hFontOld;

    POINT pt;
    HMENU hmenu, hSubmenu;


    tone_table	= current_data->tone_table;
    adpcm_data_adr	= current_data->adpcm_data_ptr;
	

    switch (msg) {
	case WM_COMMAND:
	    switch (LOWORD(wp)) {
                case IDM_END:
		    SendMessage(hWnd, WM_CLOSE, 0, 0L);
		    break;
		case IDM_ELLE:
		case IDM_SHAN:
		case IDM_TNSN:
		case IDM_DGK3:
		    switch (LOWORD(wp)){
	    		case IDM_ELLE:
			    sel_no = 0;
			    break;
			case IDM_SHAN:
			    sel_no = 1;
			    break;
			case IDM_TNSN:
			    sel_no = 2;
			    break;
			case IDM_DGK3:
			    sel_no = 3;
			    break;
		    }
		    current_data->stop();
		    SendMessage(hWnd, WM_COMMAND,(WPARAM)IDM_PLAY, 0L);
		    break;
		case IDM_PLAY:
		    current_data->stop();
		    current_data = elf_data[sel_no];
		    tone_table = current_data->tone_table;
		    adpcm_data_adr = current_data->adpcm_data_ptr;
		    current_data->play();
		    break;
		case IDM_STOP:
		    current_data->stop();
		    break;
		case IDM_PAUSE:
		    flg_pause = current_data->pause();
		    break;
		case IDM_FADEOUT:
		    current_data->fade_out();
		    break;
		case IDM_SMPL_RATE:
		    //if (flg_pause == FALSE) flg_pause = current_data->pause();
		    sample_rate = (sample_rate+1)%3;
		    X68Sound_Samprate(SampleRate[sample_rate]);
		    //flg_pause = current_data->pause();
		    break;
		case IDM_DISP_M:
		    flg_disp_mask = (flg_disp_mask) ? FALSE : TRUE;
		    break;
		case IDM_RESET:
		    current_data->stop();
		    flg_pause = (flg_pause) ? flg_pause : current_data->pause();
		    X68Sound_Reset();
		    work.init();
		    work_trk[0].init();
		    work_trk[1].init();
		    work_trk[2].init();
		    work_trk[3].init();
		    work_trk[4].init();
		    work_trk[5].init();
		    work_trk[6].init();
		    work_trk[7].init();
		    work_trk[8].init();
		    work_trk[9].init();
		    flg_pause = current_data->pause(); //pause解除
		    break;
		case IDM_PLAY_NEXT:
    		    current_data->next();
		    current_data->play();
		    break;
		case IDM_PLAY_PREV:
		    current_data->prev();
		    current_data->play();
		    break;
		case IDM_NEXT:
    		    current_data->next();
		    break;
		case IDM_PREV:
		    current_data->prev();
		    break;
		case IDM_PLAY_SLOW:
		    OPM_Write(0x12,
			(unsigned char)((current_data->get_tempo()*0x60/2+0x32)/0x64));
			break;
		case IDM_PLAY_FAST:
		    OPM_Write(0x12,
			(unsigned char)((0xf0*0x60+0x32)/0x64));
		    break;
		case IDM_NORM_TEMPO:
		    OPM_Write(0x12,
			(unsigned char)((current_data->get_tempo()*0x60+0x32)/0x64));
		    break;
		case IDM_MASK_ALL:
		    snd_mask = 0x000;
		    break;
		case IDM_UNMASK_ALL:
		    snd_mask = 0x1ff;
		    break;
		case IDM_GAME_NEXT:
    		    if ( elf_data[++sel_no] == NULL ) sel_no = 0;
		    SendMessage(hWnd, WM_COMMAND,(WPARAM)IDM_PLAY, 0L);
		    break;
		case IDM_GAME_PREV:
		    if (sel_no == 0) { while(elf_data[++sel_no] != NULL);}
		    sel_no--;
		    SendMessage(hWnd, WM_COMMAND,(WPARAM)IDM_PLAY, 0L);
		    break;
		case IDM_MASK_FM1:
		    snd_mask ^= 0x001;
		    break;
		case IDM_MASK_FM2:
		    snd_mask ^= 0x002;
		    break;
		case IDM_MASK_FM3:
		    snd_mask ^= 0x004;
		    break;
		case IDM_MASK_FM4:
		    snd_mask ^= 0x008;
		    break;
		case IDM_MASK_FM5:
		    snd_mask ^= 0x010;
		    break;
		case IDM_MASK_FM6:
		    snd_mask ^= 0x020;
		    break;
		case IDM_MASK_FM7:
		    snd_mask ^= 0x040;
		    break;
		case IDM_MASK_FM8:
		    snd_mask ^= 0x080;
		    break;
		case IDM_MASK_ADPCM:
		    snd_mask ^= 0x100;
		    break;
		default:
		    return (DefWindowProc(hWnd, msg, wp, lp));
		    break;
	    }
            InvalidateRect(hWnd, NULL, TRUE);
	    break;

        case WM_CHAR:
            if ( wp == 0x0D || /*wp == 0x08 ||*/ wp == 0x09 || wp == 0x1B) {
                return (DefWindowProc(hWnd, msg, wp, lp));
            }
/***************************************************************************/
	    if (wp >= '1' && wp <='9') {
		 snd_mask ^= (1 << (wp-'1'));
	    } else {
		switch (wp) {
		case 0x1b:  //ESC
//		    SendMessage(hWnd, WM_CLOSE, 0, 0L);
		    break;
		case 0x0d:  //ENTER
//		    SendMessage(hWnd, WM_COMMAND, (WPARAM)IDM_PLAY, 0L);
		    break;
		case ' ':   //SPC
		    SendMessage(hWnd, WM_COMMAND, (WPARAM)IDM_PLAY_NEXT, 0L);
		    break;
		case 0x08:  //BackSpace
		    SendMessage(hWnd, WM_COMMAND, (WPARAM)IDM_PLAY_PREV, 0L);
		    break;
		case '0':
		    SendMessage(hWnd, WM_COMMAND, (WPARAM)IDM_UNMASK_ALL, 0L);
		    break;
		case '-':
		    SendMessage(hWnd, WM_COMMAND, (WPARAM)IDM_MASK_ALL, 0L);
		    break;
		case 's':
		    SendMessage(hWnd, WM_COMMAND, (WPARAM)IDM_STOP, 0L);
		    break;
		case '^':
		    SendMessage(hWnd, WM_COMMAND, (WPARAM)IDM_FADEOUT, 0L);
		    break;
		case 'f':
		    SendMessage(hWnd, WM_COMMAND, (WPARAM)IDM_NEXT, 0L);
		    break;
		case 'b':
		    SendMessage(hWnd, WM_COMMAND, (WPARAM)IDM_PREV, 0L);		    
		    break;
		case 'p':
		    SendMessage(hWnd, WM_COMMAND, (WPARAM)IDM_PAUSE, 0L);
		    break;
		case 'r':
		    SendMessage(hWnd, WM_COMMAND, (WPARAM)IDM_SMPL_RATE, 0L);
		    break;
		case 'm':
		    SendMessage(hWnd, WM_COMMAND, (WPARAM)IDM_DISP_M, 0L);		    
		    break;
		case 'n':
		    SendMessage(hWnd, WM_COMMAND, (WPARAM)IDM_GAME_NEXT, 0L);
		    break;
		case 'N':
		    SendMessage(hWnd, WM_COMMAND, (WPARAM)IDM_GAME_PREV, 0L);
		    break;
		case 't':
		    SendMessage(hWnd, WM_COMMAND, (WPARAM)IDM_NORM_TEMPO, 0L);
		    break;
		case 'T':
		    SendMessage(hWnd, WM_COMMAND, (WPARAM)IDM_PLAY_FAST, 0L);
		    break;
		case 'S':
		    SendMessage(hWnd, WM_COMMAND, (WPARAM)IDM_PLAY_SLOW, 0L);
		    break;
		case 'R':
		    SendMessage(hWnd, WM_COMMAND, (WPARAM)IDM_RESET, 0L);
		    break;
		
		default:
		    break;
		}

	    }
//	    wsprintf((LPTSTR)msg_str, (LPCTSTR)"$%02x %02d: %s\n", wp, snd_no, title[snd_no]);
/***************************************************************************/

//            wsprintf((LPTSTR)msg_str, (LPCTSTR)str_org, (int)wp);
//            strcat(str, msg_str);
            InvalidateRect(hWnd, NULL, TRUE);


            break;
	case WM_LBUTTONUP:
		current_data->next();
		current_data->play();
                InvalidateRect(hWnd, NULL, TRUE);

	    break;
/*	case WM_RBUTTONUP:
		current_data->prev();
		current_data->play();
                InvalidateRect(hWnd, NULL, TRUE);
*/
	case WM_RBUTTONDOWN:
	    {
		pt.x = LOWORD(lp);
		pt.y = HIWORD(lp);
		hmenu = LoadMenu((HINSTANCE)GetWindowLong(hWnd, GWL_HINSTANCE), "MYPOPUP");
		hSubmenu = GetSubMenu(hmenu, 0);
		ClientToScreen(hWnd, &pt);
		TrackPopupMenu(hSubmenu, TPM_LEFTALIGN, pt.x, pt.y, 0, hWnd, NULL);
		DestroyMenu(hmenu);
	    }
            break;
        
        case WM_PAINT:
/*            hdc = BeginPaint(hWnd, &ps);
            hT = DrawText(hdc, 
                (LPCTSTR)str, 
                strlen(str), 
                &(ps.rcPaint), 
                DT_WORDBREAK);
            EndPaint(hWnd, &ps);
            wh = ps.rcPaint.bottom - ps.rcPaint.top;
            if (hT > wh - 20)
                strcpy(str, "");
*/

            hdc = BeginPaint(hWnd, &ps);
            SetBkMode(hdc, TRANSPARENT);
/*
	    //曲名表示
	    wsprintf((LPTSTR)msg_str, 
		(LPCTSTR)"%02d: %s""          ""          ""          ""          ",
		snd_no, snd_lst[snd_no].name);
*/
	    wsprintf((LPTSTR)msg_str, 
		(LPCTSTR)" %s""          ""          ""          ""          ""          ",
		current_data->get_title());
            hFont = SetMyFont_bold(hdc, (LPCTSTR)"ＭＳ ゴシック", 16, 0);
            hFontOld = (HFONT)SelectObject(hdc, hFont);
            SetTextColor(hdc, RGB(32,32,160));
	    TextOut(hdc, 15, 18, (LPCTSTR)msg_str, 50);
            SelectObject(hdc, hFontOld);
            DeleteObject(hFont);

	    //ゲーム名表示
	    wsprintf((LPTSTR)msg_str, current_data->game_title);
            hFont = SetMyFont(hdc, (LPCTSTR)"ＭＳ ゴシック", 12, 0);
            hFontOld = (HFONT)SelectObject(hdc, hFont);
            SetTextColor(hdc, RGB(160,32,32));
	    TextOut(hdc, 10, 3, (LPCTSTR)msg_str, strlen(msg_str));
            SelectObject(hdc, hFontOld);
            DeleteObject(hFont);

	    //サンプリング周波数表示
	    wsprintf((LPTSTR)msg_str,"(%5dHz)",SampleRate[sample_rate]);
            hFont = SetMyFont(hdc, (LPCTSTR)"ＭＳ ゴシック", 10, 0);
            hFontOld = (HFONT)SelectObject(hdc, hFont);
            SetTextColor(hdc, RGB(32,160,32));
	    TextOut(hdc, 380, 3, (LPCTSTR)msg_str, strlen(msg_str));
            SelectObject(hdc, hFontOld);
            DeleteObject(hFont);

	
	    //Mask & PAUSEの表示

	    wsprintf((LPTSTR)msg_str,(flg_disp_mask == TRUE) ? "123456789 %c" : "          %c",(flg_pause == TRUE) ? 'P' : ' ');
	    hFont = SetMyFont(hdc, (LPCTSTR)"ＭＳ ゴシック", 10, 0);
	    hFontOld = (HFONT)SelectObject(hdc, hFont);
	    SetTextColor(hdc, RGB(160,160,32));
	    TextOut(hdc, 320, 0, (LPCTSTR)msg_str, strlen(msg_str));
	    SelectObject(hdc, hFontOld);
	    DeleteObject(hFont);
	    if (flg_disp_mask == TRUE) {
		int i;
		char  c = '-';
		wsprintf((LPTSTR)msg_str,"           ");
		for (i=0;i<9;i++) {
		    if (snd_mask & (1 << i)) {
			if (work_trk[i].I05 != 0x00 ) {
			    c = '.';
			} else {
			    c = '*';
			}
		    } else {
			c = '-';
		    }
		    *(msg_str+i) = c;
		}
		hFont = SetMyFont(hdc, (LPCTSTR)"ＭＳ ゴシック", 10, 0);
		hFontOld = (HFONT)SelectObject(hdc, hFont);
		SetTextColor(hdc, RGB(160,160,32));
		TextOut(hdc, 320, 8, (LPCTSTR)msg_str, strlen(msg_str));
		SelectObject(hdc, hFontOld);
		DeleteObject(hFont);
	    }
	    break;
	case WM_CREATE:
            current_data->play();
	    break;
        case WM_CLOSE:
            id = MessageBox(hWnd,
                (LPCSTR)"終了しますか？",
                (LPCSTR)"終了確認",
                MB_YESNO | MB_ICONQUESTION);
            if (id == IDYES) {
                DestroyWindow(hWnd);
            }
            break;
        case WM_DESTROY:
            exit_program(0);	//X68Sound.dll
	    PostQuitMessage(0);
            break;
        default:
            return (DefWindowProc(hWnd, msg, wp, lp));
    }
    return 0L;
}


HFONT SetMyFont(HDC hdc, LPCTSTR face, int h, int angle)
{
    HFONT hFont;
    hFont = CreateFont(h,    //フォント高さ
        0,                    //文字幅
        angle,                    //テキストの角度
        0,                    //ベースラインとｘ軸との角度
        FW_REGULAR,            //フォントの重さ（太さ）
        FALSE,                //イタリック体
        FALSE,                //アンダーライン
        FALSE,                //打ち消し線
        SHIFTJIS_CHARSET,    //文字セット
        OUT_DEFAULT_PRECIS,    //出力精度
        CLIP_DEFAULT_PRECIS,//クリッピング精度
        PROOF_QUALITY,        //出力品質
        FIXED_PITCH | FF_MODERN,//ピッチとファミリー
        face);    //書体名
    return hFont;
}
HFONT SetMyFont_bold(HDC hdc, LPCTSTR face, int h, int angle)
{
    HFONT hFont;
    hFont = CreateFont(h,    //フォント高さ
        0,                    //文字幅
        angle,                    //テキストの角度
        0,                    //ベースラインとｘ軸との角度
        FW_BOLD, //FW_REGULAR,            //フォントの重さ（太さ）
        FALSE,                //イタリック体
        FALSE,                //アンダーライン
        FALSE,                //打ち消し線
        SHIFTJIS_CHARSET,    //文字セット
        OUT_DEFAULT_PRECIS,    //出力精度
        CLIP_DEFAULT_PRECIS,//クリッピング精度
        PROOF_QUALITY,        //出力品質
        FIXED_PITCH | FF_MODERN,//ピッチとファミリー
        face);    //書体名
    return hFont;
}


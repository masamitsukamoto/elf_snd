#include	<stdio.h>
#include	<conio.h>

#include	"sound_iocs.h"
#include	"x68sound.h"

// 16bit�l�̃o�C�g�̕��т��t�ɂ��ĕԂ�
unsigned short bswapw(unsigned short data) {
	__asm {
		mov ax,data
		ror ax,8
	}
}

// 32bit�l�̃o�C�g�̕��т��t�ɂ��ĕԂ�
void *bswapl(void *adrs) {
	__asm {
		mov	eax,adrs
		bswap	eax
	}
}


volatile unsigned char AdpcmStat=0; // $02:adpcmout $12:adpcmaot $22:adpcmlot $32:adpcmcot
volatile unsigned char OpmReg1B=0;  // OPM ���W�X�^ $1B �̓��e
volatile unsigned char DmaErrCode = 0;

volatile unsigned char *Adpcmcot_adrs;
volatile int	Adpcmcot_len;

// OPM��BUSY�҂�
void OpmWait() {
	while (X68Sound_OpmPeek() & 0x80);
}

// IOCS _OPMSET ($68) �̏���
// [����]
//   int addr : OPM���W�X�^�i���o�[(0�`255)
//   int data : �f�[�^(0�`255)
void _iocs_opmset(int addr, int data) {
	if (addr == 0x1B) {
		OpmReg1B = (OpmReg1B&0xC0)|(data&0x3F);
		data = OpmReg1B;
	}
	OpmWait();
	X68Sound_OpmReg(addr);
	OpmWait();
	X68Sound_OpmPoke(data);
}

// IOCS _OPMSNS ($69) �̏���
// [�߂�l]
//   bit 0 : �^�C�}�[A�I�[�o�[�t���[�̂Ƃ�1�ɂȂ�
//   bit 1 : �^�C�}�[B�I�[�o�[�t���[�̂Ƃ�1�ɂȂ�
//   bit 7 : 0�Ȃ�΃f�[�^�������݉\
int _iocs_opmsns() {
	return X68Sound_OpmPeek();
}

void (CALLBACK *OpmIntProc)()=NULL;		// OPM�̃^�C�}�[���荞�ݏ����A�h���X

// IOCS _OPMINTST ($6A) �̏���
// [����]
//   void *addr : ���荞�ݏ����A�h���X
//                0�̂Ƃ��͊��荞�݋֎~
// [�߂�l]
//   ���荞�݂��ݒ肳�ꂽ�ꍇ�� 0
//   ���Ɋ��荞�݂��ݒ肳��Ă���ꍇ�͂��̊��荞�ݏ����A�h���X��Ԃ�
int _iocs_opmintst(void *addr) {
	if (addr == 0) {				// ������0�̎��͊��荞�݂��֎~����
		OpmIntProc = NULL;
		X68Sound_OpmInt(OpmIntProc);
		return 0;
	}
	if (OpmIntProc != NULL) {		// ���ɐݒ肳��Ă���ꍇ�́A���̏����A�h���X��Ԃ�
		return (int)OpmIntProc;
	}
	OpmIntProc = (void (CALLBACK *)())addr;
	X68Sound_OpmInt(OpmIntProc);	// OPM�̊��荞�ݏ����A�h���X��ݒ�
	return 0;
}

// DMA�]���I�����荞�ݏ������[�`��
void CALLBACK DmaIntProc() {
	if (AdpcmStat == 0x32 && (X68Sound_DmaPeek(0x00)&0x40)!=0) {	// �R���e�B�j���[���[�h���̏���
		X68Sound_DmaPoke(0x00, 0x40);	// BTC�r�b�g���N���A
		if (Adpcmcot_len > 0) {
			int dmalen;
			dmalen = Adpcmcot_len;
			if (dmalen > 0xFF00) {	// 1�x�ɓ]���ł���o�C�g����0xFF00
				dmalen = 0xFF00;
			}
			X68Sound_DmaPokeL(0x1C, Adpcmcot_adrs);	// BAR�Ɏ���DMA�]���A�h���X���Z�b�g
			X68Sound_DmaPokeW(0x1A, dmalen);	// BTC�Ɏ���DMA�]���o�C�g�����Z�b�g
			Adpcmcot_adrs += dmalen;
			Adpcmcot_len -= dmalen;

			X68Sound_DmaPoke(0x07, 0x48);	// �R���e�B�j���[�I�y���[�V�����ݒ�
		}
		return;
	}
	if (!(AdpcmStat&0x80)) {
		X68Sound_PpiCtrl(0x01);	// ADPCM�E�o��OFF
		X68Sound_PpiCtrl(0x03);	// ADPCM���o��OFF
		X68Sound_AdpcmPoke(0x01);	// ADPCM�Đ������~
	}
	AdpcmStat = 0;
	X68Sound_DmaPoke(0x00, 0xFF);	// DMA CSR �̑S�r�b�g���N���A
}

// DMA�G���[���荞�ݏ������[�`��
void CALLBACK DmaErrIntProc() {
	DmaErrCode = X68Sound_DmaPeek(0x01);	// �G���[�R�[�h�� DmaErrCode �ɕۑ�

	X68Sound_PpiCtrl(0x01);	// ADPCM�E�o��OFF
	X68Sound_PpiCtrl(0x03);	// ADPCM���o��OFF
	X68Sound_AdpcmPoke(0x01);	// ADPCM�Đ������~

	AdpcmStat = 0;
	X68Sound_DmaPoke(0x00, 0xFF);	// DMA CSR �̑S�r�b�g���N���A
}

unsigned char	PANTBL[4] = {3, 1, 2, 0};

// �T���v�����O���g����PAN��ݒ肵��DMA�]�����J�n���郋�[�`��
// [����]
//   unsigned short mode : �T���v�����O���g��*256+PAN
//   unsigned char ccr : DMA CCR �ɏ������ރf�[�^
void SetAdpcmMode(unsigned short mode, unsigned char ccr) {
	if (mode >= 0x0200) {
		mode -= 0x0200;
		OpmReg1B &= 0x7F;	// ADPCM�̃N���b�N��8MHz
	} else {
		OpmReg1B |= 0x80;	// ADPCM�̃N���b�N��4MHz
	}
	OpmWait();
	X68Sound_OpmReg(0x1B);
	OpmWait();
	X68Sound_OpmPoke(OpmReg1B);	// ADPCM�̃N���b�N�ݒ�(8or4MHz)
	unsigned char ppireg;
	ppireg = ((mode>>6)&0x0C) | PANTBL[mode&3];
	ppireg |= (X68Sound_PpiPeek()&0xF0);
	X68Sound_DmaPoke(0x07, ccr);	// DMA�]���J�n
	X68Sound_PpiPoke(ppireg);	// �T���v�����O���[�g��PAN��PPI�ɐݒ�
}

// _iocs_adpcmout�̃��C�����[�`��
// [����]
//   unsigned char stat : ADPCM���~�������ɑ�����DMA�]�����s���ꍇ��$80
//                        DMA�]���I����ADPCM���~������ꍇ��$00
//   unsigned short len : DMA�]���o�C�g��
//   unsigned char *adrs : DMA�]���A�h���X
void AdpcmoutMain(unsigned char stat, unsigned short mode, unsigned short len, unsigned char *adrs) {
	while (AdpcmStat);	// DMA�]���I���҂�
	AdpcmStat = stat+2;
	X68Sound_DmaPoke(0x05, 0x32);	// DMA OCR ���`�F�C������Ȃ��ɐݒ�

	X68Sound_DmaPoke(0x00, 0xFF);	// DMA CSR �̑S�r�b�g���N���A
	X68Sound_DmaPokeL(0x0C, adrs);	// DMA MAR ��DMA�]���A�h���X���Z�b�g
	X68Sound_DmaPokeW(0x0A, len);	// DMA MTC ��DMA�]���o�C�g�����Z�b�g
	SetAdpcmMode(mode, 0x88);	// �T���v�����O���g����PAN��ݒ肵��DMA�]���J�n

	X68Sound_AdpcmPoke(0x02);	// ADPCM�Đ��J�n
}

// IOCS _ADPCMOUT ($60) �̏���
// [����]
//   void *addr : ADPCM�f�[�^�A�h���X
//   int mode : �T���v�����O���g��(0�`4)*256+PAN(0�`3)
//   int len : ADPCM�f�[�^�̃o�C�g��
void _iocs_adpcmout(void *addr, int mode, int len) {
	int dmalen;
	unsigned char *dmaadrs = (unsigned char *)addr;
	while (AdpcmStat);	// DMA�]���I���҂�
	while (len > 0x0000FF00) {	// ADPCM�f�[�^��0xFF00�o�C�g�ȏ�̏ꍇ��
		dmalen = 0x0000FF00;	// 0xFF00�o�C�g��������ɕ�����DMA�]�����s��
		AdpcmoutMain(0x80,mode,dmalen,dmaadrs);
		dmaadrs += dmalen;
		len -= dmalen;
	}
	AdpcmoutMain(0x00,mode,len,dmaadrs);
}

// IOCS _ADPCMAOT ($62) �̏���
// [����]
//   struct _chain *tbl : �A���C�`�F�C���e�[�u���̃A�h���X
//   int mode : �T���v�����O���g��(0�`4)*256+PAN(0�`3)
//   int cnt : �A���C�`�F�C���e�[�u���̃u���b�N��
void _iocs_adpcmaot(struct _chain *tbl, int mode, int cnt) {
	while (AdpcmStat);	// DMA�]���I���҂�

	AdpcmStat = 0x12;
	X68Sound_DmaPoke(0x05, 0x3A);	// DMA OCR ���A���C�`�F�C������ɐݒ�

	X68Sound_DmaPoke(0x00, 0xFF);	// DMA CSR �̑S�r�b�g���N���A
	X68Sound_DmaPokeL(0x1C, tbl);	// DMA BAR �ɃA���C�`�F�C���e�[�u���A�h���X���Z�b�g
	X68Sound_DmaPokeW(0x1A, cnt);	// DMA BTC �ɃA���C�`�F�C���e�[�u���̌����Z�b�g
	SetAdpcmMode(mode, 0x88);	// �T���v�����O���g����PAN��ݒ肵��DMA�]���J�n

	X68Sound_AdpcmPoke(0x02);	// ADPCM�Đ��J�n
}

// IOCS _ADPCMAOT ($64) �̏���
// [����]
//   struct _chain2 *tbl : �����N�A���C�`�F�C���e�[�u���̃A�h���X
//   int mode : �T���v�����O���g��(0�`4)*256+PAN(0�`3)
void _iocs_adpcmlot(struct _chain2 *tbl, int mode) {
	while (AdpcmStat);	// DMA�]���I���҂�

	AdpcmStat = 0x22;
	X68Sound_DmaPoke(0x05, 0x3E);	// DMA OCR �������N�A���C�`�F�C������ɐݒ�

	X68Sound_DmaPoke(0x00, 0xFF);	// DMA CSR �̑S�r�b�g���N���A
	X68Sound_DmaPokeL(0x1C, tbl);	// DMA BAR �Ƀ����N�A���C�`�F�C���e�[�u���A�h���X���Z�b�g
	SetAdpcmMode(mode, 0x88);	// �T���v�����O���g����PAN��ݒ肵��DMA�]���J�n

	X68Sound_AdpcmPoke(0x02);	// ADPCM�Đ��J�n
}


// �R���e�B�j���[���[�h�𗘗p����ADPCM�o�͂��s���T���v��
// IOCS _ADPCMOUT �Ɠ����������s�����A�f�[�^�o�C�g����0xFF00�o�C�g�ȏ�ł�
// �����Ƀ��^�[������B
// [����]
//   void *addr : ADPCM�f�[�^�A�h���X
//   int mode : �T���v�����O���g��(0�`4)*256+PAN(0�`3)
//   int len : ADPCM�f�[�^�̃o�C�g��
void _iocs_adpcmcot(void *addr, int mode, int len) {
	int dmalen;
	Adpcmcot_adrs = (unsigned char *)addr;
	Adpcmcot_len = len;
	while (AdpcmStat);	// DMA�]���I���҂�
	AdpcmStat = 0x32;

	X68Sound_DmaPoke(0x05, 0x32);	// DMA OCR ���`�F�C������Ȃ��ɐݒ�

	dmalen = Adpcmcot_len;
	if (dmalen > 0xFF00) {	// ADPCM�f�[�^��0xFF00�o�C�g�ȏ�̏ꍇ��
		dmalen = 0xFF00;	// 0xFF00�o�C�g��������ɕ�����DMA�]�����s��
	}

	X68Sound_DmaPoke(0x00, 0xFF);	// DMA CSR �̑S�r�b�g���N���A
	X68Sound_DmaPokeL(0x0C, Adpcmcot_adrs);	// DMA MAR ��DMA�]���A�h���X���Z�b�g
	X68Sound_DmaPokeW(0x0A, dmalen);	// DMA MTC ��DMA�]���o�C�g�����Z�b�g
	Adpcmcot_adrs += dmalen;
	Adpcmcot_len -= dmalen;
	if (Adpcmcot_len <= 0) {
		SetAdpcmMode(mode, 0x88);	// �f�[�^�o�C�g����0xFF00�ȉ��̏ꍇ�͒ʏ�]��
	} else {
		dmalen = Adpcmcot_len;
		if (dmalen > 0xFF00) {
			dmalen = 0xFF00;
		}
		X68Sound_DmaPokeL(0x1C, Adpcmcot_adrs);	// BAR�Ɏ���DMA�]���A�h���X���Z�b�g
		X68Sound_DmaPokeW(0x1A, dmalen);	// BTC�Ɏ���DMA�]���o�C�g�����Z�b�g
		Adpcmcot_adrs += dmalen;
		Adpcmcot_len -= dmalen;
		SetAdpcmMode(mode, 0xC8);	// DMA CNT�r�b�g��1�ɂ���DMA�]���J�n
	}

	X68Sound_AdpcmPoke(0x02);	// ADPCM�Đ��J�n
}

// IOCS _ADPCMSNS ($66) �̏���
// [�߂�l]
//   0 : �������Ă��Ȃ�
//   $02 : _iocs_adpcmout �ŏo�͒�
//   $12 : _iocs_adpcmaot �ŏo�͒�
//   $22 : _iocs_adpcmlot �ŏo�͒�
//   $32 : _iocs_adpcmcot �ŏo�͒�
int _iocs_adpcmsns() {
	return (AdpcmStat&0x7F);
}

// IOCS _ADPCMMOD ($67) �̏���
// [����]
//   0 : ADPCM�Đ� �I��
//   1 : ADPCM�Đ� �ꎞ��~
//   2 : ADPCM�Đ� �ĊJ
void _iocs_adpcmmod(int mode) {
	switch (mode) {
	case 0:
		AdpcmStat = 0;
		X68Sound_PpiCtrl(0x01);	// ADPCM�E�o��OFF
		X68Sound_PpiCtrl(0x03);	// ADPCM���o��OFF
		X68Sound_AdpcmPoke(0x01);	// ADPCM�Đ������~
		X68Sound_DmaPoke(0x07, 0x10);	// DMA SAB=1 (�\�t�g�E�F�A�A�{�[�g)
		break;
	case 1:
		X68Sound_DmaPoke(0x07, 0x20);	// DMA HLT=1 (�z���g�I�y���[�V����)
		break;
	case 2:
		X68Sound_DmaPoke(0x07, 0x08);	// DMA HLT=0 (�z���g�I�y���[�V��������)
		break;
	}
}


// IOCS�R�[���̏�����
// DMA�̊��荞�݂�ݒ肷��
void sound_iocs_init() {
	X68Sound_DmaInt(DmaIntProc);
	X68Sound_DmaErrInt(DmaErrIntProc);
}

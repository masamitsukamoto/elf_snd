unsigned short bswapw(unsigned short data);
void *bswapl(void *adrs);

// �A���C�`�F�C���e�[�u���\���̂��`
// �\���̂̃A���C�����g��2�ɂ���
#pragma pack(2)
struct _chain {
	void *addr;
	unsigned short len;
};

// �����N�A���C�`�F�C���e�[�u���\���̂��`
// �\���̂̃A���C�����g��2�ɂ���
#pragma pack(2)
struct _chain2 {
	void *addr;
	unsigned short len;
	struct _chain2 *next;
};


void sound_iocs_init();

void _iocs_opmset(int addr, int data);
int _iocs_opmsns();
int _iocs_opmintst(void *addr);
void _iocs_adpcmout(void *addr, int mode, int len);
void _iocs_adpcmaot(struct _chain *tbl, int mode, int cnt);
void _iocs_adpcmlot(struct _chain2 *tbl, int mode);
void _iocs_adpcmcot(void *addr, int mode, int len);
int _iocs_adpcmsns();
void _iocs_adpcmmod(int mode);

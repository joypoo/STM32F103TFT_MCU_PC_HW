// CommMFCDlg.cpp : ʵ���ļ�
//
/*
16λ���ɫ��ͼ����ͼ��ͷ���ݺ�ͼ�����ݹ��ɣ�

ͼ��ͷ����ͷ���� : typedef struct _HEADCOLOR
{
    unsigned char scan;
    unsigned char gray;
    unsigned short w;
    unsigned short h;
    unsigned char is565;
    unsigned char rgb;
} HEADCOLOR;

scan : ɨ��ģʽ����������Ϊ0x00
Bit7 : 0 : ��������ɨ�裬1 : ��������ɨ�衣
Bit6 : 0 : �Զ�����ɨ�裬1 : �Ե�����ɨ�衣
Bit5 : 0 : �ֽ����������ݴӸ�λ����λ���У�1 : �ֽ����������ݴӵ�λ����λ���С�
Bit4 : 0 : WORD���͸ߵ�λ�ֽ�˳����PC��ͬ��1 : WORD���͸ߵ�λ�ֽ�˳����PC�෴��
Bit3 ~2 : ������
Bit1 ~0 : [00] ˮƽɨ�裬[01] ��ֱɨ�裬[10] ����ˮƽ,
�ֽڴ�ֱ��[11] ���ݴ�ֱ,
�ֽ�ˮƽ��

gray : �Ҷ�ֵ����������Ϊ0x10
�Ҷ�ֵ��1 : ��ɫ��2 : �Ļң�4 : ʮ���ң�8 : 256ɫ��12 : 4096ɫ��16 : 16λ��ɫ��24 : 24λ��ɫ��32 : 32λ��ɫ��

w : ͼ��Ŀ�ȡ�

h : ͼ��ĸ߶ȡ�

is565 : Ϊ0��ʾRGB��ɫ������ռ�õ�λ����Ϊ5Bits��Ϊ1��ʾRGB��ɫ������ռ�õ�λ���ֱ�Ϊ5Bits,6Bits,5Bits��

rgb : ����RGB��ɫ����������˳��rgb��ÿ2Bits��ʾһ����ɫ������[00] ��ʾ�հף�[01] ��ʾRed��[10] ��ʾGreen��[11] ��ʾBlue��
*/
#include "stdafx.h"
#include "CommMFC.h"
#include "CommMFCDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// ����Ӧ�ó��򡰹��ڡ��˵���� CAboutDlg �Ի���

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// �Ի�������
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

// ʵ��
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()


// CCommMFCDlg �Ի���




CCommMFCDlg::CCommMFCDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CCommMFCDlg::IDD, pParent)
	, m_ReceiveTimeoutMS(0)
    , nPicNo(0)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CCommMFCDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_COMBO_PORT_Nr, m_PortNr);
    DDX_Control(pDX, IDC_COMBO_BAUDRATE, m_BaudRate);
    DDX_Control(pDX, IDC_BUTTON_OPEN_CLOSE, m_OpenCloseCtrl);
    DDX_Control(pDX, IDC_SendEdit, m_Send);
    DDX_Control(pDX, IDC_ReceiveEdit, m_ReceiveCtrl);
    DDX_Control(pDX, IDC_STATIC_RECV_COUNT_VALUE, m_recvCountCtrl);
    DDX_Control(pDX, IDC_STATIC_SEND_COUNT_VALUE, m_sendCountCtrl);
    DDX_Control(pDX, IDC_COMBO_PARITY, m_Parity);
    DDX_Control(pDX, IDC_COMBO_STOP, m_Stop);
    DDX_Control(pDX, IDC_COMBO_DATABITS, m_DataBits);
    DDX_Text(pDX, IDC_EDIT_RECEIVE_TIMEOUT_MS, m_ReceiveTimeoutMS);
    DDV_MinMaxUInt(pDX, m_ReceiveTimeoutMS, 0, 999999);
    DDX_Control(pDX, IDC_EDIT_RECEIVE_TIMEOUT_MS, m_ReceiveTimeoutMSCtrl);
    DDX_Control(pDX, IDC_CHECK_DTR, m_dtrCtrl);
    DDX_Control(pDX, IDC_CHECK_RTS, m_rtsCtrl);
    DDX_Control(pDX, IDC_COMBO_PIC_NO, m_PicNo);
    DDX_CBIndex(pDX, IDC_COMBO_PIC_NO, nPicNo);
    // DDV_MinMaxInt(pDX, nPicNo, 1, 100);
    DDX_Control(pDX, IDC_EDIT_IMPORTFILE, m_ctlEditImportFile);
}

BEGIN_MESSAGE_MAP(CCommMFCDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_OPEN_CLOSE, &CCommMFCDlg::OnBnClickedButtonOpenClose)
	ON_BN_CLICKED(IDC_BUTTON_SEND, &CCommMFCDlg::OnBnClickedButtonSend)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_BUTTON_CLEAR, &CCommMFCDlg::OnBnClickedButtonClear)
    ON_BN_CLICKED(IDC_CHECK_DTR, &CCommMFCDlg::OnBnClickedCheckDtr)
    ON_BN_CLICKED(IDC_CHECK_RTS, &CCommMFCDlg::OnBnClickedCheckRts)
    ON_BN_CLICKED(IDC_BUTTON_IMPORT, &CCommMFCDlg::OnBnClickedButtonImport)
    END_MESSAGE_MAP()


// CCommMFCDlg ��Ϣ�������

BOOL CCommMFCDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// ��������...���˵�����ӵ�ϵͳ�˵��С�

	// IDM_ABOUTBOX ������ϵͳ���Χ�ڡ�
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// ���ô˶Ի����ͼ�ꡣ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�
	//  ִ�д˲���
	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
	SetIcon(m_hIcon, FALSE);		// ����Сͼ��

	// TODO: �ڴ���Ӷ���ĳ�ʼ������

	rx = 0;
	tx = 0;
	m_recvCountCtrl.SetWindowText(CString("0"));
	m_sendCountCtrl.SetWindowText(CString("0"));

	// Ĭ�Ͻ��ճ�ʱʱ��(����)
	m_ReceiveTimeoutMSCtrl.SetWindowText(_T("50"));

	CString temp;
	//��Ӳ����ʵ������б�
    int BaudRateArray[] = {300, 600, 1200, 2400, 4800, 9600, 14400, 19200, 38400, 56000, 57600, 115200};
	for (int i = 0; i < sizeof(BaudRateArray) / sizeof(int); i++)
	{
		temp.Format(_T("%d"), BaudRateArray[i]);
		m_BaudRate.InsertString(i, temp);
	}

	temp.Format(_T("%d"), 57600);
	m_BaudRate.SetCurSel(m_BaudRate.FindString(0, temp));

	//У��λ
    std::string ParityArray[] = {"None", "Odd", "Even", "Mark", "Space"};
	for (int i = 0; i < sizeof(ParityArray) / sizeof(std::string); i++)
	{
#ifdef UNICODE
		temp.Format(_T("%S"), ParityArray[i].c_str());
#else
		temp.Format(_T("%s"), ParityArray[i].c_str());
#endif
		m_Parity.InsertString(i, temp);
	}
	m_Parity.SetCurSel(0);

	//����λ
    std::string DataBitsArray[] = {"5", "6", "7", "8"};
	for (int i = 0; i < sizeof(DataBitsArray) / sizeof(std::string); i++)
	{
#ifdef UNICODE
		temp.Format(_T("%S"), DataBitsArray[i].c_str());
#else
		temp.Format(_T("%s"), DataBitsArray[i].c_str());
#endif
		m_DataBits.InsertString(i, temp);
	}
	m_DataBits.SetCurSel(3);

	//ֹͣλ
    std::string StopArray[] = {"1", "1.5", "2"};
	for (int i = 0; i < sizeof(StopArray) / sizeof(std::string); i++)
	{
#ifdef UNICODE
		temp.Format(_T("%S"), StopArray[i].c_str());
#else
		temp.Format(_T("%s"), StopArray[i].c_str());
#endif
		m_Stop.InsertString(i, temp);
	}
	m_Stop.SetCurSel(0);

	//��ȡ���ں�
	std::vector<SerialPortInfo> m_portsList = CSerialPortInfo::availablePortInfos();
	TCHAR m_regKeyValue[256];
	for (size_t i = 0; i < m_portsList.size(); i++)
	{
#ifdef UNICODE
		int iLength;
		const char * _char = m_portsList[i].portName;
		iLength = MultiByteToWideChar(CP_ACP, 0, _char, strlen(_char) + 1, NULL, 0);
		MultiByteToWideChar(CP_ACP, 0, _char, strlen(_char) + 1, m_regKeyValue, iLength);
#else
		strcpy_s(m_regKeyValue, 256, m_portsList[i].portName);
#endif
		m_PortNr.AddString(m_regKeyValue);
	}
	m_PortNr.SetCurSel(0);
	
	//OnBnClickedButtonOpenClose();

	m_Send.SetWindowText(_T("ZXI_FLASH_CHK"));

	m_SerialPort.connectReadEvent(this);

	CString strPicNo;
	for (int i = 1; i < 101; i++)
    {
        strPicNo.Format("%03d", i);
        m_PicNo.InsertString(i - 1, strPicNo);
    }
	m_PicNo.SetCurSel(0);
	
	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
}

void CCommMFCDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// �����Ի��������С����ť������Ҫ����Ĵ���
//  �����Ƹ�ͼ�ꡣ����ʹ���ĵ�/��ͼģ�͵� MFC Ӧ�ó���
//  �⽫�ɿ���Զ���ɡ�

void CCommMFCDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // ���ڻ��Ƶ��豸������

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// ʹͼ���ڹ����������о���
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// ����ͼ��
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

//���û��϶���С������ʱϵͳ���ô˺���ȡ�ù��
//��ʾ��
HCURSOR CCommMFCDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CCommMFCDlg::OnBnClickedButtonOpenClose()
{
    if (m_SerialPort.isOpen())
	{
		m_SerialPort.close();
		m_OpenCloseCtrl.SetWindowText(_T("�򿪴���"));///���ð�ť����Ϊ"�򿪴���"
	}
	///�򿪴��ڲ���
	else if (m_PortNr.GetCount() > 0)///��ǰ�б�����ݸ���
	{
        char portName[256] = {0};
		int SelBaudRate;
		int SelParity;
		int SelDataBits;
		int SelStop;

		UpdateData(true);
        CString temp;
		m_PortNr.GetWindowText(temp);
#ifdef UNICODE
		strcpy_s(portName, 256, CW2A(temp.GetString()));
#else
		strcpy_s(portName, 256, temp.GetBuffer());
#endif	

		m_BaudRate.GetWindowText(temp);
		SelBaudRate = _tstoi(temp);	

		SelParity = m_Parity.GetCurSel();

		m_DataBits.GetWindowText(temp);
		SelDataBits = _tstoi(temp);

		SelStop = m_Stop.GetCurSel();

        m_SerialPort.setReadIntervalTimeout(m_ReceiveTimeoutMS);
        m_SerialPort.init(portName, SelBaudRate, itas109::Parity(SelParity), itas109::DataBits(SelDataBits), itas109::StopBits(SelStop));
		m_SerialPort.open();

		if (m_SerialPort.isOpen())
		{
			m_OpenCloseCtrl.SetWindowText(_T("�رմ���"));
		}
		else
		{
            m_OpenCloseCtrl.SetWindowText(_T("�򿪴���"));
			AfxMessageBox(_T("�����ѱ�ռ�ã�"));
		}
	}
	else
	{
		AfxMessageBox(_T("û�з��ִ��ڣ�"));
	}
}


void CCommMFCDlg::OnBnClickedButtonSend()
{
    //GetDlgItem(IDC_SendEdit)->SetFocus();
    if (!m_SerialPort.isOpen()) ///û�д򿪴���
	{
		AfxMessageBox(_T("�����ȴ򿪴���"));
		return;
	}

	CString temp;
	m_Send.GetWindowText(temp);
	int len = 0;
	char* m_str = NULL;
#ifdef UNICODE
	// ��������
	CStringA strA(temp);
	len = strA.GetLength();
	m_str = strA.GetBuffer();
#else
	len = temp.GetLength();
	m_str = temp.GetBuffer(0);
#endif
    //"ZXI_HEAD"+IMGNO+PKGNO+DATA ����ͼ��ͷ��Ϣ,IMGNOΪͼ���ţ�1-100��PKGNOΪ����ţ�0-129
	//"AXI_DATA"+IMGNO+PKGNO+DATA ����ͼ������

	//int nPicNo = 3; // ͼ���ţ���3��ʼ

	CString strFile = strImportFilePath; //
	//"C:\\Users\\Owner\\Documents\\Projects\\Mine\\others\\STM32F103_TFT\\doc\\006.bin";

    FILE *fp = fopen(strFile, "rb");
    int nFileSize;
    char *picBuf = new char[25600 + 256]; //[25600 + 14]; //160*80*2+8
    int numread = 0;

	if (fp == NULL) {
        AfxMessageBox("�����ļ�����");
        return;
	}
	else {
        CString str;
        
		fseek(fp, 0, SEEK_END);
        nFileSize = ftell(fp);
        fseek(fp, 0, SEEK_SET);
		picBuf = (char *)malloc(nFileSize);
        numread = fread(picBuf, sizeof(char), nFileSize, fp);

        //free(picBuf); ���free

		str.Format("%d - %d\n����ͼƬ......", nFileSize, numread);
        AfxMessageBox(str, MB_OK);
	}
    fclose(fp);

    char *m_data = new char[256]; //ÿ�η��͵����ݲ�����256
    
    int nPkgNo = 4;               // �������ţ���1��ʼ
    int nValidPkgSize = 200;      // ��Ч��ͼ�����С
    int nPkgCount;                // �ܰ���
	//-------------------
    m_str = "ZXI_HEAD";
    for (int i = 0; i < sizeof(m_str); i++){
        m_data[i] = m_str[i];
	}
    //int nIndex = m_PicNo.GetCurSel();
    CString strPicNo;
    m_PicNo.GetWindowText(strPicNo);
    nPicNo = atoi(strPicNo);

    m_data[8] = nPicNo + 2;     // ͼ���Ŵ�3��ʼ
    m_data[9] = 0x00;   //�����0
    m_data[10] = picBuf[2]; // ��MSB
    m_data[11] = picBuf[3]; // ��LSB
    m_data[12] = picBuf[4]; // ��MSB
    m_data[13] = picBuf[5]; // ��LSB
    len = sizeof(m_str) + 2 + 4;
    m_SerialPort.writeData(m_data, len);
    Sleep(400);

	//--------------------
	m_str = "ZXI_DATA";
    for (int i = 0; i < sizeof(m_str); i++){
        m_data[i] = m_str[i];
    }
	    
	nPkgCount = (nFileSize - 8) / nValidPkgSize;
    if ((nFileSize - 8) % nValidPkgSize) {
        nPkgCount += 1;
	}
 
    m_data[8] = nPicNo + 2; // ͼ����
    for (int nPkg = 0; nPkg < nPkgCount; nPkg++)
    {
        nPkgNo = nPkg + 1;
		m_data[9] = nPkgNo;     // �����1��ʼ
		for (int i = 0; i < nValidPkgSize; i++) {
			m_data[10 + i] = picBuf[nValidPkgSize * (nPkgNo - 1) + 8 + i];
		}
        len = sizeof(m_str) + 2 + nValidPkgSize;
        m_SerialPort.writeData(m_data, len);
        Sleep(400);
	}

	delete[] m_data;
	//free(picBuf);
    delete[] picBuf;

	tx += len;

	CString str2;
	str2.Format(_T("%d"), tx);
	m_sendCountCtrl.SetWindowText(str2);
}


void CCommMFCDlg::OnClose()
{
	m_SerialPort.close();
	CDialog::OnClose();
}

void CCommMFCDlg::onReadEvent(const char *portName, unsigned int readBufferLen)
{
    if (readBufferLen > 0)
    {
        char *data = new char[readBufferLen + 1]; // '\0'

		if (data)
        {
            int recLen = m_SerialPort.readData(data, readBufferLen);

            if (recLen > 0)
            {
                data[recLen] = '\0';

                CString str1(data);

                rx += str1.GetLength();

                if (str1.Find("ZXI>", 0) == 0)
                {
                    CTime time;
                    // int iLen;

                    time = CTime::GetCurrentTime();
                    CString curdata = time.Format("%Y-%m-%d %H:%M:%S");
                    // CString curdata = time.Format("%m-%d %H:%M:%S");
                    //CString curdata = time.Format("%H:%M:%S");
                   // curdata += ">";
                    curdata += str1;
                    m_ReceiveCtrl.SetSel(-1, -1);
                    m_ReceiveCtrl.ReplaceSel(curdata);
				}

                CString str2;
                str2.Format(_T("%d"), rx);
                m_recvCountCtrl.SetWindowText(str2);
            }

			delete[] data;
            data = NULL;
		}
	}
}


void CCommMFCDlg::OnBnClickedButtonClear()
{
	rx = 0;
	tx = 0;
	m_recvCountCtrl.SetWindowText(CString("0"));
	m_sendCountCtrl.SetWindowText(CString("0"));
}

void CCommMFCDlg::OnBnClickedCheckDtr()
{
    if(BST_CHECKED == m_dtrCtrl.GetCheck())
    {
        m_SerialPort.setDtr(true);
    }
    else
    {
        m_SerialPort.setDtr(false);
	}
}

void CCommMFCDlg::OnBnClickedCheckRts()
{
    if (BST_CHECKED == m_rtsCtrl.GetCheck())
    {
        m_SerialPort.setRts(true);
    }
    else
    {
        m_SerialPort.setRts(false);
    }
}

void CCommMFCDlg::OnBnClickedButtonImport()
{
    CFileDialog dlg(TRUE, NULL, NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
                    _T("Bin File(*.bin)|*.bin|All Files(*.*)|*.*||"), this);
    //dlg.m_ofn.lpstrFilter = "Bin Files (*.bin)|*.bin"; // �����ļ����͹�����
    //dlg.m_ofn.lpstrDefExt = "bin";               // ����Ĭ���ļ���չ��
    //dlg.m_ofn.lpstrInitialDir = _T(".\\");           // ����Ĭ���ļ�·��
    
	if (dlg.DoModal() == IDOK)
    {
        // �û������ȷ����ť
        strImportFilePath = dlg.GetPathName();
        m_ctlEditImportFile.SetWindowText(strImportFilePath); // SetDlgItemText(m_ctlEditImportFile.GetDlgCtrlID(), strImportFilePath);
    }
}

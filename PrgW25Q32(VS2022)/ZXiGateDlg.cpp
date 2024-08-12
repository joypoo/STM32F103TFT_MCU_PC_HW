// CommMFCDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "ZXiGate.h"
#include "ZXiGateDlg.h"
#include <string>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define RECEIVE_TIMER_EVENT 1

int BaudRateArray[] = { 300, 600, 1200, 2400, 4800, 9600, 14400, 19200, 38400, 56000, 57600, 115200 };

string ParityArray[] = { "None", "Odd", "Even", "Mark", "Space" };

string DataBitsArray[] = { "5", "6", "7","8" };

string StopArray[] = { "1", "1.5", "2"};

HANDLE m_hThread;
DWORD m_nThread;
DWORD WINAPI ThreadProc1(LPVOID lpParam);
void* pCommMFCDlg;

// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// 对话框数据
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
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


// CCommMFCDlg 对话框

CZXiGateDlg::CZXiGateDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CZXiGateDlg::IDD, pParent)
	, m_ReceiveTimeoutMS(0)
	, nPicNo(0)
	, m_SendInterval(0)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CZXiGateDlg::DoDataExchange(CDataExchange* pDX)
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
	DDX_Control(pDX, IDC_COMBO_PIC_NO, m_PicNo);
	DDX_Control(pDX, IDC_EDIT_IMPORTFILE, m_ctlEditImportFile);
	DDX_CBIndex(pDX, IDC_COMBO_PIC_NO, nPicNo);
	DDX_Control(pDX, IDC_EDIT_SEND_INTERVAL, m_SendIntervalCtrl);
	DDX_Text(pDX, IDC_EDIT_SEND_INTERVAL, m_SendInterval);
	DDV_MinMaxInt(pDX, m_SendInterval, 0, 999999);
}

BEGIN_MESSAGE_MAP(CZXiGateDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_OPEN_CLOSE, &CZXiGateDlg::OnBnClickedButtonOpenClose)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_BUTTON_CLEAR, &CZXiGateDlg::OnBnClickedButtonClear)
	//ON_WM_TIMER()

	//	ON_EN_CHANGE(IDC_EDIT_SNAP_Q, &CZXiGateDlg::OnEnChangeEditSnapQ)
	ON_BN_CLICKED(IDC_BUTTON_SEND, &CZXiGateDlg::OnBnClickedButtonSend)
	ON_BN_CLICKED(IDC_BUTTON_IMPORT, &CZXiGateDlg::OnBnClickedButtonImport)
	ON_WM_NCHITTEST()
END_MESSAGE_MAP()


// CCommMFCDlg 消息处理程序

BOOL CZXiGateDlg::OnInitDialog()
{
	CZXiGateApp* pApp = (CZXiGateApp*)AfxGetApp();

	CDialog::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。


	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadStringA(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码
    //顶部工具条
    
  //  if (!m_wndToolBar.CreateEx(this, 
		//	TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_ALIGN_TOP | CBRS_GRIPPER | CBRS_TOOLTIPS) || 
		//!m_wndToolBar.LoadToolBar(IDR_TOOLBAR)
		//)
  //  {
  //      TRACE0("Failed to create toolbar\n");
  //      return -1; // fail to create
  //  }
	CString strTitle = _T("MemStick ");
	strTitle += "V1.2.3";
	strTitle += "\r\n";
	strTitle += "Build: ";
	strTitle += __DATE__;
	strTitle += " ";
	strTitle += __TIME__;

	((CStatic*)GetDlgItem(IDC_STATIC_BUILD))->SetWindowText(strTitle);

	//CRect Rect;
    //m_wndToolBar.GetItemRect(0, &Rect);
    //m_wndToolBar.SetSizes(CSize(Rect.Width(), Rect.Height()), CSize(64, 64));
    m_wndToolBar.CreateEx(this, 
		TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_ALIGN_TOP | CBRS_GRIPPER | CBRS_TOOLTIPS);
    m_wndToolBar.LoadToolBar(IDR_TOOLBAR);
    //m_wndToolBar.ShowWindow(SW_SHOW);
    RepositionBars(AFX_IDW_CONTROLBAR_FIRST, AFX_IDW_CONTROLBAR_LAST, 0);

	m_wndToolBar.SetSizes(CSize(80, 80), CSize(64, 64));
	m_ImageList.Create(64, 64, ILC_COLOR24 | ILC_MASK, 1, 1);

	m_bitmap1.LoadBitmap(IDB_BITMAP1);
    m_bitmap2.LoadBitmap(IDB_BITMAP2);
    m_bitmap3.LoadBitmap(IDB_BITMAP3);
    m_bitmap4.LoadBitmap(IDB_BITMAP4);
    m_bitmap5.LoadBitmap(IDB_BITMAP5);
    m_bitmap6.LoadBitmap(IDB_BITMAP6);
    m_bitmap7.LoadBitmap(IDB_BITMAP7);
    m_bitmap8.LoadBitmap(IDB_BITMAP8);

    m_ImageList.Add(&m_bitmap1, RGB(255, 255, 255));   
	m_ImageList.Add(&m_bitmap2, RGB(255, 255, 255));
    m_ImageList.Add(&m_bitmap3, RGB(255, 255, 255));
    m_ImageList.Add(&m_bitmap4, RGB(255, 255, 255));
    m_ImageList.Add(&m_bitmap5, RGB(255, 255, 255));
    m_ImageList.Add(&m_bitmap6, RGB(255, 255, 255));
    m_ImageList.Add(&m_bitmap7, RGB(255, 255, 255));
    m_ImageList.Add(&m_bitmap8, RGB(255, 255, 255));

    m_bitmap1.DeleteObject();
	m_bitmap2.DeleteObject();
	m_bitmap3.DeleteObject();
    m_bitmap4.DeleteObject();
    m_bitmap5.DeleteObject();
    m_bitmap6.DeleteObject();
    m_bitmap7.DeleteObject();
    m_bitmap8.DeleteObject();

    m_wndToolBar.GetToolBarCtrl().SetImageList(&m_ImageList);

	rx = 0;
	tx = 0;
	m_recvCountCtrl.SetWindowText(CString("0"));
	m_sendCountCtrl.SetWindowText(CString("0"));

	//m_isTimerRunning = FALSE;

	// 默认接收超时时间(毫秒)
	m_ReceiveTimeoutMSCtrl.SetWindowText(_T("50"));
	m_SendIntervalCtrl.SetWindowText(_T("150"));
	CString temp;
	//添加波特率到下拉列表
	int BaudRateArray[] = { 300, 600, 1200, 2400, 4800, 9600, 14400, 19200, 38400, 56000, 57600, 115200 };
	for (int i = 0; i < sizeof(BaudRateArray) / sizeof(int); i++)
	{
		temp.Format(_T("%d"), BaudRateArray[i]);
		m_BaudRate.InsertString(i, temp);
	}

	temp.Format(_T("%d"), 115200);
	m_BaudRate.SetCurSel(m_BaudRate.FindString(0, temp));

	//校验位
	std::string ParityArray[] = { "None", "Odd", "Even", "Mark", "Space" };
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

	//数据位
	std::string DataBitsArray[] = { "5", "6", "7", "8" };
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

	//停止位
	std::string StopArray[] = { "1", "1.5", "2" };
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

	//获取串口号
	std::vector<SerialPortInfo> m_portsList = CSerialPortInfo::availablePortInfos();
	TCHAR m_regKeyValue[256];
	for (size_t i = 0; i < m_portsList.size(); i++)
	{
#ifdef UNICODE
		int iLength;
		const char* _char = m_portsList[i].portName;
		iLength = MultiByteToWideChar(CP_ACP, 0, _char, strlen(_char) + 1, NULL, 0);
		MultiByteToWideChar(CP_ACP, 0, _char, strlen(_char) + 1, m_regKeyValue, iLength);
#else
		strcpy_s(m_regKeyValue, 256, m_portsList[i].portName);
#endif
		m_PortNr.AddString(m_regKeyValue);
	}
	m_PortNr.SetCurSel(0);

	/////////////////////////////////////////////////
	
	OnBnClickedButtonOpenClose(); //2022-9-5 add,慎用，因为要准确确定串口号，除非能自适应获取。

	m_Send.SetWindowText(_T("ZXI_CTL55AAABCDCF55AA"));

	//m_SerialPort.readReady.connect(this, &CZXiGateDlg::OnReceive);
	m_SerialPort.connectReadEvent(this);

	//连接到MySQL
	pApp->strHostDatebase = "rm-bp1h2an8b2la4vw1q6o.mysql.rds.aliyuncs.com";
	pApp->strUserDatabase = "root";
	pApp->strPasswdDatabase = "root";
	pApp->strDbDatabase = "datadb";
	CString strConSuc = "[" + pApp->strHostDatebase + "]数据库连接成功！\r\n";
	if (pApp->ConnectDatabase()) {
		m_ReceiveCtrl.ReplaceSel("数据库连接失败！\r\n");
		//MessageBox("数据库连接失败！");
	}
	else {
		m_ReceiveCtrl.ReplaceSel(strConSuc);
	}

	CString strPicNo;
	for (int i = 1; i < 101; i++)
	{
		strPicNo.Format("%03d", i);
		m_PicNo.InsertString(i - 1, strPicNo);
	}
	m_PicNo.SetCurSel(0);

	pCommMFCDlg = this;
	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CZXiGateDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CZXiGateDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CZXiGateDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CZXiGateDlg::OnBnClickedButtonOpenClose()
{
	if (m_SerialPort.isOpen())
	{
		m_SerialPort.close();
		m_OpenCloseCtrl.SetWindowText(_T("打开串口"));///设置按钮文字为"打开串口"
	}
	///打开串口操作
	else if (m_PortNr.GetCount() > 0)///当前列表的内容个数
	{
		char portName[256] = { 0 };
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
			m_OpenCloseCtrl.SetWindowText(_T("关闭串口"));
		}
		else
		{
			m_OpenCloseCtrl.SetWindowText(_T("打开串口"));
			AfxMessageBox(_T("串口已被占用！"));
		}
	}
	else
	{
		AfxMessageBox(_T("没有发现串口！"));
	}
}

void CZXiGateDlg::OnClose()
{
	CZXiGateApp* pApp = (CZXiGateApp*)AfxGetApp();

	m_SerialPort.close();

	pApp->DisConnectDatabase();
	
	CDialog::OnClose();
}


void CZXiGateDlg::Split(CString source, CStringArray& dest, CString division)
{
	dest.RemoveAll();
	int pos = 0;
	int pre_pos = 0;
	while (-1 != pos) {
		pre_pos = pos;
		pos = source.Find(division, (pos + 1));
		dest.Add(source.Mid(pre_pos, (pos - pre_pos)));
	}
}

void CZXiGateDlg::OnBnClickedButtonClear()
{
	rx = 0;
	tx = 0;
	m_recvCountCtrl.SetWindowText(CString("0"));
	m_sendCountCtrl.SetWindowText(CString("0"));
}

//void CZXiGateDlg::OnTimer(UINT_PTR nIDEvent)
//{
//	switch (nIDEvent)
//	{
//	case RECEIVE_TIMER_EVENT:
//		KillTimer(RECEIVE_TIMER_EVENT);
//		OnReceiveBusiness();
//		break;
//	default:
//		break;
//	}
//
//	__super::OnTimer(nIDEvent);
//}


BOOL CZXiGateDlg::PreTranslateMessage(MSG* pMsg)
{
	// TODO: 在此添加专用代码和/或调用基类
	if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_ESCAPE) {
		return TRUE;
	}

	if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_RETURN && pMsg->wParam) {
		return TRUE;
	}

	return __super::PreTranslateMessage(pMsg);
}



void CZXiGateDlg::OnBnClickedButtonSend()
{
	//GetDlgItem(IDC_SendEdit)->SetFocus();
	if (!m_SerialPort.isOpen()) ///没有打开串口
	{
		AfxMessageBox(_T("请首先打开串口"));
		return;
	}

	CString temp;
	m_Send.GetWindowText(temp);
	int len = 0;
	char* m_str = NULL;
#ifdef UNICODE
	// 兼容中文
	CStringA strA(temp);
	len = strA.GetLength();
	m_str = strA.GetBuffer();
#else
	len = temp.GetLength();
	m_str = temp.GetBuffer(0);
#endif
	//"ZXI_HEAD"+IMGNO+PKGNO+DATA 发送图像头信息,IMGNO为图像编号：1-100，PKGNO为包编号：0-129
	//"AXI_DATA"+IMGNO+PKGNO+DATA 发送图像数据

	//int nPicNo = 3; // 图像编号，从3开始

	CString strFile = strImportFilePath; //

	FILE* fp = fopen(strFile, "rb");
	int nFileSize;
	char* picBuf = new char[25600 + 256]; //[25600 + 14]; //160*80*2+8
	int numread = 0;

	if (fp == NULL) {
		AfxMessageBox("加载文件错误！");
		return;
	}
	else {
		CString strSendInterval;
		CString str;

		UpdateData(TRUE);
		//strSendInterval.Format("%d", m_SendInterval);
		UpdateData(FALSE);
		//AfxMessageBox(strSendInterval);
		//return;

		fseek(fp, 0, SEEK_END);
		nFileSize = ftell(fp);
		fseek(fp, 0, SEEK_SET);
		picBuf = (char*)malloc(nFileSize);
		numread = fread(picBuf, sizeof(char), nFileSize, fp);

		free(picBuf); //最后free

		str.Format("%d - %d\n传输图片......", nFileSize, numread);
		AfxMessageBox(str, MB_OK);
	}
	fclose(fp);

	//-----------------------------
	if (m_hThread)
		ResumeThread(m_hThread);

	m_hThread = CreateThread(NULL, 0, &ThreadProc1, NULL, 0, &m_nThread);
	if (m_hThread == NULL) // 如果为NULL，表示创建失败
	{
		AfxMessageBox("线程创建失败!");
		//return FALSE; // 返回
	}
	else
	{
		//AfxMessageBox("线程创建成功!");
		// m_hThread->ResumeThread();			//如果创建新线程成功了，调用线程恢复函数恢复线程
	}
}


void CZXiGateDlg::OnBnClickedButtonImport()
{
	CFileDialog dlg(TRUE, NULL, NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		_T("Bin File(*.bin)|*.bin|All Files(*.*)|*.*||"), this);
	//dlg.m_ofn.lpstrFilter = "Bin Files (*.bin)|*.bin";	// 设置文件类型过滤器
	//dlg.m_ofn.lpstrDefExt = "bin";						// 设置默认文件扩展名
	//dlg.m_ofn.lpstrInitialDir = _T(".\\");				// 设置默认文件路径

	if (dlg.DoModal() == IDOK)
	{
		// 用户点击了确定按钮
		strImportFilePath = dlg.GetPathName();
		m_ctlEditImportFile.SetWindowText(strImportFilePath);
	}
}

void CZXiGateDlg::onReadEvent(const char* portName, unsigned int readBufferLen)
{
	if (readBufferLen > 0)
	{
		char* data = new char[readBufferLen + 1]; // '\0'

		if (data)
		{
			int recLen = m_SerialPort.readData(data, readBufferLen);

			if (recLen > 0)
			{
				data[recLen] = '\0';

				CString str1(data);

				rx += str1.GetLength();

				//if (str1.Find("ZXI>", 0) == 0)
				{
					CTime time;
					// int iLen;

					time = CTime::GetCurrentTime();
					CString curdata = time.Format("%Y-%m-%d %H:%M:%S");
					// curdata += ">";
					curdata += str1;
					m_ReceiveCtrl.SetSel(-1, -1);
					m_ReceiveCtrl.ReplaceSel(curdata);

					//if (m_ReceiveCtrl.GetLineCount() > 1000)
					//{
					//    m_ReceiveCtrl.SetWindowText("");

					//}
					int lines = m_ReceiveCtrl.GetLineCount(); // m_prlog是绑定CEDIT控件的对象
					if (lines > 1000)                   // 如果输出日志超过1000行，则删第一行
					{
						CString tmp;
						char tmpstr[100];                   // 定义一个大于你程序输出日志一行可能最大字数的字符数组
						m_ReceiveCtrl.GetWindowText(tmpstr, 100); // 只取前100个字符
						tmp = tmpstr;
						int it1 = tmp.Find("\r\n") + 2; // 查找第一行的回车换行位置
						m_ReceiveCtrl.SetSel(0, it1);   // 选择要删除的首行
						m_ReceiveCtrl.ReplaceSel("");   // 用空串替换掉首行
					}

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

LRESULT CZXiGateDlg::OnNcHitTest(CPoint point)
{
	// 禁止拉对话框的四边和四角
	if (CWnd::OnNcHitTest(point) == HTRIGHT ||
		CWnd::OnNcHitTest(point) == HTLEFT ||
		CWnd::OnNcHitTest(point) == HTTOP ||
		CWnd::OnNcHitTest(point) == HTBOTTOM ||
		CWnd::OnNcHitTest(point) == HTTOPLEFT ||
		CWnd::OnNcHitTest(point) == HTTOPRIGHT ||
		CWnd::OnNcHitTest(point) == HTBOTTOMLEFT ||
		CWnd::OnNcHitTest(point) == HTBOTTOMRIGHT)

		return HTCLIENT;
	return __super::OnNcHitTest(point);
}

volatile BYTE m_nShowFlag = 0x1F & (~0x08);
DWORD WINAPI ThreadProc1(LPVOID lpParam)
{
	//while (m_nShowFlag)
	{ // 0x1F
		CTime time;
		CString strTime, strFormat;

		time = CTime::GetCurrentTime();
		strFormat = " %H:%M";

		if (m_nShowFlag & 2)
		{
			strFormat = "%Y-%m-%d" + strFormat;
		}
		if (m_nShowFlag & 4)
		{ // 秒钟
			strFormat += ":%S";
		}
		if (m_nShowFlag & 8)
		{ // 周数
			strFormat += " %W";
		}
		if (m_nShowFlag & 16)
		{ // 星期
			strFormat += " %a";
		}

		strTime = time.Format(strFormat);
		//::SetDlgItemText(AfxGetApp()->m_pMainWnd->m_hWnd, IDC_STATIC_TIME, strTime);

		Sleep(100);
		/**************************************************************/

		//CCommMFCDlg dlgCommMFC;
		//CCommMFCDlg *pCommMFCDlg = reinterpret_cast<CCommMFCDlg *>(lpParam);
		//CCommMFCDlg *pCommMFCDlg = (CCommMFCDlg *)lpParam;

		CZXiGateDlg* dlg; // 将void类型强制转换成dlg类型

		dlg = (CZXiGateDlg*)pCommMFCDlg;
		int len = 0;
		char* m_str = NULL;
		char m_str_tail[2];

		char* m_data = new char[256]; // 每次发送的数据不大于256
		//char *m_data_tail = new char[2]; //协议尾

		int nPkgNo = 4;          // 传入包序号，从1开始
		int nValidPkgSize = 200; // 有效的图像包大小
		int nPkgCount;           // 总包数
		int nRetSend;            // 发送串口数据的返回值
		int nDelayMs = 450; //间隔下发数据包
		CString strFile = dlg->strImportFilePath; //

		FILE* fp = fopen(strFile, "rb");
		int nFileSize;
		char* picBuf = new char[25600 + 256]; //[25600 + 14]; //160*80*2+8
		int numread = 0;

		if (fp == NULL)
		{
			AfxMessageBox("加载文件错误！");
			fclose(fp);
			return -1;
		}
		else
		{
			CString str;

			fseek(fp, 0, SEEK_END);
			nFileSize = ftell(fp);
			fseek(fp, 0, SEEK_SET);
			picBuf = (char*)malloc(nFileSize);
			numread = fread(picBuf, sizeof(char), nFileSize, fp);

			// free(picBuf); 最后free

			str.Format("%d - %d\n传输图片......", nFileSize, numread);
			//AfxMessageBox(str, MB_OK);
			fclose(fp);
			//return 0;
		}
		//fclose(fp);

		//-------------------
		nDelayMs = dlg->m_SendInterval;
		m_str_tail[0] = '0'; //0x0D
		m_str_tail[1] = '#'; //0x0A

		m_str = "ZXI_HEAD";
		for (int i = 0; i < sizeof(m_str); i++)
		{
			m_data[i] = m_str[i];
		}
		// int nIndex = m_PicNo.GetCurSel();
		CString strPicNo;
		dlg->m_PicNo.GetWindowText(strPicNo);
		dlg->nPicNo = atoi(strPicNo);
		nPkgNo = 0;
		m_data[8] = dlg->nPicNo + 2; // 图像编号从3开始
		m_data[9] = nPkgNo;     // 包编号0
		m_data[10] = picBuf[2]; // 宽MSB
		m_data[11] = picBuf[3]; // 宽LSB
		m_data[12] = picBuf[4]; // 高MSB
		m_data[13] = picBuf[5]; // 高LSB
		m_data[14] = m_str_tail[0]; //0x0D; //'\r';        //
		m_data[15] = m_str_tail[1]; //0x0A; //'\n';        //
		len = sizeof(m_str) + 8;
		nRetSend = dlg->m_SerialPort.writeData(m_data, len);
		if (nRetSend == -1)	{
			AfxMessageBox("ZXI_HEAD发送数据错误（未知错误）！");
		}
		else if (nRetSend != len) {
			AfxMessageBox("ZXI_HEAD发送数据错误（长度错误）！");
		}
		else
		{
			CString str;
			str.Format("ZXI_DATA发送数据成功，\r\n分包号(1-...128)：%d，\r\n数据长度：%d Bytes", nPkgNo, nRetSend);
			// AfxMessageBox(str);
		}
		Sleep(nDelayMs);
		Sleep(nDelayMs);
		Sleep(nDelayMs);
		Sleep(nDelayMs);
		Sleep(nDelayMs);

		//--------------------
		m_str = "ZXI_DATA";
		for (int i = 0; i < sizeof(m_str); i++)
		{
			m_data[i] = m_str[i];
		}

		nPkgCount = (nFileSize - 8) / nValidPkgSize;
		if ((nFileSize - 8) % nValidPkgSize)
		{
			nPkgCount += 1;
		}

		m_data[8] = dlg->nPicNo + 2; // 图像编号
		/////////////////////////////
		//nValidPkgSize = 16;
		for (int nPkg = 0; nPkg < nPkgCount; nPkg++)
		{
			nPkgNo = nPkg + 1;
			m_data[9] = nPkgNo; // 包编号1开始
			for (int i = 0; i < nValidPkgSize; i++)
			{
				m_data[10 + i] = picBuf[nValidPkgSize * (nPkgNo - 1) + 8 + i];
			}
			m_data[10 + nValidPkgSize] = m_str_tail[0];     // 0x0D;     //'\r';
			m_data[10 + nValidPkgSize + 1] = m_str_tail[1]; // 0x0A; //'\n';

			len = sizeof(m_str) + 2 + nValidPkgSize + 2;
			nRetSend = dlg->m_SerialPort.writeData(m_data, len);
			Sleep(nDelayMs);
			if (nRetSend == -1)
			{
				AfxMessageBox("ZXI_DATA发送数据错误（未知错误）！");
			}
			else if (nRetSend != len)
			{
				AfxMessageBox("ZXI_DATA发送数据错误（长度错误）！");
			}
			else
			{
				CString str;
				str.Format("ZXI_DATA发送数据成功，\r\n分包号(1-...)：%d，\r\n数据长度：%d Bytes", nPkgNo, nRetSend);
				// AfxMessageBox(str);
			}
		}

		delete[] m_data;
		free(picBuf);
		//delete[] picBuf;

	}
	return 0;
}
// CommMFCDlg.h : 头文件
//

#pragma once
#include <string>
//About CSerialPort start
#include "CSerialPort/SerialPort.h"
#include "CSerialPort/SerialPortInfo.h"
#include "afxwin.h"
using namespace itas109;
//About CSerialPort end


// CCommMFCDlg 对话框
class CZXiGateDlg : public CDialog, public CSerialPortListener //About CSerialPort 
{
// 构造
public:
	CZXiGateDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
	enum { IDD = IDD_ZXIGATE_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持
	void onReadEvent(const char* portName, unsigned int readBufferLen); // About CSerialPort

//	void OnReceive();//About CSerialPort
//	void OnReceiveBusiness(); // receive business
	void Split(CString source, CStringArray& dest, CString division); //分割字符串


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
//	afx_msg void OnTimer(UINT_PTR nIDEvent);
	DECLARE_MESSAGE_MAP()

public:
	CComboBox m_PortNr;
	CComboBox m_BaudRate;
	CComboBox m_Parity;
	CComboBox m_Stop;
	CComboBox m_DataBits;
	CButton m_OpenCloseCtrl;
	CEdit m_Send;
	CEdit m_ReceiveCtrl;
	CStatic m_recvCountCtrl;
	CStatic m_sendCountCtrl;

	CEdit m_ReceiveTimeoutMSCtrl;
	UINT m_ReceiveTimeoutMS;

	CEdit m_SendIntervalCtrl;
	UINT m_SendInterval;

	CSerialPort m_SerialPort;

	afx_msg void OnClose();
	afx_msg void OnBnClickedButtonOpenClose();
	afx_msg void OnBnClickedButtonClear();

private:
	//CSerialPort m_SerialPort;//About CSerialPort 

	int rx; 
	int tx;

	//BOOL m_isTimerRunning;	

private:
    CToolBar m_wndToolBar;

	CBitmap m_bitmap1;
    CBitmap m_bitmap2;
    CBitmap m_bitmap3;
    CBitmap m_bitmap4;
    CBitmap m_bitmap5;
    CBitmap m_bitmap6;
    CBitmap m_bitmap7;
    CBitmap m_bitmap8;
    CImageList m_ImageList;
public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);

	afx_msg void OnBnClickedButtonSend();
	afx_msg void OnBnClickedButtonImport();
	afx_msg LRESULT OnNcHitTest(CPoint point);

	CComboBox m_PicNo;
	int nPicNo;
	
	CEdit m_ctlEditImportFile;
	CString strImportFilePath;	

};

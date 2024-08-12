// CommMFC.cpp : 定义应用程序的类行为。
//

#include "stdafx.h"
#include "ZXiGate.h"
#include "ZXiGateDlg.h"

#include <mysql.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


BEGIN_MESSAGE_MAP(CZXiGateApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


// CCommMFCApp 构造

CZXiGateApp::CZXiGateApp()
{
	// TODO: 在此处添加构造代码，
	// 将所有重要的初始化放置在 InitInstance 中
	nTypeTranslateData = TRANSTYOE_MYSQL;
}


// 唯一的一个 CCommMFCApp 对象

CZXiGateApp theApp;


// CCommMFCApp 初始化

BOOL CZXiGateApp::InitInstance()
{
	// 如果一个运行在 Windows XP 上的应用程序清单指定要
	// 使用 ComCtl32.dll 版本 6 或更高版本来启用可视化方式，
	//则需要 InitCommonControlsEx()。否则，将无法创建窗口。
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// 将它设置为包括所有要在应用程序中使用的
	// 公共控件类。
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinApp::InitInstance();

	AfxEnableControlContainer();

	// 标准初始化
	// 如果未使用这些功能并希望减小
	// 最终可执行文件的大小，则应移除下列
	// 不需要的特定初始化例程
	// 更改用于存储设置的注册表项
	// TODO: 应适当修改该字符串，
	// 例如修改为公司或组织名
	SetRegistryKey(_T("ZXiMemoryStick"));

	CZXiGateDlg dlg;
	m_pMainWnd = &dlg;
	INT_PTR nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO: 在此放置处理何时用
		//  “确定”来关闭对话框的代码
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: 在此放置处理何时用
		//  “取消”来关闭对话框的代码
	}

	// 由于对话框已关闭，所以将返回 FALSE 以便退出应用程序，
	//  而不是启动应用程序的消息泵。
	return FALSE;
}

int CZXiGateApp::DataLoading(CString strSQL)
{
	int nRet = 0;
	MYSQL_RES* res;

	mysql_query(&mysql, strSQL);
	//if (mysql_query(&mysql, strSQL)) { //执行insert操作画蛇添足
	//	nRet =  0;
	//}
	//else {
	//	nRet = -1;
	//	return nRet;
	//}

	//获取结果集
	res = mysql_store_result(&mysql); //返回值对insert无效,总是返回NULL

//	if (res == NULL) {
//		nRet = -1;
//		mysql_free_result(res);
//		return nRet;
//	}

//	int rows = mysql_num_rows(res);
//	mysql_free_result(res); //释放结果集

//	if (rows == 0) //通过判断所得的返回行数判断是否有查询到的数据
//	{
//		nRet = -1;
//	}
	if (mysql_errno(&mysql) == 0) nRet = 0;
	else nRet = -1;

	return nRet;
}

void CZXiGateApp::DisConnectDatabase(void)
{
	mysql_close(&mysql);
}

int CZXiGateApp::ConnectDatabase(void)
{
//	MYSQL_RES* res; //结果集结构体
//	MYSQL_ROW row;  //char** 二维数组，存放记录

	int nRet = 0;

	//初始化数据库
	mysql_init(&mysql);

	//设置编码方式
	mysql_options(&mysql, MYSQL_SET_CHARSET_NAME, "gbk");

	if (mysql_real_connect(&mysql, strHostDatebase, strUserDatabase, strPasswdDatabase, strDbDatabase, 3306, NULL, 0) == NULL) {
		TRACE(mysql_error(&mysql));
		nRet = -1;
	}
	else {
		TRACE(mysql_error(&mysql));
		nRet = 0;
	}

	return nRet;
}

/*
	database:	sensor_igate
	table:		sensor_node_rt
	struction:
	id				int			id auto
	node_id			varchar		node number
	temp			varchar		NTC Temperature
	tempc			varchar		Cell Temperature
	db_datetime		datetime	update dbase time
	node_datetime	datetime	sensor node time
*/
//数据库方式连接
void CZXiGateApp::RunConnectMySQL(CString strSQL)
{
	MYSQL mysql;    //数据库结构体
	MYSQL_RES* res; //结果集结构体
	MYSQL_ROW row;  //char** 二维数组，存放记录

	CString str;
	int nRet;

	//初始化数据库
	mysql_init(&mysql);

	//设置编码方式
	mysql_options(&mysql, MYSQL_SET_CHARSET_NAME, "gbk");

	if (mysql_real_connect(&mysql, "rm-bp159an86cl84v71p6o.mysql.rds.aliyuncs.com", "root", "root", "sensor_igate", 3306, NULL, 0) == NULL) {

		str.Format("%s", mysql_error(&mysql));
		//2022-8-14,写入日志
		//MessageBox(NULL, str, "数据库连接错误", MB_OK);
	}
	else {
		TRACE(mysql_error(&mysql));
	}

	//查询数据
	//CString strSQL;

	//strSQL = "SELECT * from tb_sensor_node";
	nRet = mysql_query(&mysql, strSQL);
	TRACE("%d\n", nRet);

	//获取结果集
	res = mysql_store_result(&mysql);

	sensor_node = new CSensorNode();
	/*
	//取得字段个数及名称
	int fieldcount = mysql_num_fields(res);

	MYSQL_FIELD* field = NULL;
	for (unsigned int i = 0; i < fieldcount; i++) {
		field = mysql_fetch_field_direct(res, i);
		TRACE(field->name);
		TRACE("\r\n");
	}

	//取得记录数
	node_rowcount = mysql_num_rows(res);

	int niRow = 0;


	while (row = mysql_fetch_row(res))
	{
		//sensor_node[niRow] = new CSensorNode();
		TRACE("\r\n");
		sensor_node[niRow].id = atoi(row[0]);
		sensor_node[niRow].node_id = row[1];
		sensor_node[niRow].temp = atof(row[2]);
		sensor_node[niRow].tempc = atof(row[3]);
		sensor_node[niRow].db_datetime = row[4];
		sensor_node[niRow].node_datetime = row[5];
		//TRACE(row[0]);		TRACE("\r\n");
		//TRACE(row[1]);		TRACE("\r\n");
		//TRACE(row[2]);		TRACE("\r\n");
		//TRACE(row[3]);		TRACE("\r\n");
		//TRACE(row[4]);		TRACE("\r\n");
		//TRACE(row[5]);		TRACE("\r\n");
		niRow++;
	}
	
	mysql_free_result(res); //释放结果集
	mysql_close(&mysql); //关闭数据库
	*/
}
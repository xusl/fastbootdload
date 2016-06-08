// MyButton.cpp : implementation file
//

#include "stdafx.h"
#include "JN516x Flash Programmer.h"
#include "MyButton.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMyButton

CMyButton::CMyButton()
{
	 m_Style = 0;               //��ť��״���

    b_InRect = false;          //�������־

    m_strText = _T("");        //��ť���֣�ʹ��Ĭ�����֣�

    m_ForeColor = RGB(0,0,0);            //������ɫ����ɫ��

    m_BackColor = RGB(243,243,243);      //����ɫ���Ұ�ɫ��

    m_LockForeColor = GetSysColor(COLOR_GRAYTEXT);    //������ť��������ɫ

    p_Font = NULL;                       //����ָ��

}

CMyButton::~CMyButton()
{
	 if ( p_Font )    delete p_Font;        //ɾ������
}


BEGIN_MESSAGE_MAP(CMyButton, CButton)
	//{{AFX_MSG_MAP(CMyButton)
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMyButton message handlers

void CMyButton::PreSubclassWindow() 
{
	// TODO: Add your specialized code here and/or call the base class
	ModifyStyle( 0, BS_OWNERDRAW );        //���ð�ť����Ϊ�Ի�ʽ

	CButton::PreSubclassWindow();
}

void CMyButton::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct) 
{
	// TODO: Add your code to draw the specified item
	CDC *pDC = CDC::FromHandle( lpDrawItemStruct->hDC );

    m_ButRect = lpDrawItemStruct->rcItem;    //��ȡ��ť�ߴ�

    if( m_strText.IsEmpty() )

        GetWindowText( m_strText );          //��ȡ��ť�ı�

    int nSavedDC = pDC->SaveDC();

    VERIFY( pDC );

    DrawButton( pDC );                //���ư�ť

    pDC->RestoreDC( nSavedDC );

}

void CMyButton::DrawButton(CDC *pDC)

{

    //����״̬

    if( m_Style==3 ) m_Style = 0;

    if( GetStyle() & WS_DISABLED )

        m_Style = 3;    //��ֹ״̬

    //����״̬�����߿���ɫ��������ɫ

    COLORREF bColor, fColor;    //bColorΪ�߿���ɫ��fColorΪ������ɫ

    switch( m_Style )

    {

    case 0: bColor = RGB(192,192,192); fColor = m_ForeColor; break;  //������ť

    case 1: bColor = RGB(255,255,255); fColor = m_ForeColor; break;  //������ʱ��ť

    case 2: bColor = RGB(192,192,192); fColor = m_ForeColor; break;  //���µİ�ť

    case 3: bColor = m_BackColor; fColor = m_LockForeColor; break;   //�����İ�ť

    }

    //���ư�ť����

    CBrush Brush;

    Brush.CreateSolidBrush( m_BackColor );    //����ˢ

    pDC->SelectObject( &Brush );

    CPen Pen;

    Pen.CreatePen(PS_SOLID, 1, bColor );

    pDC->SelectObject( &Pen );

    pDC->RoundRect(&m_ButRect,CPoint(5,5));   //��Բ�Ǿ���

    //���ư�ť����ʱ�ı߿�

    if( m_Style!=2 )

    {

        CRect Rect;

        Rect.SetRect( m_ButRect.left+2, m_ButRect.top+1, m_ButRect.right, m_ButRect.bottom );

        pDC->DrawEdge( &Rect, BDR_RAISEDINNER, BF_RECT );    //���߿�

    }

    //���ư�ť����

    pDC->SetTextColor( fColor );        //������

    pDC->SetBkMode( TRANSPARENT );

    pDC->DrawText( m_strText, &m_ButRect, DT_SINGLELINE | DT_CENTER

        | DT_VCENTER | DT_END_ELLIPSIS);

    //����ӵ�н��㰴ť�����߿�

    if( GetFocus()==this )

    {

        CRect Rect;

        Rect.SetRect( m_ButRect.left+3, m_ButRect.top+2, m_ButRect.right-3, m_ButRect.bottom-2 );

        pDC->DrawFocusRect( &Rect );    //��ӵ�н�������߿�

    }

} 



void CMyButton::OnMouseMove(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	RECT rect;
	rect.left = 10;
	rect.top = 10;
	rect.right = 100;
	rect.bottom = 50;

    if( !b_InRect || GetCapture()!=this )    //�����밴ť

    {

        b_InRect = true;    //���ý����־

        //SetCapture();       //�������

        m_Style = 1;        //���ð�ť״̬

        //Invalidate();       //�ػ水ť
		InvalidateRect(&rect, FALSE);
    }

    else

    {

        if ( !m_ButRect.PtInRect(point) )    //����뿪��ť

        {

            b_InRect = false;   //��������־

            //ReleaseCapture();   //�ͷŲ�������

            m_Style = 0;        //���ð�ť״̬

           // Invalidate();       //�ػ水ť
		   InvalidateRect(&rect, FALSE);

        }

    }

//	CButton::OnMouseMove(nFlags, point);
}

void CMyButton::OnLButtonDown(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	m_Style = 2;

    Invalidate();        //�ػ水ť

	CButton::OnLButtonDown(nFlags, point);
}

void CMyButton::OnLButtonUp(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	 m_Style = 1;

    Invalidate();        //�ػ水ť

	CButton::OnLButtonUp(nFlags, point);
}

void CMyButton::SetText(CString str)
{
    m_strText = _T("");

    SetWindowText(str);
} 

 

//�����ı���ɫ

void CMyButton::SetForeColor(COLORREF color)

{

    m_ForeColor = color;

    Invalidate();

} 

 

//���ñ�����ɫ

void CMyButton::SetBkColor(COLORREF color)
{
    m_BackColor = color;

    Invalidate();
} 
//��������(����߶ȡ�������)

void CMyButton::SetTextFont(int FontHight,LPCTSTR FontName)
{
    if ( p_Font )    delete p_Font;    //ɾ��������

    p_Font = new CFont;

    p_Font->CreatePointFont( FontHight, FontName );    //����������

    SetFont( p_Font );                //��������
} 

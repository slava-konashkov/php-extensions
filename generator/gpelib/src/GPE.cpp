#include "GPE.h"

#include "../../aeslib/include/aes.h"
#include <string.h>
#include <memory.h>


//#ifdef _DEBUG
//#pragma comment(lib,"../../aeslib/include//aes.lib")
//#else
//#pragma comment(lib,"../Release/aes.lib")
//#endif


CMatrixBitmap::CMatrixBitmap(void)
: m_pMatrix(NULL)
, m_nWidth(0)
, m_nHeight(0)
{
}
CMatrixBitmap::~CMatrixBitmap(void)
{
	Destroy();
}
void CMatrixBitmap::Destroy(void)
{
	if(m_pMatrix)
	{
		delete [] m_pMatrix;
		m_pMatrix=NULL;
		m_nWidth=0;
		m_nHeight=0;
	}
}
void CMatrixBitmap::Create(GPE_UINT nWidth,GPE_UINT nHeight)
{
	Destroy();
	if(nWidth==0||nHeight==0) return;
	m_nWidth=nWidth;
	m_nHeight=nHeight;
	m_pMatrix=new GPE_BYTE[m_nWidth*m_nHeight];
	::memset(m_pMatrix,0xFF,m_nWidth*m_nHeight);
}
void CMatrixBitmap::operator=(const CMatrixBitmap& MatrixBitmap)
{
	Create(MatrixBitmap.m_nWidth,MatrixBitmap.m_nHeight);
	memcpy(m_pMatrix,MatrixBitmap.m_pMatrix,MatrixBitmap.m_nWidth*MatrixBitmap.m_nHeight);
}
void CMatrixBitmap::SetPixel(GPE_UINT nX,GPE_UINT nY,GPE_BYTE cbPixValue)
{
	if(nX>=m_nWidth) return;
	if(nY>=m_nHeight) return;
	m_pMatrix[nY*m_nWidth+nX]=cbPixValue;
}
GPE_BYTE CMatrixBitmap::GetPixel(GPE_UINT nX,GPE_UINT nY) const
{
	if(nX>=m_nWidth) return 0x80;
	if(nY>=m_nHeight) return 0x80;
	return m_pMatrix[nY*m_nWidth+nX];
}
// ********************************************************************************
// class CGPEMask
// ********************************************************************************
CGPEMask::CGPEMask(void)
: CMatrixBitmap()
, m_pRNDMask(NULL)
, m_pRNDMaskPosition(NULL)
{
}
CGPEMask::~CGPEMask(void)
{
	Destroy();
}
void CGPEMask::Destroy(void)
{
	if(m_pRNDMask)
	{
		delete [] m_pRNDMask;
		m_pRNDMask=NULL;
	}
	if(m_pRNDMaskPosition)
	{
		delete [] m_pRNDMaskPosition;
		m_pRNDMaskPosition=NULL;
	}
	CMatrixBitmap::Destroy();
}
void CGPEMask::Generate(GPE_UINT dwSeed)
{
	// ***********************************************************************************
	//  ��������� ����� 141*141 - � ��������.
	// ��������� ����� ������������ ����� ����� �������� � ������� ����� 
	// ������������ ����������� ����.
	// ����������� ���� ������������ ������������� � ������ ������� (�������� �������=0).
	// ��������� �������� ������������ ����� �� ����� ��� ����� ����������� ���������������,
	// � ������� �������� �������� ��� �������������� (������������������ ����).
	// ***********************************************************************************
	// 1. ������� ������
	Create(141,141);
	// 2. ��������� ����� � ����� ������������� �������� 13x13
	for(GPE_INT y=0;y<m_nHeight;y++)
	{
		for(GPE_INT x=0;x<m_nWidth;x++)
		{
			// *************************************************************
			// ������ ������
			// *************************************************************
			if(y>=0&&y<4||y>=137&&y<141||x>=0&&x<4||x>=137&&x<141)
			{
				SetPixel(x,y,0x00);
			}
			// *************************************************************
			// ������ ������������� �������
			// *************************************************************
			else if((y>=4&&y<17||y>=44&&y<57||y>=84&&y<97||y>=124&&y<137)
				  &&(x>=4&&x<17||x>=44&&x<57||x>=84&&x<97||x>=124&&x<137))
			{
				SetPixel(x,y,0x00);
			}
			// *************************************************************
			// ������ ����������� �������
			// *************************************************************
			else if(y>=57&&y<84&&x>=57&&x<84)
			{
				SetPixel(x,y,0x00);
			}
		}
	}
	// ********************************************************************
	// ������������ ������ ��� ������� ��������� ��������
	// ********************************************************************
	m_pRNDMask=new GPE_BYTE[GPE_MAX_RND_MASK_SIZE];
	m_pRNDMaskPosition=new GPE_UINT[GPE_MAX_RND_AREAS];
	::memset(m_pRNDMask,0xFF,GPE_MAX_RND_MASK_SIZE);
	::memset(m_pRNDMaskPosition,0,GPE_MAX_RND_AREAS*sizeof(GPE_UINT));
	// ********************************************************************
	// ��������� ������� �������� �������
	// ********************************************************************
	m_RND.SetSeed(dwSeed);
	GPE_INT nBlackPixCounter=0;
	GPE_UINT nPos;
	while(nBlackPixCounter<GPE_MAX_RND_AREAS)
	{
		nPos=0;
		nPos=m_RND.GenByte();nPos<<=8;
		nPos|=m_RND.GenByte();
		nPos%=GPE_MAX_RND_MASK_SIZE;
		if(m_pRNDMask[nPos]==0xFF)
		{
			m_pRNDMask[nPos]=0x00;
			nBlackPixCounter++;
		}
	}
	// ********************************************************************
	// ��������� ������ ����������� ��������� ����������� ��������
	// ********************************************************************
	for(GPE_INT i=0,c=0;i<GPE_MAX_RND_MASK_SIZE;i++)
	{
		if(m_pRNDMask[i]==0x00)
		{
			// ��������� ����������
			m_pRNDMaskPosition[c++]=i;
		}
	}
	// *******************************************************************
	// 3. ��������� ������ ������������.
	// ���������� �������� ��� ���������� ������ ���������� 3960 ��.
	// *******************************************************************
	GPE_INT nRndPixCounter=0;
	for(GPE_UINT y=0;y<m_nHeight;y++)
	{
		for(GPE_UINT x=0;x<m_nWidth;x++)
		{
			// *******************************************************************
			// ���������� ������ ������������ ����� ��� ����� ��������.
			// *******************************************************************
			if (GetPixel(x,y) == 0xFF)
			{
				SetPixel(x,y,m_pRNDMask[nRndPixCounter++]);
			}
		}
	}
}
void CGPEMask::PositionsToVector(std::vector<GPE_UINT>& Positions)
{
	Positions.clear();
	for(GPE_INT i = 0;i<GPE_MAX_RND_AREAS;i++)
	{
		Positions.push_back(m_pRNDMaskPosition[i]);
	}
}
CGPEProtectionLayer::CGPEProtectionLayer(void)
{
}
CGPEProtectionLayer::~CGPEProtectionLayer(void)
{
}
void CGPEProtectionLayer::Destroy(void)
{
	CMatrixBitmap::Destroy();
}
// ***********************************************************************************
// ������ CV - ������ ��� ������
// TEST
/*/
int g_TEST[]=
{
-1,  1, -1,  1, -1, -1, -1, -1,  1, -1,  1, -1,  1,  1,  1,  1, -1, -1, -1, -1, -1, -1, -1, -1,  1,  1, -1,  1,  1,  1,  1,  1,  1, -1, -1,  1, -1, -1,  1,  1, -1, -1,  1, -1, -1, -1, -1,  1,  1, -1,  1,  1,
 1, -1, -1,  1, -1,  1,  1,  1,  1,  1, -1,  1,  1,  1, -1, -1, -1, -1,  1, -1,  1,  1,  1,  1, -1, -1, -1,  1,  1, -1, -1, -1,  1, -1, -1, -1, -1,  1, -1,  1, -1,  1,  1, -1, -1, -1, -1, -1,  1,  1,  1,  1,
 1,  1, -1, -1, -1,  1,  1,  1,  1, -1, -1,  1, -1, -1, -1,  1,  1, -1,  1,  1,  1, -1,  1,  1, -1, -1,  1,  1, -1,  1, -1,  1, -1, -1,  1, -1,  1,  1, -1,  1, -1,  1, -1,  1,  1,  1, -1,  1, -1, -1, -1,  1,
-1,  1, -1,  1, -1, -1, -1, -1, -1, -1, -1,  1, -1, -1, -1, -1, -1, -1, -1,  1,  1,  1, -1, -1,  1, -1,  1, -1,  1,  1, -1, -1, -1,  1, -1,  1,  1, -1, -1, -1, -1,  1,  1,  1,  1, -1,  1, -1,  1,  1, -1,  1,
 1,  1,  1,  1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  1,  1, -1, -1, -1, -1, -1,  1,  1,  1,  1, -1, -1,  1,  1,  1, -1, -1,  1,  1, -1,  1,  1,  1,  1,  1,  1, -1,  1, -1, -1,  1, -1,  1, -1,  1, -1,
-1, -1, -1, -1, -1, -1, -1, -1,  1, -1,  1, -1,  1, -1, -1,  1,  1,  1,  1,  1,  1, -1,  1,  1, -1, -1,  1, -1, -1,  1,  1,  1,  1,  1, -1,  1, -1, -1, -1,  1,  1, -1,  1, -1,  1, -1,  1,  1,  1,  1,  1, -1,
 1,	 1,  1, -1, -1, -1, -1, -1,  1, -1, -1,  1, -1,  1,  1, -1, -1, -1, -1,  1,  1,  1, -1,  1,  1, -1, -1,  1,  1, -1,  1, -1, -1,  1, -1, -1,  1, -1, -1,  1, -1,  1,  1,  1, -1,  1, -1,  1, -1, -1, -1, -1,
 1, -1,  1,  1,  1, -1,  1,  1, -1,  1, -1, -1, -1, -1, -1,  1,  1,  1, -1, -1,  1, -1, -1,  1,  1, -1, -1, -1, -1,  1, -1, -1, -1, -1,  1,  1, -1, -1, -1,  1, -1,  1,  1, -1, -1, -1,  1,  1,  1, -1,  1,  1,
 1, -1, -1,  1, -1,  1, -1, -1,  1,  1,  1,  1, -1,  1,  1,  1,  1,  1, -1, -1,  1, -1,  1,  1, -1,  1, -1, -1,  1,  1, -1, -1,  1, -1, -1,  1,  1, -1, -1,  1,  1,  1, -1,  1,  1,  1,  1,  1,  1, -1, -1,  1,
-1,  1, -1,  1,  1, -1,  1,  1,  1, -1,  1,  1, -1, -1, -1,  1,  1,  1, -1,  1, -1,  1,  1,  1, -1, -1,  1, -1, -1, -1,  1, -1,  1, -1, -1,  1,  1, -1, -1,  1,  1,  1, -1, -1,  1,  1, -1, -1, -1,  1,  1, -1,
 1, -1,  1, -1, -1,  1,  1, -1, -1, -1, -1,  1,  1, -1, -1,  1, -1,  1, -1, -1,  1,  1, -1,  1,  1, -1,  1, -1,  1, -1, -1, -1, -1,  1,  1, -1,  1, -1, -1, -1, -1, -1, -1,  1,  1, -1,  1, -1,  1, -1, -1,  1,
-1,  1,  1,  1, -1,  1,  1, -1,  1,  1, -1, -1,  1, -1,  1, -1,  1, -1, -1, -1, -1, -1, -1, -1,  1,  1,  1,  1, -1,  1, -1,  1,  1, -1,  1,  1,  1, -1, -1,  1, -1, -1,  1, -1, -1, -1,  1,  1,  1, -1,  1,  1,
-1,  1,  1,  1, -1, -1, -1,  1, -1,  1, -1, -1,  1,  1, -1, -1,  1, -1, -1,  1,  1,  1,  1,  1, -1,  1,  1, -1,  1,  1, -1,  1,  1,  1,  1,  1, -1,  1,  1,  1, -1,  1, -1, -1, -1, -1,  1, -1,  1, -1, -1, -1,
 1, -1, -1,  1, -1,  1, -1,  1,  1,  1,  1, -1, -1, -1,  1,  1, -1, -1,  1,  1,  1, -1, -1, -1, -1,  1,  1, -1,  1,  1,  1,  1, -1,  1,  1,  1, -1,  1, -1, -1, -1,  1, -1,  1,  1,  1,  1,  1, -1,  1,  1, -1,
 1, -1,  1,  1,  1, -1, -1,  1,  1,  1, -1,  1,  1, -1, -1,  1,  1, -1,  1,  1,  1,  1, -1,  1,  1, -1,  1,  1, -1, -1,  1,  1, -1,  1,  1, -1, -1,  1,  1,  1, -1, -1,  1,  1, -1,  1, -1, -1,  1,  1,  1, -1,
 1, -1,  1,  1,  1,  1,  1, -1,  1,  1,  1,  1, -1, -1, -1,  1, -1, -1,  1, -1, -1,  1, -1, -1,  1,  1, -1,  1, -1,  1, -1,  1, -1, -1, -1, -1, -1,  1, -1, -1, -1, -1, -1, -1,  1,  1,  1,  1, -1,  1,  1, -1,
 1, -1, -1,  1, -1, -1,  1,  1,  1,  1, -1,  1, -1, -1,  1, -1,  1, -1, -1, -1, -1, -1,  1,  1, -1,  1,  1,  1, -1, -1,  1, -1, -1, -1, -1, -1,  1, -1, -1, -1,  1, -1,  1,  1,  1,  1,  1,  1,  1, -1,  1, -1,
 1, -1,  1, -1,  1, -1, -1, -1, -1, -1, -1, -1, -1,  1,  1, -1,  1, -1, -1,  1, -1,  1,  1, -1, -1,  1, -1, -1,  1, -1,  1,  1, -1, -1,  1,  1,  1, -1, -1,  1,  1,  1,  1,  1, -1, -1, -1,  1, -1, -1, -1,  1,
-1,  1, -1, -1, -1,  1,  1, -1, -1,  1,  1,  1,  1, -1, -1,  1, -1, -1,  1,  1, -1, -1, -1, -1,  1,  1,  1,  1,  1, -1, -1, -1,  1,  1,  1,  1,  1, -1,  1,  1,  1,  1, -1,  1,  1, -1,  1, -1,  1,  1, -1,  1,
-1, -1, -1,  1, -1,  1, -1,  1,  1,  1, -1,  1, -1,  1,  1,  1,  1,  1,  1, -1, -1, -1, -1,  1,  1, -1, -1,  1,  1,  1,  1,  1,  1,  1, -1,  1,  1,  1,  1,  1,  1, -1,  1, -1, -1, -1,  1,  1, -1, -1,  1, -1,
 1, -1,  1, -1, -1,  1, -1,  1, -1, -1, -1, -1,  1, -1, -1, -1, -1,  1,  1,  1, -1,  1,  1, -1,  1,  1,  1,  1, -1, -1,  1, -1, -1,  1,  1, -1,  1,  1, -1, -1, -1,  1, -1, -1, -1,  1, -1,  1, -1,  1, -1,  1,
-1,  1, -1, -1, -1,  1, -1,  1,  1,  1, -1,  1,  1,  1, -1,  1,  1, -1,  1, -1,  1, -1, -1,  1, -1, -1,  1, -1, -1, -1,  1, -1, -1, -1, -1,  1, -1,  1,  1,  1,  1, -1,  1,  1, -1, -1, -1,  1, -1,  1, -1,  1,
-1, -1,  1, -1, -1,  1, -1,  1, -1, -1,  1,  1,  1, -1,  1, -1,  1,  1,  1, -1,  1, -1, -1, -1, -1,  1, -1,  1, -1, -1, -1, -1,  1, -1, -1,  1,  1,  1, -1,  1, -1, -1, -1,  1,  1,  1,  1, -1, -1,  1,  1,  1,
-1,  1,  1, -1, -1, -1, -1,  1,  1, -1,  1, -1,  1, -1, -1, -1, -1,  1, -1,  1,  1, -1,  1,  1, -1, -1, -1,  1,  1,  1,  1,  1,  1, -1, -1, -1,  1,  1,  1,  1,  1,  1,  1, -1, -1,  1,  1, -1,  1,  1,  1, -1,
 1, -1, -1,  1,  1, -1, -1,  1,  1, -1,  1,  1,  1, -1,  1,  1, -1,  1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  1, -1,  1,  1,  1,  1, -1, -1,  1, -1,  1,  1,  1, -1, -1, -1,  1,  1,  1,  1,  1, -1,  1,  1, -1,
 1,  1, -1,  1, -1,  1,  1,  1, -1, -1,  1, -1,  1,  1, -1, -1,  1, -1,  1,  1, -1, -1, -1, -1,  1,  1,  1, -1,  1,  1,  1, -1, -1, -1, -1, -1,  1, -1, -1, -1,  1, -1, -1,  1, -1, -1, -1,  1,  1,  1, -1, -1,
 1,  1,  1,  1, -1, -1, -1,  1, -1,  1, -1,  1,  1,  1, -1,  1,  1,  1,  1,  1, -1, -1, -1, -1,  1, -1,  1,  1,  1,  1,  1,  1, -1,  1,  1,  1, -1,  1,  1,  1, -1,  1, -1,  1, -1,  1, -1, -1,  1, -1, -1,  1,
-1,  1, -1,  1, -1,  1, -1, -1, -1, -1,  1,  1,  1,  1, -1, -1, -1,  1, -1,  1, -1, -1,  1,  1, -1, -1,  1, -1, -1,  1,  1, -1,  1, -1, -1,  1,  1, -1,  1,  1, -1, -1, -1,  1, -1, -1,  1, -1,  1, -1,  1, -1,
-1,  1,  1, -1, -1,  1, -1,  1,  1,  1,  1, -1,  1, -1,  1,  1, -1, -1,  1,  1,  1, -1,  1,  1,  1, -1,  1,  1, -1, -1, -1,  1,  1, -1, -1, -1, -1,  1,  1, -1, -1,  1, -1,  1, -1, -1, -1, -1,  1, -1,  1,  1,
 1, -1, -1, -1, -1,  1,  1,  1,  1, -1, -1, -1,  1,  1, -1,  1, -1, -1, -1, -1, -1, -1,  1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  1, -1, -1,  1,  1,  1, -1, -1,  1, -1,  1,  1, -1,  1,  1, -1, -1,  1,  1,
-1, -1, -1, -1,  1,  1, -1,  1,  1, -1,  1, -1,  1, -1, -1, -1,  1, -1,  1,  1,  1, -1, -1, -1,  1,  1,  1,  1, -1, -1,  1,  1, -1,  1, -1,  1,  1, -1,  1,  1, -1, -1, -1, -1, -1,  1,  1, -1,  1,  1,  1, -1,
-1, -1, -1, -1,  1,  1, -1,  1,  1,  1, -1, -1, -1,  1,  1, -1, -1,  1, -1,  1, -1,  1, -1, -1,  1,  1,  1,  1,  1, -1,  1, -1, -1, -1,  1, -1,  1, -1,  1,  1,  1, -1, -1,  1,  1,  1, -1,  1,  1, -1,  1, -1,
-1,  1, -1, -1, -1,  1,  1, -1,  1, -1, -1, -1,  1, -1, -1,  1, -1,  1,  1, -1, -1,  1, -1, -1, -1,  1, -1, -1,  1,  1, -1, -1, -1, -1, -1, -1,  1,  1, -1,  1,  1, -1, -1, -1, -1, -1,  1, -1, -1, -1, -1, -1,
-1,  1, -1, -1, -1,  1,  1,  1, -1,  1, -1,  1,  1, -1, -1, -1, -1,  1, -1,  1, -1,  1,  1,  1,  1, -1,  1, -1,  1,  1, -1,  1,  1, -1,  1,  1,  1, -1, -1,  1,  1, -1, -1,  1, -1,  1,  1,  1, -1, -1,  1, -1,
-1,  1, -1,  1,  1,  1, -1,  1, -1,  1, -1,  1,  1,  1, -1, -1, -1,  1,  1,  1, -1, -1, -1, -1, -1, -1, -1, -1,  1, -1,  1,  1,  1,  1,  1,  1, -1, -1,  1,  1,  1,  1, -1, -1,  1, -1, -1,  1, -1,  1, -1,  1,
-1, -1, -1,  1, -1,  1,  1,  1,  1, -1,  1,  1,  1, -1, -1, -1,  1, -1, -1,  1, -1, -1, -1,  1,  1, -1, -1, -1,  1,  1, -1,  1, -1, -1,  1,  1,  1,  1,  1,  1,  1, -1,  1, -1,  1, -1, -1, -1, -1, -1, -1,  1,
-1,  1,  1, -1, -1, -1, -1, -1, -1,  1,  1,  1, -1,  1, -1,  1, -1, -1,  1, -1,  1,  1,  1,  1,  1, -1,  1, -1, -1, -1,  1, -1,  1, -1, -1,  1, -1,  1, -1,  1, -1,  1,  1, -1, -1,  1,  1,  1, -1,  1, -1, -1,
-1,  1, -1, -1,  1,  1,  1,  1,  1, -1,  1, -1,  1, -1, -1,  1,  1,  1,  1,  1,  1,  1,  1, -1, -1,  1,  1, -1, -1, -1,  1, -1, -1,  1, -1,  1, -1,  1, -1,  1, -1,  1,  1,  1,  1, -1,  1,  1, -1, -1, -1, -1,
-1, -1,  1,  1,  1,  1,  1,  1,  1,  1,  1, -1, -1, -1,  1, -1, -1, -1,  1,  1, -1, -1,  1,  1, -1,  1, -1, -1,  1, -1,  1, -1, -1,  1, -1,  1,  1,  1, -1,  1,  1,  1, -1, -1,  1, -1, -1, -1,  1,  1,  1,  1,
-1,  1, -1,  1, -1, -1, -1, -1, -1,  1, -1, -1, -1,  1, -1,  1,  1, -1,  1, -1,  1, -1, -1, -1,  1, -1,  1,  1,  1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  1,  1,  1, -1,  1,  1,  1,  1, -1,  1, -1,  1, -1,  1,
-1,  1, -1, -1, -1, -1, -1,  1,  1,  1, -1,  1, -1, -1, -1, -1,  1, -1,  1,  1, -1,  1, -1, -1, -1,  1,  1, -1,  1,  1, -1, -1,  1, -1, -1, -1,  1,  1, -1,  1,  1, -1, -1, -1,  1, -1,  1,  1,  1,  1, -1,  1,
-1, -1, -1,  1, -1,  1, -1,  1,  1, -1,  1, -1,  1, -1,  1, -1, -1, -1, -1, -1,  1,  1,  1, -1, -1,  1,  1, -1, -1, -1, -1, -1,  1,  1,  1, -1,  1, -1, -1,  1,  1, -1,  1, -1, -1,  1, -1, -1, -1, -1, -1,  1,
-1, -1,  1, -1, -1,  1, -1,  1, -1,  1,  1,  1, -1,  1,  1, -1, -1, -1, -1,  1, -1,  1, -1,  1,  1, -1, -1, -1, -1, -1, -1, -1, -1,  1, -1,  1, -1, -1,  1,  1, -1, -1,  1, -1, -1,  1, -1,  1,  1, -1, -1,  1,
-1, -1,  1, -1,  1, -1,  1, -1, -1,  1, -1,  1,  1, -1,  1, -1,  1,  1, -1, -1, -1,  1, -1,  1, -1,  1, -1,  1, -1, -1, -1, -1, -1,  1, -1, -1,  1, -1,  1, -1, -1,  1, -1, -1, -1,  1, -1, -1,  1,  1,  1,  1,
-1, -1, -1,  1,  1,  1,  1, -1, -1,  1, -1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1, -1,  1,  1,  1, -1, -1,  1,  1,  1, -1,  1,  1,  1,  1, -1,  1, -1, -1,  1,  1,  1,  1, -1, -1, -1,  1, -1,
 1,  1, -1,  1, -1,  1,  1,  1,  1,  1, -1, -1, -1, -1, -1,  1, -1, -1, -1,  1, -1,  1, -1,  1,  1, -1, -1, -1,  1,  1,  1,  1,  1,  1, -1, -1,  1, -1,  1,  1, -1,  1,  1,  1,  1,  1, -1,  1, -1,  1,  1,  1,
-1, -1, -1,  1, -1,  1, -1, -1,  1,  1, -1,  1,  1, -1,  1, -1, -1, -1, -1,  1, -1, -1,  1, -1, -1,  1,  1,  1,  1,  1,  1, -1, -1, -1, -1,  1,  1, -1, -1,  1, -1, -1, -1,  1,  1, -1,  1, -1,  1, -1, -1,  1,
 1, -1, -1,  1,  1,  1,  1, -1, -1,  1,  1,  1,  1, -1,  1, -1,  1, -1, -1,  1,  1,  1, -1,  1, -1, -1, -1, -1, -1, -1, -1,  1,  1,  1, -1, -1,  1, -1, -1, -1,  1, -1,  1, -1, -1, -1, -1,  1, -1, -1, -1,  1,
 1,  1, -1,  1,  1, -1,  1, -1, -1, -1,  1,  1, -1, -1, -1, -1, -1, -1, -1,  1,  1,  1, -1, -1,  1,  1, -1, -1, -1, -1,  1,  1, -1, -1, -1,  1, -1, -1,  1, -1, -1, -1,  1,  1, -1, -1, -1,  1, -1,  1, -1, -1,
 1,  1,  1, -1,  1,  1,  1,  1,  1, -1,  1,  1, -1, -1, -1, -1,  1,  1, -1,  1, -1, -1,  1,  1, -1, -1, -1, -1, -1,  1, -1, -1,  1,  1,  1, -1,  1,  1, -1, -1, -1, -1,  1, -1, -1, -1, -1,  1, -1, -1,  1,  1,
-1, -1, -1, -1,  1, -1, -1, -1,  1,  1,  1,  1, -1, -1,  1, -1,  1,  1, -1,  1,  1, -1,  1,  1, -1, -1, -1,  1, -1,  1, -1, -1, -1,  1, -1, -1,  1,  1,  1, -1, -1,  1, -1, -1,  1,  1, -1,  1, -1,  1, -1,  1,
 1,  1, -1, -1, -1,  1, -1, -1,  1, -1, -1,  1, -1, -1,  1, -1, -1,  1, -1,  1, -1,  1, -1,  1,  1, -1, -1,  1, -1, -1, -1,  1,  1, -1,  1,  1, -1, -1, -1,  1, -1, -1, -1,  1,  1, -1,  1, -1,  1,  1,  1, -1 
};*/
void CGPEProtectionLayer::Generate(const CGPEMask& MaskLayer,GPE_UINT dwFixedSeed,GPE_UINT dwGlobalSeed)
{
	Create(MaskLayer.GetWidth(),MaskLayer.GetHeight());
	// **************************************************************************
	// ������� �����
	// **************************************************************************
	for(GPE_INT y=0;y<m_nHeight;y++)
	{
		for(GPE_INT x=0;x<m_nWidth;x++)
		{
			if(y>=0&&y<4||y>=137&&y<141||x>=0&&x<4||x>=137&&x<141)
			{
				SetPixel(x,y,0x00);
			}
		}
	}
	// **************************************************************************
	// ���������� �������� ������������������ �� �����
	// **************************************************************************
	m_Bits.clear();
	m_RND.SetSeed(dwFixedSeed);
	GPE_INT pixIdx=0;
	// ******************************************************************
	// ��������� ������������� ������� ������������������ ������
	// ����� ��������:
	//   1  2  3  4 
	//   5  6  7  8
	//   9 10 11 12
	//  13 14 15 16 
	// ******************************************************************
	GPE_FILL_SQUARE(4,17,4,17,m_RND,(*this));
	GPE_FILL_SQUARE(4,17,44,57,m_RND,(*this));
	GPE_FILL_SQUARE(4,17,84,97,m_RND,(*this));
	GPE_FILL_SQUARE(4,17,124,137,m_RND,(*this));
	
	GPE_FILL_SQUARE(44,57,4,17,m_RND,(*this));
	GPE_FILL_SQUARE(44,57,44,57,m_RND,(*this));
	GPE_FILL_SQUARE(44,57,84,97,m_RND,(*this));
	GPE_FILL_SQUARE(44,57,124,137,m_RND,(*this));

	GPE_FILL_SQUARE(84,97,4,17,m_RND,(*this));
	GPE_FILL_SQUARE(84,97,44,57,m_RND,(*this));
	GPE_FILL_SQUARE(84,97,84,97,m_RND,(*this));
	GPE_FILL_SQUARE(84,97,124,137,m_RND,(*this));

	GPE_FILL_SQUARE(124,137,4,17,m_RND,(*this));
	GPE_FILL_SQUARE(124,137,44,57,m_RND,(*this));
	GPE_FILL_SQUARE(124,137,84,97,m_RND,(*this));
	GPE_FILL_SQUARE(124,137,124,137,m_RND,(*this));
	// ************************************************************************************************************
	// ��������� �������� ���� ��� ���������� ��������.
	// ************************************************************************************************************
	m_RND.SetSeed(dwGlobalSeed);
	for(GPE_INT y=4;y<MaskLayer.GetHeight()-4;y++)
	{
		for(GPE_INT x=4;x<MaskLayer.GetWidth()-4;x++)
		{
			if((y>=4&&y<17||y>=44&&y<57||y>=84&&y<97||y>=124&&y<137)
		 	 &&(x>=4&&x<17||x>=44&&x<57||x>=84&&x<97||x>=124&&x<137))
			{
				continue;
			}
			// *************************************************************
			// ��������� ����������� ������� (��������� ��������� �����)
			// *************************************************************
			else if(y>=57&&y<84&&x>=57&&x<84)
			{
				if((m_RND.GenByte()&0x80)!=0) SetPixel(x,y,0x00);
			}
			// ******************************************************************
			// ��������� ���������� ������� �������� �������������������
			// ******************************************************************
			else if(MaskLayer.GetPixel(x,y)==0x00)
			{
				if((m_RND.GenByte()&0x80)==0)
				{
					SetPixel(x,y,0x00);
					m_Bits.push_back(0);
				}
				else
				{
					m_Bits.push_back(1);
				}
			}
		}
	}
}
void CGPEProtectionLayer::BitsToVector(std::vector<GPE_UINT>& Bits)
{
	Bits=m_Bits;
}
// ********************************************************************************
// class CGPEProtectionLayer
// ********************************************************************************
CGPELayer::CGPELayer(void)
: m_pGPE(NULL)
{
}
CGPELayer::~CGPELayer(void)
{
	Destroy();
}
void CGPELayer::Destroy(void)
{
	if(m_pGPE)
	{
		delete [] m_pGPE;
		m_pGPE=NULL;
	}
	CMatrixBitmap::Destroy();
}
void CGPELayer::Generate(const CGPEMask& MaskLayer
	                    ,const CGPEProtectionLayer& ProtectionLayer
	                    ,const GPE_BYTE* pUserData
	                    ,GPE_INT nUserDataLength
						,const GPE_BYTE* pKenc
					    ,GPE_INT nKencBitLength)
{
	// ****************************************************************************************
	// ��������� �������������� ���
	// ****************************************************************************************
	// ������ ��� �������
	// ��������� ����������� RSA
	// unsigned char cbEncodedMsg[]=
	// {
	//	0x11,0x51,0x48,0xf9,0x85,0x2c,0xc5,0x5f,0xe6,0x45,0x2a,0x89,0x2a,0xa2,0xed,0xc1,
	//	0x23,0x78,0xf0,0xd1,0xe4,0xe6,0xaf,0x84,0x1d,0x17,0xaa,0xa3,0xf8,0xcd,0x6f,0x3d
	//};
	// ****************************************************************************************
	// ������� ��������� ������������
	// ���� ����������� � ���������.
	// 0-------------------------------31
	// MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM0
	// ****************************************************************************************
	GPE_BYTE cbEncodedMsg[32];
	memset(cbEncodedMsg,0,sizeof(cbEncodedMsg));
	if(nUserDataLength>31) nUserDataLength=31;
	memcpy(cbEncodedMsg,pUserData,nUserDataLength);
	cbEncodedMsg[nUserDataLength]=0x80;
	// ****************************************************************************************
	// ���������� AES
	// ****************************************************************************************
	AES_KEY Kenc;
	GPE_BYTE cbIV[16]={0}; // ������� ��������� ������
	AES_set_encrypt_key(pKenc,nKencBitLength,&Kenc);
	//BYTE cbTest[]={0x12,0x34,0x56,0x78,0x90,0x12,0x34,0x56,0x78,0x90,0x12,0x34,0x56,0x78,0x90,0x12};
	AES_cbc_encrypt(cbEncodedMsg,cbEncodedMsg,32,&Kenc,cbIV,AES_ENCRYPT);
	// �������� ����������������� ���������
	//::memset(cbIV,0,16);
	//AES_set_decrypt_key(pKenc,nKencBitLength,&Kenc);
	//AES_cbc_encrypt(cbEncodedMsg,cbEncodedMsg,32,&Kenc,cbIV,AES_DECRYPT);
	// ****************************************************************************************
	// ������������ ���������
	// ****************************************************************************************
	std::vector<bool> Msg;
	std::vector<bool> T3Result;
	Msg=CT3Coder::ByteArrayToBooleanVector(cbEncodedMsg,32);
	CT3Coder T3C(Msg.size());
	T3Result=T3C.EncodeMessage(Msg);
	// ****************************************************************************************
	// ��������� �������� �� ����� ������
	// ****************************************************************************************
	Create(MaskLayer.GetWidth(),MaskLayer.GetHeight());
	GPE_INT i=0;
	m_RND.SetSeed(0xff10ff10);
	for(GPE_INT y=0;y<MaskLayer.GetHeight();y++)
	{
		for(GPE_INT x=0;x<MaskLayer.GetWidth();x++)
		{
			if(MaskLayer.GetPixel(x,y) == 0xFF)
			{
				// ****************************************************************************
				// ������������ �������������� ��������
				// ****************************************************************************
				SetPixel(x,y,T3Result[i++]?0xFF:0x00);
				if(i>=T3Result.size()) i=0;
			}
			else
			{
				SetPixel(x,y,ProtectionLayer.GetPixel(x,y));
			}
		}
	}
}
void CGPELayer::BitsToBmpVector(std::vector<std::vector<bool>>& BmpBits)
{
	BmpBits.clear();
	for(GPE_INT y=0;y<GetHeight();y++)
	{
		std::vector<bool> Row;
		for(GPE_INT x=0;x<GetWidth();x++)
		{
			Row.push_back(GetPixel(x,y)==0xFF);
		}
		BmpBits.push_back(Row);
	}
}

#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv/cv.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

using namespace std;
using namespace cv;


/// ��������
int CompareHist(const char* imagefile1, const char* imagefile2, double similar);
void SaveImg(IplImage* img, CvRect *rect, const char* file_name);
Point TemplateMatching(Mat* bigImg, Mat* smallImg, Mat* bigImgMatch, char* bigImgName, char* smallImgName);
void getTimeNow();
//��־
FILE *logger = fopen("point.log", "a");


int main(int argc, char** argv)
{
	getTimeNow();
	logger == NULL ? 0 : fprintf(logger, "*********************����ʼ����*********************\n");
	//�жϲ���
	if (argc < 3 || argc > 4)
	{
		if (logger != NULL)
		{
			getTimeNow();
			fprintf(logger, "��������: ");
			for (int i = 0; i < argc; i++)
			{
				fprintf(logger, "argv[%d]=%s ", i, argv[i]);
			}
			fprintf(logger, "\n");
		}
		printf("%d,%d", 0, 0);
		return 1;
	}
	
	if (logger != NULL)
	{
		getTimeNow();
		fprintf(logger, "������Ϣ: ");
		for (int i = 0; i < argc; i++)
		{
			fprintf(logger, "argv[%d]=%s ", i, argv[i]);
		}
		fprintf(logger, "\n");
	}

	//����ͼƬ����ͼ��Сͼ��ƥ���Ĵ�ͼ, ƥ���(Ĭ����0.9)
	Mat bigImg, smallImg, bigImgMatch;
	double similar = 0.9;
	Point clickPoint = Point(0, 0);//��������Ҫ���������

	//����ͼƬ,��ȡƥ���
	bigImg = imread(argv[1], 1);
	smallImg = imread(argv[2], 1);
	if (argc == 4)
	{
		similar = atof(argv[3]);
		if (similar > 1.0 || similar < 0.0)
		{
			getTimeNow();
			logger == NULL ? 0 : fprintf(logger, "����-ƥ���(%.4f) ����!(ƥ���Ϊ0��1)\n", similar);
			printf("%d,%d", 0, 0);
			return 1;
		}
	}
	if (bigImg.data == NULL)
	{
		//printf("%s����ʧ��!\n", argv[1]);
		getTimeNow();
		logger == NULL ? 0 : fprintf(logger, "ͼƬ(%s) ����ʧ��!\n", argv[1]);
		printf("%d,%d", 0, 0);
		return 1;
	}
	if (smallImg.data == NULL)
	{
		getTimeNow();
		logger == NULL ? 0 : fprintf(logger, "ͼƬ(%s) ����ʧ��!\n", argv[2]);
		printf("%d,%d", 0, 0);
		return 1;
	}

	getTimeNow();
	logger == NULL ? 0 : fprintf(logger, "������Ϣ: ��ͼ:%s, Сͼ:%s, ƥ���:%f\n", argv[1], argv[2], similar);

	//����ģ��ƥ���ͼ��Сͼ,����ƥ���Ĵ�ͼ����
	Point maxMatchLoc = TemplateMatching(&bigImg, &smallImg, &bigImgMatch, argv[1], argv[2]);

	//�ٽ���ͼ�е�Сͼ�ٳ���,����ΪtmpSmallImg.png
	IplImage *tmpBigImg = cvLoadImage(argv[1], -1);
	CvRect rect = { maxMatchLoc.x, maxMatchLoc.y, smallImg.cols, smallImg.rows };
	SaveImg(tmpBigImg, &rect, "tmpSmallImg.png");

	//���Ա�tmpSmallImg��smallImg����ɫֱ��ͼ
	int compareResult = CompareHist(argv[2], "tmpSmallImg.png", similar);

	if (compareResult == 1)
	{
		clickPoint.x = maxMatchLoc.x + smallImg.cols / 2;
		clickPoint.y = maxMatchLoc.y + smallImg.rows / 2;
	}
	printf("%d,%d", clickPoint.x, clickPoint.y);

	fclose(logger);
	return 0;
}

//opencvģ��ƥ��, �������ƥ������
Point TemplateMatching(Mat* bigImg, Mat* smallImg, Mat* bigImgMatch, char* bigImgName, char* smallImgName)
{
	//tmpResult�����ͼ��Сͼ�ĶԱ�����
	Mat tmpResult;
	//�Ƚ�bigImg������bigImgMatch��
	bigImg->copyTo(*bigImgMatch);
	//����tmpResult�Ĵ�С
	int tmp_cols = bigImg->cols - smallImg->cols + 1;
	int tmp_rows = bigImg->rows - smallImg->rows + 1;
	//ΪtmpResult�����ڴ�
	tmpResult.create(tmp_cols, tmp_rows, CV_32FC1);
	if (tmpResult.data == NULL)
	{
		//printf("�����ڴ�ʧ��!\n");
		getTimeNow();
		logger == NULL ? 0 : fprintf(logger, "ΪͼƬ�����ڴ�ʱ����!\n");
		return Point(0,0);
	}
	//opencvģ��ƥ�亯��(����ƽ����ƥ���㷨)���Ƚϴ�ͼ��Сͼ,�����ȽϽ����¼��tmpResult��
	matchTemplate(*bigImg, *smallImg, tmpResult, CV_TM_SQDIFF_NORMED);
	//��tmpResult��׼��
	normalize(tmpResult, tmpResult, 0, 1, NORM_MINMAX, -1, Mat());

	//��opencv minMaxLoc����,����tmpResult�е����ֵ����Сֵ,��ȡ���ƥ���
	double minVal; double maxVal; Point minLoc; Point maxLoc;
	Point matchLoc;
	minMaxLoc(tmpResult, &minVal, &maxVal, &minLoc, &maxLoc, Mat());
	//����ģ��ƥ��ʹ�õ�ƽ����ƥ���㷨,ƥ��Խ��,ƥ��ֵԽС
	matchLoc = minLoc;
	getTimeNow();
	logger == NULL ? 0 : fprintf(logger, "ģ��ƥ������ƥ���: %d,%d\n", matchLoc.x, matchLoc.y);
	//�ڴ�ͼ�ϵ����ƥ��㴦��һ����������ʾ���ƥ��λ��,��������ͼ�񱣴�(ͼ������ΪbigImgName+smallImgName+.png)
	char *tmp1 = strrchr(bigImgName, '\\'); tmp1 == 0 ? tmp1 = bigImgName : tmp1 = tmp1 + 1;
	char *tmp2 = strrchr(smallImgName, '\\'); tmp2 == 0 ? tmp2 = smallImgName : tmp2 = tmp2 + 1;
	char *bigImagMatchName = (char*)calloc((strlen(tmp1) + strlen(tmp2) + 6), sizeof(char));
	strncat(bigImagMatchName, tmp1, strlen(tmp1));
	strncat(bigImagMatchName, "+", strlen("+"));
	strncat(bigImagMatchName, tmp2, strlen(tmp2));
	strncat(bigImagMatchName, ".png", strlen(".png"));
	
	rectangle(*bigImgMatch, matchLoc, Point(matchLoc.x + smallImg->cols, matchLoc.y + smallImg->rows), Scalar(0, 0, 255), 2, 8, 0);
	imwrite(bigImagMatchName, *bigImgMatch);
	getTimeNow();
	logger == NULL ? 0 : fprintf(logger, "����ģ��ƥ����ͼƬ: %s\n", bigImagMatchName);
	free(bigImagMatchName);

	return matchLoc;
}

/// ���ƶȶԱ�
int CompareHist(const char* imagefile1, const char* imagefile2, double similar)
{
	//��ֱ��ͼ��  
	int HistogramBins = 256;
	float HistogramRange1[2] = { 0, 255 };
	float *HistogramRange[1] = { &HistogramRange1[0] };

	IplImage *image1 = cvLoadImage(imagefile1, 0);
	IplImage *image2 = cvLoadImage(imagefile2, 0);
	double result1, result2, result3, result4;

	CvHistogram *Histogram1 = cvCreateHist(1, &HistogramBins, CV_HIST_ARRAY, HistogramRange);
	CvHistogram *Histogram2 = cvCreateHist(1, &HistogramBins, CV_HIST_ARRAY, HistogramRange);

	cvCalcHist(&image1, Histogram1);
	cvCalcHist(&image2, Histogram2);

	cvNormalizeHist(Histogram1, 1);
	cvNormalizeHist(Histogram2, 1);


	result1 = cvCompareHist(Histogram1, Histogram2, CV_COMP_CHISQR);
	result2 = cvCompareHist(Histogram1, Histogram2, CV_COMP_BHATTACHARYYA);
	result3 = cvCompareHist(Histogram1, Histogram2, CV_COMP_CORREL);
	result4 = cvCompareHist(Histogram1, Histogram2, CV_COMP_INTERSECT);

	getTimeNow();
	logger == NULL ? 0 : fprintf(logger,
		"ģ��ƥ��Ľ����ԭͼƬ��ֱ��ͼ�Ա�: ���ż��� %.4f,���Ͼ��� %.4f,����� %.4f,ֱ��ͼ�ཻ %.4f\n", 
		result1, result2, result3, result4);
	cvReleaseImage(&image1);
	cvReleaseImage(&image2);
	cvReleaseHist(&Histogram1);
	cvReleaseHist(&Histogram2);

	//�������Ϲ������
	if (result3 >= similar || result4 >= similar)
	{
		return 1;
	}
	return 0;
}

//��ͼ
void SaveImg(IplImage* img, CvRect *rect, const char* file_name)
{
	assert(img != NULL);
	assert(rect != NULL);

	//����ͼƬROI
	cvSetImageROI(img,*rect);
	
	IplImage *dst = 0;   //Ŀ��ͼ��ָ��
	CvSize dst_cvsize;   //Ŀ��ͼ��ߴ�
	dst_cvsize.width = rect->width;
	dst_cvsize.height = rect->height;
	dst = cvCreateImage(dst_cvsize, img->depth, img->nChannels); //����Ŀ��ͼ��
	cvCopy(img, dst);
	cvResetImageROI(dst);
	cvSaveImage(file_name, dst);
	getTimeNow();
	logger == NULL ? 0 : fprintf(logger, "��ԭͼ�пٳ�ģ��ƥ���������򱣴�:%s\n", file_name);
}

//ϵͳʱ��
void getTimeNow()
{
	SYSTEMTIME systemTime;
	GetLocalTime(&systemTime);
	if (logger == NULL)
	{
		return;
	}
	fprintf(logger, "%d-%02d-%02d,%02d:%02d:%02d ",
		systemTime.wYear, systemTime.wMonth, systemTime.wDay, systemTime.wHour, systemTime.wMinute, systemTime.wSecond);
}
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv/cv.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

using namespace std;
using namespace cv;


/// 函数声明
int CompareHist(const char* imagefile1, const char* imagefile2, double similar);
void SaveImg(IplImage* img, CvRect *rect, const char* file_name);
Point TemplateMatching(Mat* bigImg, Mat* smallImg, Mat* bigImgMatch, char* bigImgName, char* smallImgName);
void getTimeNow();
//日志
FILE *logger = fopen("point.log", "a");


int main(int argc, char** argv)
{
	getTimeNow();
	logger == NULL ? 0 : fprintf(logger, "*********************程序开始运行*********************\n");
	//判断参数
	if (argc < 3 || argc > 4)
	{
		if (logger != NULL)
		{
			getTimeNow();
			fprintf(logger, "参数错误: ");
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
		fprintf(logger, "参数信息: ");
		for (int i = 0; i < argc; i++)
		{
			fprintf(logger, "argv[%d]=%s ", i, argv[i]);
		}
		fprintf(logger, "\n");
	}

	//保存图片：大图，小图，匹配后的大图, 匹配度(默认是0.9)
	Mat bigImg, smallImg, bigImgMatch;
	double similar = 0.9;
	Point clickPoint = Point(0, 0);//保存最终要点击的坐标

	//载入图片,读取匹配度
	bigImg = imread(argv[1], 1);
	smallImg = imread(argv[2], 1);
	if (argc == 4)
	{
		similar = atof(argv[3]);
		if (similar > 1.0 || similar < 0.0)
		{
			getTimeNow();
			logger == NULL ? 0 : fprintf(logger, "参数-匹配度(%.4f) 错误!(匹配度为0到1)\n", similar);
			printf("%d,%d", 0, 0);
			return 1;
		}
	}
	if (bigImg.data == NULL)
	{
		//printf("%s载入失败!\n", argv[1]);
		getTimeNow();
		logger == NULL ? 0 : fprintf(logger, "图片(%s) 载入失败!\n", argv[1]);
		printf("%d,%d", 0, 0);
		return 1;
	}
	if (smallImg.data == NULL)
	{
		getTimeNow();
		logger == NULL ? 0 : fprintf(logger, "图片(%s) 载入失败!\n", argv[2]);
		printf("%d,%d", 0, 0);
		return 1;
	}

	getTimeNow();
	logger == NULL ? 0 : fprintf(logger, "参数信息: 大图:%s, 小图:%s, 匹配度:%f\n", argv[1], argv[2], similar);

	//先用模板匹配大图和小图,并将匹配后的大图保存
	Point maxMatchLoc = TemplateMatching(&bigImg, &smallImg, &bigImgMatch, argv[1], argv[2]);

	//再将大图中的小图抠出来,保存为tmpSmallImg.png
	IplImage *tmpBigImg = cvLoadImage(argv[1], -1);
	CvRect rect = { maxMatchLoc.x, maxMatchLoc.y, smallImg.cols, smallImg.rows };
	SaveImg(tmpBigImg, &rect, "tmpSmallImg.png");

	//最后对比tmpSmallImg和smallImg的颜色直方图
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

//opencv模板匹配, 返回最佳匹配坐标
Point TemplateMatching(Mat* bigImg, Mat* smallImg, Mat* bigImgMatch, char* bigImgName, char* smallImgName)
{
	//tmpResult保存大图和小图的对比数据
	Mat tmpResult;
	//先将bigImg拷贝到bigImgMatch中
	bigImg->copyTo(*bigImgMatch);
	//计算tmpResult的大小
	int tmp_cols = bigImg->cols - smallImg->cols + 1;
	int tmp_rows = bigImg->rows - smallImg->rows + 1;
	//为tmpResult申请内存
	tmpResult.create(tmp_cols, tmp_rows, CV_32FC1);
	if (tmpResult.data == NULL)
	{
		//printf("申请内存失败!\n");
		getTimeNow();
		logger == NULL ? 0 : fprintf(logger, "为图片申请内存时出错!\n");
		return Point(0,0);
	}
	//opencv模板匹配函数(采用平方差匹配算法)来比较大图和小图,并将比较结果记录在tmpResult中
	matchTemplate(*bigImg, *smallImg, tmpResult, CV_TM_SQDIFF_NORMED);
	//对tmpResult标准化
	normalize(tmpResult, tmpResult, 0, 1, NORM_MINMAX, -1, Mat());

	//用opencv minMaxLoc函数,计算tmpResult中的最大值和最小值,获取最佳匹配点
	double minVal; double maxVal; Point minLoc; Point maxLoc;
	Point matchLoc;
	minMaxLoc(tmpResult, &minVal, &maxVal, &minLoc, &maxLoc, Mat());
	//由于模板匹配使用的平方差匹配算法,匹配越好,匹配值越小
	matchLoc = minLoc;
	getTimeNow();
	logger == NULL ? 0 : fprintf(logger, "模板匹配的最佳匹配点: %d,%d\n", matchLoc.x, matchLoc.y);
	//在大图上的最佳匹配点处画一个矩形来显示最佳匹配位置,并将最后的图像保存(图像名称为bigImgName+smallImgName+.png)
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
	logger == NULL ? 0 : fprintf(logger, "保存模板匹配后的图片: %s\n", bigImagMatchName);
	free(bigImagMatchName);

	return matchLoc;
}

/// 相似度对比
int CompareHist(const char* imagefile1, const char* imagefile2, double similar)
{
	//画直方图用  
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
		"模板匹配的结果和原图片的直方图对比: 卡放检验 %.4f,巴氏距离 %.4f,相关性 %.4f,直方图相交 %.4f\n", 
		result1, result2, result3, result4);
	cvReleaseImage(&image1);
	cvReleaseImage(&image2);
	cvReleaseHist(&Histogram1);
	cvReleaseHist(&Histogram2);

	//这儿就是瞎扯蛋的
	if (result3 >= similar || result4 >= similar)
	{
		return 1;
	}
	return 0;
}

//抠图
void SaveImg(IplImage* img, CvRect *rect, const char* file_name)
{
	assert(img != NULL);
	assert(rect != NULL);

	//设置图片ROI
	cvSetImageROI(img,*rect);
	
	IplImage *dst = 0;   //目标图像指针
	CvSize dst_cvsize;   //目标图像尺寸
	dst_cvsize.width = rect->width;
	dst_cvsize.height = rect->height;
	dst = cvCreateImage(dst_cvsize, img->depth, img->nChannels); //构造目标图象
	cvCopy(img, dst);
	cvResetImageROI(dst);
	cvSaveImage(file_name, dst);
	getTimeNow();
	logger == NULL ? 0 : fprintf(logger, "从原图中抠出模板匹配的最佳区域保存:%s\n", file_name);
}

//系统时间
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
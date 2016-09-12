/******************************************************************************

��Ȩ���� (C), 2016, �й���ѧԺ�����Զ����о�����һ�ң�������л����˿�����

******************************************************************************
�� �� ��   : v1.0
��	  ��   : Ф��
******************************************************************************/
#include "KNN_OCR.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <fstream>
#include <sys/stat.h>
#include <time.h>
#include "ros/ros.h"
#include <std_msgs/Float32.h>
#include <image_transport/image_transport.h>
#include <cv_bridge/cv_bridge.h>
#include <opencv2/opencv.hpp>
#include <pthread.h>
#include <boost/thread.hpp>
//#include <math.h>
//#include <linux/videodev2.h>

#define  OPENCV_VIDEOCAPTURE    0
#define  MVMODE                              1
#define  POINTGRAYCAMERA            2
#define  OFFLINEDATA                       3

using namespace std;
using namespace cv; 

class CapturedImage
{
public:
    Mat rawImg;
};

class RectMark
{
public:
	RectMark(void)
	{
		indexId = -1;
		validFlag = false;
		blackFrameDetectedFlag = false;
		minSideLength = 0;	
		maxSideLength = 0;	
		area = 0;
        rectKind = -1;
		//perspectiveImg.create(int(200*1.25),200,CV_8UC3);//create���У��У����ͣ�
		//possibleDigitBinaryImg.create(int(200*1.25),200,CV_8UC1);
	}
    int frameNo;
    int indexId;                    //Rect���
	bool validFlag;				//��Rect�Ƿ���Ч
	bool blackFrameDetectedFlag;
    //������𣬾��μ�⵽�������ȡֵ��0�� 1�� 2
    int rectKind;

	//contour��Ϣ
    float minSideLength;            //��С�߳�
    float maxSideLength;            //���߳�
    float area;                     //���
	vector<Point2f> m_points;	//�ĸ�����λ��
    Point3f position;

	//͸�ӱ任���ͼ��
	Mat perspectiveImg;
    //��͸�ӱ任��Ķ�ֵ�������ͼ��
    Mat possibleRectBinaryImg;
	//������ʶ��ͼ��
	Mat possibleDigitBinaryImg;
};

class Attitude3D
{
public:
    Attitude3D()
    {
        roll = 0;
        pitch = 0;
        yaw = 0;
    }
    float roll;
    float pitch;
    float yaw;
};

//���ս�����Զ�����������
class VisionResult
{
public:
    VisionResult( void )
    {
        frameNo = -1;
        digitNo = -1;
    }
    int frameNo;
    int digitNo;
    Point2f imagePos2D;
    Point3f cameraPos3D;
    Point3d negPos3D;
};

//ultrasonic data
class GuidanceDistance
{
public:
    double vbus_1_distance;
    double vbus_1_reliability;
    double vbus_2_distance;
    double vbus_2_reliability;
    double vbus_3_distance;
    double vbus_3_reliability;
    double vbus_4_distance;
    double vbus_4_reliability;
    double vbus_5_distance;
    double vbus_5_reliability;
};

//read offline data
int ReadOfflineData(  );
//����ros��Ϣ�����ĸ����߳�
void CreateRosPublishThread(const char* dir);
//����ros����ͼ���߳�
void*  RosImagePublishThread(void*);
//�����ļ���
int CreateDir(char* saveDir);
//Save
int CreatSaveDir (char* dir , bool saveImgFlag);
//SDK��ʽ��ʼ�����
//int MindvisionCaptureInit(void);

void ResizeImageByDistance( Mat& inputImg, Mat& outputImg, vector<VisionResult>& oldResult);
//���Σ��ı��Σ����
void RectangleDetect( Mat& resultImg, vector< vector<RectMark> >& rectCategory, int frameNo );
//���ı����ڲ�н�
double GetTwoSideAngle(Point2f p1,Point2f p2, Point2f p3);
//�޳��غϵ��ı���
void RectErase( vector<RectMark>& rectPossible );
//�ı��η���
void RectClassify( vector<RectMark>& rectPossible, vector< vector<RectMark> >& rectCategory);
//�ı��ΰ��������
void RectSortByArea( vector< vector<RectMark> >& rectCategory );
//��ͬ����ı��ΰ���������
void RectSortByPositionX( vector< vector<RectMark> >& rectCategory );
//�������ı���
void DrawAllRect(Mat& resultImg, vector< vector<RectMark> >& rectCategory);
//��͸�ӱ任���ͼ���������жϳ���⵽�ľ��ε����
void GetRectKinds( vector< vector<RectMark> >&  rectCategory );
//�ڿ���
void BlackFrameDetect(vector< vector<RectMark> >& rectCategory);
//�����Χ�ڿ������ƽ��ֵ
int GetBoxFramePixelAverageValue(const Mat& img);
//��Ŀλ�ù���
void EstimatePosition(Mat& srcColor, vector< vector<RectMark> >& rectCategory);
//͸�ӱ任
void PerspectiveTransformation(Mat& srcImg, vector<Mat>& rectCandidateImg, vector< vector<RectMark> >& rectCategory);
//����ʶ��
void DigitDetector(Mat& rectResultImg, basicOCR* ocr, vector< vector<RectMark> >& rectCategory, bool saveDigitBinaryImg);
//��ʾʱ�䡢֡��
void ShowTime(Mat& inputImg, int frameNo, float shrink);
//�жϴ�ʶ��ͼ�����Ч��
int FindValidContours(Mat& src);
//
double GetAllPixelAverageValue(Mat& img);
//�Ӿ����������Ϊtxt
void SaveResultToTxt(char* baseDir,  float shrink, vector<VisionResult>& result);
//Guidance
void DepthTo3D( Mat& depth_img, Mat& xyz_img);
void ConvertToPseudoColor( Mat& mat_xyz, Mat& img_pseudo_color );
//
double GetROI_AverageVal( Mat src, Point point, int channel, int radius);
//�����ۻ���������
int RectDectByStatisticsError(Mat& input_img);

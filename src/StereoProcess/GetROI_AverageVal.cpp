#include <opencv2/opencv.hpp>
using namespace cv;

//��ȡͼ��roi�����ƽ��ֵ
double GetROI_AverageVal( Mat src, Point point, int channel, int radius)
{
    Mat src_roi;
    int rect_x1=0;
    int rect_y1=0;
    int rect_x2=0;
    int rect_y2=0;
    rect_x1 = point.x-radius;
    rect_y1 = point.y-radius;
    rect_x2 = point.x+radius;
    rect_y2 = point.y+radius;

    //��ֹԽ��
    if(rect_x1<0)
    {
        rect_x1 = 0;
    }
    if(rect_y1<0)
    {
        rect_y1 = 0;
    }
    if(rect_x2 > src.cols-1)
    {
        rect_x2 = src.cols-1;
    }
    if(rect_y2 > src.rows-1)
    {
        rect_y2 = src.rows-1;
    }

    //ѡȡroi����
    src_roi = src(Rect(rect_x1,rect_y1, rect_x2 - rect_x1,rect_y2 - rect_y1));

    //ͳ��roi��������ֵ��ƽ��ֵ
    double sum = 0.0;
    int count = 0;
    for(int i=0; i<src_roi.rows; i++)
    {
        for(int j=0;j<src_roi.cols;j++)
        {
            double temp;
            if( CV_8UC1 == src.type() )
            {
                //8λ��ͨ��ucharͼ��
                temp = double( src_roi.at<uchar>(i,j) );
            }
            else if( CV_16SC1 == src.type() )
            {
                //8λ��ͨ��ushortͼ��
                temp = double( src_roi.at<ushort>(i,j) );
            }
            else if( CV_8UC3 == src.type() )
            {
                //8λ3ͨ��uchar��ͼ��
                Vec3b temp3b = src_roi.at<Vec3b>(i,j);
                temp = double(temp3b[channel]);
            }
            else if( CV_32FC3 == src.type() )
            {
                //32λ3ͨ��float��ͼ��
                Vec3f temp3f = src_roi.at<Vec3f>(i,j);
                temp = double(temp3f[channel]);
            }
            if( fabs(temp) > 0.0001)
            {
                //ֻͳ�Ʋ�Ϊ�����ص�
                sum += temp;
                count++;
            }
        }
    }
    if( count>0 )
        return sum/count;
    else
        return 0;
}

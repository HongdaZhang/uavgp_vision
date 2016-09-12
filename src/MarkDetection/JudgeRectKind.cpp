/******************************************************************************

��Ȩ����:  2016, �й���ѧԺ�����Զ����о�����һ�ң�������л����˿�����

******************************************************************************
��	  ��   : Ф��
******************************************************************************/
#include "declare.h"


char windowName[100];
//�����а�Χ���ΰ�����������򣬴����ǰ
void BoundingRectSortByAreaSize( vector< Rect>& minBoundingRect );
//�԰�Χ���ε��������ж�
int RectKind(  Mat& inputImg, Mat& outputImg, vector< Rect>& minBoundingRect  );
//��������֮�������ƽ��ֵ
double GetPixelAverageValueBetweenTwoRect( Mat inputImg, Mat outputImg, Rect& rect_outside, Rect& rect_inside);
//���������洢��������ͼ��
void GetDigitRoiImg( Mat& binaryImg, vector< Rect>& minBoundingRect, int rectKind,  vector<RectMark>&  rectCategory );

Mat img;
//��͸�ӱ任���ͼ���������жϳ���⵽�ľ��ε����
void GetRectKinds( vector< vector<RectMark> >&  rectCategory )
{
    for (int i=0; i<(int)rectCategory.size(); ++i)
    {
        rectCategory[i][0].perspectiveImg.copyTo( img );
        Mat showImg = img.clone();

        Mat srcGray;
        cvtColor(img,srcGray,CV_BGR2GRAY);
        Mat imgBinary;
        int min_size = 100;
        int thresh_size = (min_size/4)*2 + 1;
        adaptiveThreshold(srcGray, imgBinary, 255, ADAPTIVE_THRESH_GAUSSIAN_C, THRESH_BINARY_INV, thresh_size, thresh_size/3); //THRESH_BINARY_INV
        //morphologyEx(imgBinary, imgBinary, MORPH_CLOSE, Mat());

        ////������ӳ��ǰ�ľ��δ�С�����ñ������windowsize
        ////�����......
        Mat element=getStructuringElement(MORPH_ELLIPSE, Size(7,7) ); //MORPH_RECT=0, MORPH_CROSS=1, MORPH_ELLIPSE=2
        morphologyEx(imgBinary, imgBinary, MORPH_CLOSE ,element);
        imgBinary.copyTo(rectCategory[i][0].possibleRectBinaryImg);

        sprintf(windowName,"imgBinary-%d",i);
        //imshow(windowName, imgBinary);
        Mat img_bin = imgBinary.clone();

        vector< vector<Point> > contours;
        vector<Vec4i> hierarchy;
        vector<RotatedRect>  box;
        vector<Rect> minBoundingRect;
        minBoundingRect.push_back( Rect(0,0,img.cols,img.rows) );
        //��������
        findContours( img_bin, contours, hierarchy ,CV_RETR_CCOMP, CV_CHAIN_APPROX_NONE );//CV_RETR_CCOMP ; CV_RETR_EXTERNAL
        //cout<<"�����ܸ���=" << all_contours.size() <<endl;
        for (int k = 0; k < (int)contours.size(); ++k)
        {
            if ( (int)contours[k].size() < img.cols || fabs( (float)contourArea(contours[k])<pow( img.cols*0.15,2) )  )
            {
                continue;
            }

            //RotatedRect boxTemp = minAreaRect( Mat(contours[i]) );
            //Point2f vertex[4];
            //boxTemp.points(vertex);
            //����������С����İ�Χ����
            //for(int j=0; j<4;++j)
            //{
                //line(showImg, vertex[j], vertex[(j+1)%4], Scalar(0,255,0), 2, 8);
            //}

            //��С��Χ��������
            Rect minBoundingRectTemp = boundingRect( Mat(contours[k]) );
            //����ɫ�����������Χ����
            rectangle( showImg, minBoundingRectTemp, Scalar(255,0,0), 2, 8);
            //ֻ�洢���ĵ���ͼ�����ĵľ���
            Point2d minBoundingRectMiddlePoint = Point2d( minBoundingRectTemp.x + minBoundingRectTemp.width*0.5, minBoundingRectTemp.y + minBoundingRectTemp.height*0.5 );
            double err = sqrt( pow(minBoundingRectMiddlePoint.x - img.cols*0.5, 2) + pow(minBoundingRectMiddlePoint.y - img.rows*0.5, 2) );
            if ( err < img.cols*0.06)
            {
                if ( minBoundingRectTemp.height < img.rows*0.9 )
                {
                    minBoundingRect.push_back(minBoundingRectTemp);
                }
            }
        }
        //�����򲻸���Ȥ
        if ( minBoundingRect.size() <= 1)
        {
            continue;
        }

        //��Χ���ΰ��������
        BoundingRectSortByAreaSize( minBoundingRect );

        for (int j=0;j<(int)minBoundingRect.size(); ++j)
        {
            //�ú�ɫ�����������ĵ���ͼ�����ĵ��������
            rectangle( showImg, minBoundingRect[j], Scalar(0,0,255), 2, 8);
            circle(showImg, Point(minBoundingRect[j].x+minBoundingRect[j].width*0.5, minBoundingRect[j].y+minBoundingRect[j].height*0.5), 2, Scalar(0,0,255), -1, 8);
        }

        //�������жϾ�������
        int rectKind = RectKind( imgBinary, showImg, minBoundingRect );
        GetDigitRoiImg( imgBinary, minBoundingRect, rectKind,  rectCategory[i] );

        imshow("minRect", showImg);
    }
    return;
}

//�����а�Χ���ΰ�����������򣬴����ǰ
void BoundingRectSortByAreaSize( vector< Rect>& minBoundingRect )
{
        for (int i=0;i<(int)minBoundingRect.size();++i)
        {
            for (int j=i+1;j<(int)minBoundingRect.size();++j)
            {
                float width = (minBoundingRect[j].width > minBoundingRect[i].width) ? minBoundingRect[j].width : minBoundingRect[i].width;
                if (abs(minBoundingRect[j].width - minBoundingRect[i].width) < width*0.1)
                {
                    minBoundingRect.erase(minBoundingRect.begin() + j);
                    j--;
                    continue;
                }
                if ( (minBoundingRect[j].width * minBoundingRect[j].height) > (minBoundingRect[i].width * minBoundingRect[i].height) )
                {
                    swap(minBoundingRect[j], minBoundingRect[i]);
                }
            }
        }
    return;
}

//�԰�Χ���ε��������ж�
int RectKind(  Mat& inputImg, Mat& outputImg, vector< Rect>& minBoundingRect  )
{
    int kind = -1;
    double rectOutside_rectInside_pixelValue = GetPixelAverageValueBetweenTwoRect( inputImg, outputImg, minBoundingRect[0], minBoundingRect[1]);
    //printf("rectOutside_rectInside_pixelValue = %f\n", rectOutside_rectInside_pixelValue);

    //�þ���Ϊ�ڶ����������⵽�����м����
    if (rectOutside_rectInside_pixelValue > 100 && minBoundingRect.size() >=3)
    {
        kind = 1;
        //��Ϊ��ȫ�ڵĺڿ�
        rectangle(outputImg, minBoundingRect[0], Scalar(0,0,0), 4, 8);
        rectangle(outputImg, minBoundingRect[1], Scalar(0,0,0), 4, 8);
    }
    //����Ϊ��һ�����������
    else if (rectOutside_rectInside_pixelValue < 50 )
    {
        //��Ϊ��ȫ�׵İ׿�
        rectangle(outputImg, minBoundingRect[0], Scalar( 255, 255, 255 ), 4, 8);
        rectangle(outputImg, minBoundingRect[1], Scalar( 255, 255, 255 ), 4, 8);
        //���ֻ������������Σ��϶��ǵ�һ�����, ���ڲ����
        if (minBoundingRect.size() <= 2)
        {
            kind = 0;
        }
        else if (3 == minBoundingRect.size() )
        {
            //����������0
            kind = 0;
        }
        //�������������⵽������ߵľ���
        else
        {
            double widthRatio0 = double(minBoundingRect[0].width)/minBoundingRect[1].width;
            double heightRatio0 = double(minBoundingRect[0].height)/minBoundingRect[1].height;
            double widthRatio1 = double(minBoundingRect[1].width)/minBoundingRect[2].width;
            double heightRatio1 = double(minBoundingRect[1].height)/minBoundingRect[2].height;
            double widthError0 = fabs( widthRatio0 - 60.0/40);  //
            double heightError0 = fabs( heightRatio0 - 70.0/50);
            double widthError1 = fabs( widthRatio1 - 40.0/30);
            double heightError1 = fabs( heightRatio1 - 50.0/40);
            if (widthError0 < 60.0/40*0.1 && heightError0<70.0/50*0.1)
            {
                if ( widthError0 < 40.0/30*0.1 && heightError0 < 50.0/40*0.1 )
                {
                    kind = 2;
                }
            }
        }
    }
    return kind;
}

//���������, �洢��������ͼ��
void GetDigitRoiImg( Mat& binaryImg, vector< Rect>& minBoundingRect, int rectKind,  vector<RectMark>&  rectCategory_i )
{

    Mat img;
     Rect roi;

    if ( -1 == rectKind)
    {
        return;
     }
    if ( 0 == rectKind)
    {
        roi = minBoundingRect[1];
     }
    if ( 1 == rectKind)
    {
        roi = minBoundingRect[2];
     }
    if ( 2 == rectKind)
    {
        roi = minBoundingRect[3];
     }

        //roi��������1.1��
        Point middlePoint = Point(roi.x + roi.width/2, roi.y + roi.height/2);
        roi.height = roi.height * 1.1;  //1.1
        roi.width = roi.height * 2.7/4;
        roi.x = middlePoint.x - roi.width/2 * 1.0;
        roi.y = middlePoint.y - roi.height/2;

        if ( roi.x <= 0 )
            roi.x = 0;
        if ( roi.y <= 0 )
            roi.y = 0;
        if ( roi.x + roi.width >= binaryImg.cols )
            roi.width =  binaryImg.cols - roi.x;
        if ( roi.y + roi.height >= binaryImg.rows )
            roi.height = binaryImg.rows - roi.y;

        //��ֵ����ת
        binaryImg( roi ).copyTo(img);
        //imshow( "roi", img );
        threshold(img, img, 125, 255, THRESH_BINARY_INV);

        resize(img, img, Size(128,128));
        Mat element=getStructuringElement(MORPH_ELLIPSE, Size(7,7) );
        erode( img, img, element);
        rectCategory_i[0].possibleDigitBinaryImg = img.clone();
        rectCategory_i[0].rectKind = rectKind;

        //imshow("digitROI", img);
    return;
}


//��������Χ����֮�������ƽ��ֵ
double GetPixelAverageValueBetweenTwoRect( Mat inputImg, Mat outputImg, Rect& rect_outside, Rect& rect_inside)
{
    double totalValue = 0;
    int count = 0;
    for (int i=0;i<inputImg.rows;++i)
    {
        uchar* data= inputImg.ptr<uchar>(i);
        //���º���
        if ( (i>= rect_outside.y && i<=rect_inside.y) || ( i >= (rect_inside.y + rect_inside.height) && i <= (rect_outside.y + rect_outside.height) ) )
        {
            for (int j=0;j<inputImg.cols;++j)
            {
                    totalValue += data[j];
                    count++;
                    //outputImg.at<Vec3b>( i,j )[0] = 255;
                    //outputImg.at<Vec3b>( i,j )[1] = 0;
                    //outputImg.at<Vec3b>( i,j )[2] = 0;
            }
        }
        //��������
        if (  i > rect_inside.y  &&  i < (rect_inside.y + rect_inside.height) )
        {
            for (int j=0;j<inputImg.cols;++j)
            {
                if ( (j> rect_outside.x && j<rect_inside.x) || ( j > (rect_inside.x + rect_inside.width) && j < (rect_outside.x + rect_outside.width) ) )
                {
                    totalValue += data[j];
                    count++;
                    //outputImg.at<Vec3b>( i,j )[0] = 255;
                    //outputImg.at<Vec3b>( i,j )[1] = 0;
                    //outputImg.at<Vec3b>( i,j )[2] = 0;
                }
            }
        }
    }

    return (totalValue/count);
}

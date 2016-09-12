#include "DetectRectToGetImageLightness.h"
//���γ��������ֵ
static float maxSideLengthRatioAllowed = 2.5f;
static float rectClassifyThres = 1.0f;

DetectRectToGetImageLightness::DetectRectToGetImageLightness()
{
    shrink = 0.23;
    frameNo = -1;
    rect_area_lightness = -1;
    maxDistance = 2.5;
    minDistance = 0.6;
    return;
}


//���Σ��ı��Σ����
void DetectRectToGetImageLightness::RectangleDetect( )
{
    //Mat gaussianImg;
    //GaussianBlur(resultImg, gaussianImg, Size(5,5),2,2);
    //imshow("gauss",gaussianImg);
    //Canny(gaussianImg,gaussianImg,80,40);
    //imshow("canny",gaussianImg);
    //waitKey(0);

    Mat srcGray;
    cvtColor(resultImg,srcGray,CV_BGR2GRAY);
    Mat imgBinary;
    int min_size = 100; //100
    int thresh_size = (min_size/4)*2 + 1;
    adaptiveThreshold(srcGray, imgBinary, 255, ADAPTIVE_THRESH_GAUSSIAN_C, THRESH_BINARY_INV, thresh_size, thresh_size/3); //THRESH_BINARY_INV
    //morphologyEx(imgBinary, imgBinary, MORPH_OPEN, Mat());
    //printf("adaptiveThreshold done!\n");

    double s = 5*shrink;   //* (640)
    int size = ( 1 == int(s)%2 ) ? int(s) : int(s)+1;
    if (size < 3)
        size = 3;
    Mat element;
    //if (oldResult.size()>0 && oldResult[0].position3D.z<1)
    //{
    //    element=getStructuringElement(MORPH_ELLIPSE, Size( size,size ) );  //Size( 9,9 ) //MORPH_RECT=0, MORPH_CROSS=1, MORPH_ELLIPSE=2
    //}
    //else
    //{
    //    element=getStructuringElement(MORPH_RECT, Size( size,size ) );  //Size( 9,9 ) //MORPH_RECT=0, MORPH_CROSS=1, MORPH_ELLIPSE=2
    //}

    element=getStructuringElement(MORPH_RECT, Size( size,size ) );
    morphologyEx(imgBinary, imgBinary, MORPH_CLOSE ,element);

    Mat imgBinaryShow;
    //resize(imgBinary, imgBinaryShow, Size(640,480),0,0,INTER_AREA);
    imshow("captureAdaptiveThresholdImg",imgBinary);
    //printf("imshow imgBinaryShow done!\n");

    //vector< Point > hull;	//͹����
    vector<Vec4i>hierarchy;
    vector< vector<Point> >all_contours;
    vector< vector<Point> >contours;
    //��������
    //findContours( imgBinary, all_contours, hierarchy ,RETR_LIST, CHAIN_APPROX_NONE );//CV_RETR_CCOMP ; CV_RETR_EXTERNAL
    findContours( imgBinary, all_contours, RETR_LIST, CHAIN_APPROX_NONE );//CV_RETR_CCOMP ; CV_RETR_EXTERNAL
    //printf("Contours number before filter: %d\n", int(all_contours.size()) );

    //���˵�̫С������
    for (int i = 0; i < (int)all_contours.size(); ++i)
    {
        if ((int)all_contours[i].size() > srcGray.cols/40*4 && (int)all_contours[i].size()<srcGray.rows*4)
        {
            float area = fabs( (float)contourArea(all_contours[i]) );
            if (area > pow((float)srcGray.rows/40,2))
                contours.push_back(all_contours[i]);
        }
    }
    //cout<<"contours num=" << contours.size() <<endl;

    //�ı���ɸѡ
    vector<Point> approxCurve;
    int id = 0;
    for (int i=0; i<(int)contours.size(); ++i)
    {
        //��Ͼ���
        double fitting_accuracy = 0.015;    //0.005
        //���ƶ���αƽ�
        approxPolyDP(contours[i], approxCurve, double(contours[i].size())*fitting_accuracy, true);	//double(contours[i].size())*0.05
        //��4���β�����Ȥ
        if (approxCurve.size() != 4)
            continue;
        //��͹4���β�����Ȥ
        if (!isContourConvex(approxCurve))
            continue;
        //�ĸ���������˳ʱ�룺0��1��2��3�����Ͻ�Ϊ0��
        for(int m=0;m<(int)approxCurve.size();++m)
        {
            for (int n=m+1;n<(int)approxCurve.size();++n)
            {
                if (approxCurve[m].y > approxCurve[n].y)
                    std::swap(approxCurve[m], approxCurve[n]);
            }
        }
        if (approxCurve[0].x > approxCurve[1].x)
            std::swap(approxCurve[0], approxCurve[1]);
        if (approxCurve[3].x > approxCurve[2].x)
            std::swap(approxCurve[2], approxCurve[3]);

        // ���ı��ε���С�ߡ�����
        float minDist = float(1384 * shrink);
        float maxDist = 0.0f;
        float sideLength[4] = {0};
        for (int n=0; n<(int)approxCurve.size(); ++n)
        {
            sideLength[n] = sqrt( pow(float(approxCurve[n].x - approxCurve[(n+1)%4].x),2)+pow(float(approxCurve[n].y - approxCurve[(n+1)%4].y),2) );
            if (sideLength[n] < minDist)
            {
                minDist = sideLength[n];
            }
            if (sideLength[n] > maxDist)
            {
                maxDist = sideLength[n];
            }
        }
        //�������Σ�0��2�ű߳�ӦС��1��3�ű߳�
        float minLength = sideLength[0];
        if (minLength > sideLength[2])
        {
            minLength = sideLength[2];
        }
        if (minLength > sideLength[1] || minLength > sideLength[3] )
        {
            continue;
        }
        //���߼нǲ���̫С
        double angle0 = GetTwoSideAngle(approxCurve[3],approxCurve[0], approxCurve[1]);
        double angle1 = GetTwoSideAngle(approxCurve[0],approxCurve[1], approxCurve[2]);
        double angle2 = GetTwoSideAngle(approxCurve[1],approxCurve[2], approxCurve[3]);
        double angle3 = GetTwoSideAngle(approxCurve[2],approxCurve[3], approxCurve[0]);
        double minAngleThres = 90 - 30;
        double maxAngleThres = 90 + 30;
        //�������ǲ���ͬʱ����90��
        if( (angle0>100 && angle1>100) || (angle1>100 && angle2>100) || (angle2>100 && angle3>100) || (angle3>100 && angle0>100) )
        {
            continue;
        }
        if ( (angle0<minAngleThres || angle0>maxAngleThres) || (angle1<minAngleThres || angle1>maxAngleThres)
                || (angle2<minAngleThres || angle2>maxAngleThres) || (angle3<minAngleThres || angle3>maxAngleThres) )
        {
            continue;
        }
        ////����������ƽ���ı���
        //double angle0_0 = GetTwoSideAngle(approxCurve[1],approxCurve[0],Point2f(approxCurve[1].x,approxCurve[0].y) );
        //double angle1_0 = GetTwoSideAngle(approxCurve[0],approxCurve[1],Point2f(approxCurve[1].x,approxCurve[0].y) );
        //if (angle0<80 && angle1>100 && angle0_0>20 && angle1_0>20)
        //{
        //	continue;
        //}
        //�ı�����̱߲���̫С
        float m_minSideLengthAllowed = float(srcGray.rows/40);
        //�������̱�֮�Ȳ��ɹ�С
        float m_maxSideLengthRatio = maxDist/minDist;
        //���˲��洢��Ч���ı�����Ϣ
        if (minDist > m_minSideLengthAllowed && m_maxSideLengthRatio < maxSideLengthRatioAllowed)
        {
            RectMark markTemp;
            for (int n=0; n<4; ++n)
            {
                markTemp.m_points.push_back( Point2f( (float)approxCurve[n].x, (float)approxCurve[n].y ) );
            }
            markTemp.area = fabs( (float)contourArea(contours[i]) );
            markTemp.minSideLength = minDist;
            markTemp.maxSideLength = maxDist;
            markTemp.validFlag = true;
            markTemp.indexId = id;
            markTemp.frameNo = frameNo;
            id++;
            rectPossible.push_back(markTemp);
        }
     }

    //�޳��غϵ��ı���
    RectErase( );
    //�ı��η���
    RectClassify();
    //���������,larger one in front
    RectSortByArea( );
    //��ͬ����ı��ΰ���������
    RectSortByPositionX( );
    //�������ı���
    DrawAllRect( );


    return;
}



//���ı����ڲ�н�
double DetectRectToGetImageLightness::GetTwoSideAngle(Point2f p1,Point2f p2, Point2f p3)
{
    //vector1
    double xV1 = p2.x-p1.x;
    double yV1 =p2.y - p1.y;
    //vector2
    double xV2 = p3.x - p2.x;
    double yV2 = p3.y - p2.y;
    if ((0==xV1 && 0 ==yV1) || (0 == xV2 && 0 == yV2))
        return 0;
    else
        return ( acos((xV1*xV2 + yV1*yV2) / sqrt((xV1*xV1 + yV1*yV1)*(xV2*xV2 + yV2*yV2))) * 180 / 3.1415926 );
}


//�޳��غϵ��ı���
void DetectRectToGetImageLightness::RectErase( )
{
    float rectErr[4] = {0};
    float maxErr = 5;
    for (int i=0; i<(int)rectPossible.size(); ++i)
    {
        for (int j=i+1; j<(int)rectPossible.size(); ++j)
        {
            for (int k=0; k<4; ++k)
            {
                //�����������ζ�Ӧ�ĵ�k����������ؾ���
                rectErr[k] = sqrt(pow((rectPossible[i].m_points[k].x - rectPossible[j].m_points[k].x),2)
                                + pow((rectPossible[i].m_points[k].y - rectPossible[j].m_points[k].y),2));
                if (rectErr[k] > maxErr)
                    break;
            }
            if (rectErr[0] <= maxErr  &&  rectErr[1] <= maxErr  &&  rectErr[2] <= maxErr  &&  rectErr[3] <= maxErr)
            {
                //ȡ�ظ������ĸ������ƽ��ֵ��Ϊ��������
                for (int k=0; k<4; k++)
                {
                    rectPossible[j].m_points[k].x = (rectPossible[i].m_points[k].x + rectPossible[j].m_points[k].x)/2;
                    rectPossible[j].m_points[k].y = (rectPossible[i].m_points[k].y + rectPossible[j].m_points[k].y)/2;
                }
                //�޳��ظ����εĶ���
                rectPossible.erase(rectPossible.begin() + j);
                j--;
                break;
            }
        }
    }
    return;
}


//�ı��η���
void DetectRectToGetImageLightness::RectClassify()
{
    vector<RectMark> rectArrayTemp;
    for (int i=0;i<(int)rectPossible.size();++i)
    {
        rectArrayTemp.push_back(rectPossible[i]);

        Point2f middlePoint_i = Point2f((rectPossible[i].m_points[0].x+rectPossible[i].m_points[2].x)/2,
                                        (rectPossible[i].m_points[0].y+rectPossible[i].m_points[2].y)/2);
        for (int j=i+1;j<(int)rectPossible.size();++j)
        {
            Point2f middlePoint_j = Point2f((rectPossible[j].m_points[0].x+rectPossible[j].m_points[2].x)/2,
                                            (rectPossible[j].m_points[0].y+rectPossible[j].m_points[2].y)/2);
            //�����ı��ζԽ����е�ľ���
            float twoPointDistance = sqrt(pow(middlePoint_i.x-middlePoint_j.x,2)+pow(middlePoint_i.y-middlePoint_j.y,2));
            float minSide = 0;
            if (rectPossible[i].minSideLength < rectPossible[j].minSideLength)
                minSide = rectPossible[i].minSideLength;
            else
                minSide = rectPossible[j].minSideLength;

            //�ж����ı����Ƿ�����Ϊͬһ�����־
            if (twoPointDistance < (minSide * rectClassifyThres))
            {
                rectArrayTemp.push_back(rectPossible[j]);
                rectPossible.erase(rectPossible.begin() + j);
                j--;
            }
        }
        //ͬһ��ķ���һ��
        rectCategory.push_back(rectArrayTemp);
    }
    return;
}

//ͬһ���ڵ��ı��ΰ��������
void DetectRectToGetImageLightness::RectSortByArea( )
{
    for (int k=0;k<(int)rectCategory.size();++k)
    {
        //ͬһ��������
        for (int i=0;i<(int)rectCategory[k].size();++i)
        {
            for (int j=i+1;j<(int)rectCategory[k].size();++j)
            {
                //put the larger front
                if (rectCategory[k][j].area > rectCategory[k][i].area)
                {
                    swap(rectCategory[k][i],rectCategory[k][j]);
                }
            }
        }
    }
    return;
}

//��ͬ����ı��ΰ���������
void DetectRectToGetImageLightness::RectSortByPositionX( )
{
        //ͬһ��������
        for (int i=0;i<(int)rectCategory.size();++i)
        {
            for (int j=i+1;j<(int)rectCategory.size();++j)
            {
                if (rectCategory[j][0].m_points[0].x < rectCategory[i][0].m_points[0].x)
                {
                    swap(rectCategory[j],rectCategory[i]);
                }
            }
        }
    return;
}

//�������ı���
void DetectRectToGetImageLightness::DrawAllRect( )
{
    //image = Mat::zeros(480, 640, CV_8UC3);
    for (int k=0;k<(int)rectCategory.size();++k)
    {
        for (int i=0; i<(int)rectCategory[k].size(); i++)
        {
            int b = (unsigned)theRNG() & 255;
            int g = (unsigned)theRNG() & 255;
            int r = (unsigned)theRNG() & 255;
            line(resultImg, rectCategory[k][i].m_points[0], rectCategory[k][i].m_points[1], Scalar(255,255,0), 2, 8);
            line(resultImg, rectCategory[k][i].m_points[1], rectCategory[k][i].m_points[2], Scalar(255,255,0), 2, 8);
            line(resultImg, rectCategory[k][i].m_points[2], rectCategory[k][i].m_points[3], Scalar(255,255,0), 2, 8);
            line(resultImg, rectCategory[k][i].m_points[3], rectCategory[k][i].m_points[0], Scalar(255,255,0), 2, 8);
            if (0 == i)
            {
                //ֻ�����ڲ��ı����ϱ��0��1��2��3
                for (int j=0;j<4;j++)
                {
                    circle(resultImg,rectCategory[k][i].m_points[j],4,Scalar(0,255,0),-1);
                    char strNumber[200];
                    sprintf( strNumber,"%d",j);
                    putText(resultImg,strNumber,rectCategory[k][i].m_points[j],CV_FONT_HERSHEY_COMPLEX_SMALL,1.0,Scalar(0,255,0),1);
                }
            }
        }
    }
    //cout<<"totalRect="<<rectCategory.size()<<endl;
    //imshow("captureRectDetect",resultImg);
    return;
}


//��Ŀλ�ù���
void DetectRectToGetImageLightness::EstimatePosition()
{
    //��ͼ������ʾƽ��������x,y,z��
    Mat imgColor;
    imgColor = resultImg;

    vector<Point2f> imagePoints2d(4);
    vector<Point3f> objectPoints3d(4);
    const static double kind_0_width = 0.3;
    const static double kind_0_height = 0.4;
    const static double kind_1_width = 0.4;
    const static double kind_1_height = 0.5;
    const static double kind_2_width = 0.6;
    const static double kind_2_height = 0.7;
    double realRectHalfWidth;
    double realRectHalfHeight;

    for (int i=0; i<(int)rectCategory.size(); ++i)
    {
        realRectHalfWidth  = kind_1_width/2;
        realRectHalfHeight = kind_1_height/2;

        //Ŀ��������ĸ�����������
        objectPoints3d[0]=Point3d(-realRectHalfWidth, -realRectHalfHeight, 0);
        objectPoints3d[1]=Point3d(realRectHalfWidth,  -realRectHalfHeight, 0);
        objectPoints3d[2]=Point3d(realRectHalfWidth,   realRectHalfHeight, 0);
        objectPoints3d[3]=Point3d(-realRectHalfWidth,  realRectHalfHeight, 0);

        //Ŀ����ζ�Ӧͼ������0~3
        imagePoints2d[0]=Point2d(rectCategory[i][0].m_points[0].x,rectCategory[i][0].m_points[0].y);
        imagePoints2d[1]=Point2d(rectCategory[i][0].m_points[1].x,rectCategory[i][0].m_points[1].y);
        imagePoints2d[2]=Point2d(rectCategory[i][0].m_points[2].x,rectCategory[i][0].m_points[2].y);
        imagePoints2d[3]=Point2d(rectCategory[i][0].m_points[3].x,rectCategory[i][0].m_points[3].y);


        //Point Gray Flea3-03s2c
        //double t_fx=557.24612 *1384*shrink/640, t_fy=555.36689 *1384*shrink/640, t_cx=313.54129 *1384*shrink/640, t_cy=240.08147 *1384*shrink/640;
        //Mat t_distcoef=(Mat_<double>(1,5) << -0.04746, 0.07647, 0.00038, 0.00167,0);
        //Mat t_cameraMatrix=(Mat_<double>(3,3) << t_fx,0,t_cx,0,t_fy,t_cy,0,0,1);

        //Point Gray Flea3-14s3c
        double t_fx=893.25550*shrink, t_fy=894.63039*shrink, t_cx=686.67029*shrink, t_cy=518.99170*shrink;
        Mat t_distcoef=(Mat_<double>(1,5) << -0.05173, 0.07077, -0.00047, 0.00061,0);
        Mat t_cameraMatrix=(Mat_<double>(3,3) << t_fx,0,t_cx,0,t_fy,t_cy,0,0,1);

        //������ת����ƽ�ƾ���
        Mat rvec,tvec;
        solvePnP(objectPoints3d,imagePoints2d,t_cameraMatrix,t_distcoef,rvec,tvec);

        double tvec_x,tvec_y,tvec_z;
        tvec_x=tvec.at<double>(0,0);
        tvec_y=tvec.at<double>(1,0);
        tvec_z=tvec.at<double>(2,0);
        rectCategory[i][0].position = Point3d( tvec_x,tvec_y,tvec_z );
        //�޳���Զ�������
        if(tvec_z>maxDistance || tvec_z<minDistance)
        {
            rectCategory.erase(rectCategory.begin()+i);
            i--;
            continue;
        }

        //draw the rect
        line(resultImg, rectCategory[i][0].m_points[0], rectCategory[i][0].m_points[1], Scalar(0,255,255), 3, 8);
        line(resultImg, rectCategory[i][0].m_points[1], rectCategory[i][0].m_points[2], Scalar(0,255,255), 3, 8);
        line(resultImg, rectCategory[i][0].m_points[2], rectCategory[i][0].m_points[3], Scalar(0,255,255), 3, 8);
        line(resultImg, rectCategory[i][0].m_points[3], rectCategory[i][0].m_points[0], Scalar(0,255,255), 3, 8);
        //ֻ�����ڲ��ı����ϱ��0��1��2��3
        for (int j=0;j<4;j++)
        {
             circle(resultImg,rectCategory[i][0].m_points[j],4,Scalar(0,0,255),-1);
             char strNumber[200];
             sprintf( strNumber,"%d",j);
             putText(resultImg,strNumber,rectCategory[i][0].m_points[j],CV_FONT_HERSHEY_COMPLEX_SMALL,1.0,Scalar(0,0,255),1);
        }

        char transf[50];
        //sprintf_s(transf,"T%u:[%0.3fm,%0.3fm,%0.3fm]",i,tvec_y,tvec_x,tvec_z);
        sprintf(transf,"%0.3fm",tvec_z);
        Point T_showCenter;
        T_showCenter=Point2d(rectCategory[i][0].m_points[1].x,(rectCategory[i][0].m_points[1].y + rectCategory[i][0].m_points[2].y)/2);
        putText(imgColor, transf, T_showCenter,CV_FONT_HERSHEY_PLAIN,1.5,Scalar(0,0,255),int(2.5));
    }
    //imshow("distance",imgColor);
    return;
}

void DetectRectToGetImageLightness::GetTheLargestRect()
{
        for (int i=0;i<(int)rectCategory.size();++i)
        {
            for (int j=i+1;j<(int)rectCategory.size();++j)
            {
                //put the larger front
                if (rectCategory[j][0].area > rectCategory[i][0].area)
                {
                    swap(rectCategory[i],rectCategory[j]);
                }
            }
        }
    return;
}

//͸�ӱ任
void DetectRectToGetImageLightness::PerspectiveTransformation( )
{
    char windowName[50];
    vector<Point2f> imagePoints2d(4);
    for (int i=0; i<1; ++i)
    {
        int minLoopCount = ((int)rectCategory[i].size()>2) ? (2) : ((int)rectCategory[i].size());
        for (int j=0;j<1;++j)
        {
            //Ŀ����ζ�Ӧͼ������0~3
            imagePoints2d[0]=Point2d(rectCategory[i][j].m_points[0].x,rectCategory[i][j].m_points[0].y);
            imagePoints2d[1]=Point2d(rectCategory[i][j].m_points[1].x,rectCategory[i][j].m_points[1].y);
            imagePoints2d[2]=Point2d(rectCategory[i][j].m_points[2].x,rectCategory[i][j].m_points[2].y);
            imagePoints2d[3]=Point2d(rectCategory[i][j].m_points[3].x,rectCategory[i][j].m_points[3].y);

            // ��׼Rect��2d�ռ�Ϊ100*100�ľ���
            Size m_RectSize = Size(130, int(130*1.25));
            // ���� 4���ǵ������ͶӰ��׼ֵ
            vector<Point2f> m_RectCorners2d;
            //vector<Point3f> m_RectCorners3d;
            m_RectCorners2d.push_back(Point2f(0, 0));
            m_RectCorners2d.push_back(Point2f(float(m_RectSize.width-1), 0));
            m_RectCorners2d.push_back(Point2f(float(m_RectSize.width-1), float(m_RectSize.height-1)));
            m_RectCorners2d.push_back(Point2f(0, float(m_RectSize.height-1) ) );

            // ͶӰ�任,�ָ�2ά��׼��ͼ
            Mat canonicalImg;
            // �õ���ǰmarker��͸�ӱ任����M
            Mat M = getPerspectiveTransform(rectCategory[i][j].m_points, m_RectCorners2d);
            // ����ǰ��marker�任Ϊ����ͶӰ
            warpPerspective(srcImg, canonicalImg, M, m_RectSize);
            //�洢��������ͼ��
            canonicalImg.copyTo(rectCategory[i][j].perspectiveImg);
            //�ִ�����ʾ����׼��������
            sprintf(windowName,"capturePerspectiveImg-%d",j);
            imshow(windowName, rectCategory[i][j].perspectiveImg);
        }
    }
    return;
}


double DetectRectToGetImageLightness::HSVCenterROI( )
{
    Mat srct = rectCategory[0][0].perspectiveImg.clone();
    double HSV_h_value;
    double HSV_s_value;
    double HSV_v_value;

    Mat hsv;
    cvtColor(srct,hsv,CV_BGR2HSV_FULL);
    Mat srctShow;
    srct.copyTo(srctShow);

    Mat ROI = hsv;
    double HSV_h=0;
    double HSV_s=0;
    double HSV_v=0;

     for(unsigned int j=0;j < ROI.rows;j++)
     {
        uchar* data=ROI.ptr<uchar>(j);
        for(unsigned int p=0;p<ROI.cols*3;p=p+3)
        {
            HSV_h=HSV_h+data[p];
            HSV_s=HSV_s+data[p+1];
            HSV_v=HSV_v+data[p+2];
         }
      }

      HSV_h_value=HSV_h/(ROI.rows*ROI.cols);
      HSV_s_value=HSV_s/(ROI.rows*ROI.cols);
      HSV_v_value=HSV_v/(ROI.rows*ROI.cols);
      return HSV_v_value;
}


double DetectRectToGetImageLightness::GetRectAreaLightness(Mat& inputImg)
{
    double brightness = 0;
    resize(inputImg,srcImg,Size(int(inputImg.cols*shrink),int(inputImg.rows*shrink)),0,0,INTER_AREA);
    resultImg = srcImg.clone();

    RectangleDetect( );
    EstimatePosition( );
    if ( rectCategory.size() >= 1 )
    {
        GetTheLargestRect();
        PerspectiveTransformation( );
        rect_area_lightness = HSVCenterROI();
        brightness = rect_area_lightness;
    }
    //printf("Lightness=%.2f\n",rect_area_lightness);

    char light[50];
    sprintf(light,"L:%4.1f",rect_area_lightness);
    Point showPoint;
    showPoint=Point2d(2,20);
    putText(resultImg, light, showPoint, CV_FONT_HERSHEY_PLAIN,1.5,Scalar(0,0,255),int(2.5));

    rectCategory.clear();
    rectPossible.clear();
    rect_area_lightness = 0;

    imshow("Lightness",resultImg);
    int c = waitKey(1);
    if (113 == c)
        exit(1);
    return brightness;
}

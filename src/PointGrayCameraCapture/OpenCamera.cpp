#include "declare.h"


extern bool copyIsRunningFlag;
extern bool rawImgHasCopiedOut;
extern Mat g_rawImage;
VideoCapture capture;

////Opencv����ɼ��߳�
void* CameraCapture(void*)
{
    int rate = 15;
    ros::NodeHandle rawImgPubNode;
    image_transport::ImageTransport it(rawImgPubNode);
    image_transport::Publisher pub;
    pub = it.advertise("camera/raw_image",  1 );
    ros::Rate loop_rate( rate );

    while ( rawImgPubNode.ok() )
    {
        Mat captureImg;
        capture>>captureImg;
        copyIsRunningFlag = true;
        captureImg.copyTo(g_rawImage);
        copyIsRunningFlag = false;
        rawImgHasCopiedOut = false;
        usleep(1000);


        sensor_msgs::ImagePtr msg = cv_bridge::CvImage(std_msgs::Header(), "bgr8", captureImg).toImageMsg();
        //˵��Ҫ��������Ϣ����
         pub.publish(msg);
         loop_rate.sleep();

    }
}

////����Opencv����ɼ��߳�
void CreateCameraCaptureThread()
{
    pthread_t captureThread;
    int ret=pthread_create(&captureThread,NULL,CameraCapture,NULL);
    if(ret!=0)
    {
        //�̴߳���ʧ��
        printf ("Create camera capture thread error!..\n");
        exit (1);
    }
    return;
}

////Opencv��ʽ��UVC����ͷ
int OpenCameraByOpencv(int cameraNo)
{
    //opencv��ʽ�����
    capture.open(cameraNo);
    capture.set(CV_CAP_PROP_FRAME_WIDTH,640);
    capture.set(CV_CAP_PROP_FRAME_HEIGHT,480);
    double fps = capture.get(CV_CAP_PROP_FPS);

    if(false == capture.isOpened())
    {
        cout<<"Open camera failed..."<<endl;
        return -1;
    }
    CreateCameraCaptureThread();
    return 1;
}

////�����
void OpenCamera(int openCameraMode, int cameraNo)
{
    //����һ�����߳����ڲ����������
    //OpenGetKeyThread();

    //if (MVMODE == openCameraMode)
    //{
    //    ////open camera with SDK
    //    if( -1 == OpenCameraWithMvSDK( ) )
    //        exit(-1);
    //}

    if (POINTGRAYCAMERA == openCameraMode)
    {
        ////open camera with pg sdk
        //if( -1 == OpenPointGrayCamera( ) )
        //     exit(-1);
        return;
     }

    else if (OPENCV_VIDEOCAPTURE == openCameraMode)
    {
        ////open camera with opencv
        if( -1 == OpenCameraByOpencv( cameraNo ) )
             exit(-1);
     }
    else
    {
        //read offline data
        if( -1 == ReadOfflineData(  ) )
             exit(-1);
    }
    return;
}

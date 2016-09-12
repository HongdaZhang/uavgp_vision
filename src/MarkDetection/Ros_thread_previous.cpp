#include "declare.h"


int k;
extern Mat g_rectResultImg;
extern image_transport::Publisher pub;
extern bool exit_flag;

void* RosThread(void*)
{
    ros::Rate loop_rate(10);

    while( false == exit_flag )
    {
        if( !g_rectResultImg.data )
        {
            sleep(50);
            continue;
        }

        //��Mat��ͼ��ת��Ϊros����Ϣ����
        sensor_msgs::ImagePtr msg = cv_bridge::CvImage(std_msgs::Header(), "bgr8", g_rectResultImg).toImageMsg();
        //˵��Ҫ��������Ϣ����
         pub.publish(msg);
         //ִ��һ��ros��Ϣ����
         ros::spinOnce();
         //����Ϣ������ѭ��Ƶ������һ��ʱ��
         loop_rate.sleep();

//       printf("ros_thread is running..%d\n",k);
//       k++;
    }
    pthread_exit(0);
}

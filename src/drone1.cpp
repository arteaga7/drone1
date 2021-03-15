/* drone1_node: Se subscribe a los tópicos:
 *  "/ground_truth/state" para obtener la postura de un Ardrone (de Gazebo)
 *  "/ardrone/navdata" para obtener los ángulos de orientación medidos por el Ardrone
 *  Publica en los tópicos:
    "/cmd_vel" para asignar angulos.
    "/ardrone/takeoff" para despegar el drone
 * El launch correspondiente es:
 * $ roslaunch cvg_sim_gazebo ardrone_testworld.launch
*/
#include "ros/ros.h"
#include "std_msgs/Empty.h"
#include "nav_msgs/Odometry.h"
#include "geometry_msgs/Twist.h"
#include "ardrone_autonomy/Navdata.h"
#include "tf/transform_broadcaster.h"

using namespace std;
//Variables globales
ros::Publisher  takeoff_publisher, land_publisher, velocity_publisher;
//Crea objetos "takeoff_publisher, land_publisher, velocity_publisher" de tipo "Publisher"
ros::Subscriber pose_subscriber, angulos_subscriber;
//Crea objetos "pose_subscriber,angulos_subscriber" de tipo "Subscriber"
geometry_msgs::Twist vel;   //Crea objeto "vel" para mensajes tipo "geometry_msgs/Twist"
double x, y, z, roll, pitch, yaw, alab,cab;
const double PI = 3.14159265359;

void poseCallback(const nav_msgs::Odometry::ConstPtr msg){
  x = msg->pose.pose.position.x;
  y = msg->pose.pose.position.y;
  z = msg->pose.pose.position.z;
  //Operaciones para obtener los angulos de Euler a partir de Quaterniones
  tf::Quaternion quater;
  tf::quaternionMsgToTF(msg->pose.pose.orientation, quater);
  tf::Matrix3x3(quater).getRPY(roll, pitch, yaw);  //Angulos en radianes
}
geometry_msgs::Twist movimiento (float x, float y, float z, float turn) {
        vel.linear.x = x;       vel.linear.y = y;
        vel.linear.z = z;       vel.angular.z = turn;
        return(vel);
}
void takeoff (void) {
    std_msgs::Empty empty;          //Crea objeto "empty" para mensajes tipo "std_msgs/Empty"
    takeoff_publisher.publish(empty);
    cout<< "Despegando..." << endl;
    vel = movimiento(0,0,0,0);
    velocity_publisher.publish(vel);
}
void alabeo_cabeceo(const ardrone_autonomy::Navdata::ConstPtr &msg){
    alab = msg->rotX; //ángulo de alabeo obtenido del ardrone [grados]
    cab = msg->rotY;  //ángulo de cabeceo obtenido del ardrone [grados]
}

int main(int argc, char **argv)
{
ros::init(argc, argv, "drone1_node");  //Inicializa el nodo
ros::NodeHandle n;                     //Crea el handle
int cont=0;
takeoff_publisher = n.advertise<std_msgs::Empty>("/ardrone/takeoff",1,true);
//Mensaje "std_msgs/Empty" correspondiente al topico "/ardrone/takeoff", buffer=1,true
land_publisher = n.advertise<std_msgs::Empty>("/ardrone/land",1,true);
velocity_publisher = n.advertise<geometry_msgs::Twist>("/cmd_vel", 1);
//Mensaje "geometry_msgs/Twist" correspondiente al topico "/cmd_vel", buffer=1
pose_subscriber = n.subscribe("/ground_truth/state", 1, poseCallback);
//Topico "/ground_truth/state", buffer=1, funcion de callback
angulos_subscriber = n.subscribe("/ardrone/navdata", 1, alabeo_cabeceo);
takeoff();
//Hoover
vel.linear.x = 0;  //thetad
vel.linear.y = 0;  //phid
vel.linear.z = 0;  //dzd
vel.angular.z = 0; //wyawd
ros::Rate loop_rate(100);  //Frecuencia de ejecucion = 100 Hz
  while(ros::ok()){
        velocity_publisher.publish(vel);
        if(cont==100){
            printf("x: %f y: %f z: %f alab: %f cab: %f yaw: %f\n", x,y,z,alab,cab,yaw);
            cont=0;
        }else{cont++;}
        ros::spinOnce();    //Activa callbacks
        loop_rate.sleep();  //Espera lo necesario para completar el loop rate
  }
return 0;
}

// Author of SSL_SLAM2: Wang Han 
// Email wh200720041@gmail.com
// Homepage https://wanghan.pro
#include "laserProcessingClass.h"

void LaserProcessingClass::init(lidar::Lidar lidar_param_in){
    
    lidar_param = lidar_param_in;

}

void LaserProcessingClass::featureExtraction(const pcl::PointCloud<pcl::PointXYZRGB>::Ptr& pc_in, pcl::PointCloud<pcl::PointXYZRGB>::Ptr& pc_out_edge, pcl::PointCloud<pcl::PointXYZRGB>::Ptr& pc_out_surf){

    std::vector<int> indices;
    pcl::removeNaNFromPointCloud(*pc_in, indices);
   
    // coordinate transform
    // 把相机坐标系下的点云转换到body坐标系。(z轴向前->z轴向上)
    for (int i = 0; i < (int) pc_in->points.size(); i++){
        double new_x = pc_in->points[i].z;
        double new_y = -pc_in->points[i].x;
        double new_z = -pc_in->points[i].y;
        pc_in->points[i].x = new_x;
        pc_in->points[i].y = new_y;
        pc_in->points[i].z = new_z;
    }

    std::vector<pcl::PointCloud<pcl::PointXYZRGB>::Ptr> laserCloudScans;

    // 论文Sec. III.A节
    double last_angle = atan2(pc_in->points[0].z, pc_in->points[0].y) * 180 / M_PI;    // arctan(z/y)
    // 记录当前角度区间内点的数量
    int count = 0;
    // 获取输入点云的点的数量
    int point_size = pc_in->points.size() - 1;

    // 首先将无序的点云，投影到二维点矩阵中。
    for (int i = 0; i < (int) pc_in->points.size(); i++)
    {
        // pc_in->points[i].intensity = (double)i / pc_in->points.size();
        // int scanID = 0;
        
        // 相机到点的距离d
        // double distance = sqrt(pc_in->points[i].x * pc_in->points[i].x + pc_in->points[i].y * pc_in->points[i].y + pc_in->points[i].z * pc_in->points[i].z);
        
        // 点与x轴的水平夹角theta_i，含义见公式(1)
        double angle = atan2(pc_in->points[i].x, pc_in->points[i].z) * 180 / M_PI;    // arctan(x/z)
        
        count++;
        // 检查当前点与上一个点的角度差，如果超过阈值 0.05 deg，则将当前点之前的所有点组成的子点云加入laserCloudScans中。
        if(fabs(angle - last_angle) > 0.05){
            
            if(count > 30){
                pcl::PointCloud<pcl::PointXYZRGB>::Ptr pc_temp(new pcl::PointCloud<pcl::PointXYZRGB>());
                
                // 当前点的索引为i
                for(int k = 0; k < count; k++){
                    pc_temp->push_back(pc_in->points[i - count + k + 1]);
                }
                
                if(pc_temp->points.size() > 0){
                    laserCloudScans.push_back(pc_temp);
                }
            }
            // 重置计数器 count，并更新last_angle。
            count =0;
            last_angle = angle;
        }
    }

    //ROS_WARN_ONCE("total points array %d", laserCloudScans.size());

    for(int i = 0; i < laserCloudScans.size(); i++){
        
        std::vector<Double2d> cloudCurvature; 
        int total_points = laserCloudScans[i]->points.size()-10;
        for(int j = 5; j < (int)laserCloudScans[i]->points.size() - 5; j++){
            double angle_difference = fabs((atan2(laserCloudScans[i]->points[j-5].y, laserCloudScans[i]->points[j-5].z)- atan2(laserCloudScans[i]->points[j+5].y,laserCloudScans[i]->points[j+5].z)) * 180 / M_PI); 
            if(angle_difference>5){
                //consider as a surf points
                pc_out_surf->push_back(laserCloudScans[i]->points[j]);              
                continue;  
            }

            // 计算每个点的曲率
            double diffX = laserCloudScans[i]->points[j - 5].x + laserCloudScans[i]->points[j - 4].x + laserCloudScans[i]->points[j - 3].x + laserCloudScans[i]->points[j - 2].x + laserCloudScans[i]->points[j - 1].x - 10 * laserCloudScans[i]->points[j].x + laserCloudScans[i]->points[j + 1].x + laserCloudScans[i]->points[j + 2].x + laserCloudScans[i]->points[j + 3].x + laserCloudScans[i]->points[j + 4].x + laserCloudScans[i]->points[j + 5].x;
            double diffY = laserCloudScans[i]->points[j - 5].y + laserCloudScans[i]->points[j - 4].y + laserCloudScans[i]->points[j - 3].y + laserCloudScans[i]->points[j - 2].y + laserCloudScans[i]->points[j - 1].y - 10 * laserCloudScans[i]->points[j].y + laserCloudScans[i]->points[j + 1].y + laserCloudScans[i]->points[j + 2].y + laserCloudScans[i]->points[j + 3].y + laserCloudScans[i]->points[j + 4].y + laserCloudScans[i]->points[j + 5].y;
            double diffZ = laserCloudScans[i]->points[j - 5].z + laserCloudScans[i]->points[j - 4].z + laserCloudScans[i]->points[j - 3].z + laserCloudScans[i]->points[j - 2].z + laserCloudScans[i]->points[j - 1].z - 10 * laserCloudScans[i]->points[j].z + laserCloudScans[i]->points[j + 1].z + laserCloudScans[i]->points[j + 2].z + laserCloudScans[i]->points[j + 3].z + laserCloudScans[i]->points[j + 4].z + laserCloudScans[i]->points[j + 5].z;
            Double2d distance(j, diffX * diffX + diffY * diffY + diffZ * diffZ);
            cloudCurvature.push_back(distance);
        }

        featureExtractionFromSector(laserCloudScans[i], cloudCurvature, pc_out_edge, pc_out_surf);    
    }

    //remove ground point
    /*
    pcl::PointIndices::Ptr inliers(new pcl::PointIndices());
    for(int i=0;i<pc_out_edge->points.size();i++){
        if(pc_out_edge->points[i].z<=-0.55)
            inliers->indices.push_back(i);
    }
    pcl::ExtractIndices<pcl::PointXYZRGB> extract;
    extract.setInputCloud(pc_out_edge);
    extract.setIndices(inliers);
    extract.setNegative(true);
    extract.filter(*pc_out_edge);
*/

}


void LaserProcessingClass::featureExtractionFromSector(const pcl::PointCloud<pcl::PointXYZRGB>::Ptr& pc_in, std::vector<Double2d>& cloudCurvature, pcl::PointCloud<pcl::PointXYZRGB>::Ptr& pc_out_edge, pcl::PointCloud<pcl::PointXYZRGB>::Ptr& pc_out_surf){

    std::sort(cloudCurvature.begin(), cloudCurvature.end(), [](const Double2d & a, const Double2d & b)
    { 
        return a.value < b.value; 
    });

    int largestPickedNum = 0;
    std::vector<int> picked_points;
    int point_info_count =0;
    for (int i = cloudCurvature.size()-1; i >= 0; i--)
    {
        int ind = cloudCurvature[i].id; 
        if(std::find(picked_points.begin(), picked_points.end(), ind)==picked_points.end()){
            if(cloudCurvature[i].value <= 0.1){
                break;
            }
            
            largestPickedNum++;
            picked_points.push_back(ind);
            
            if (largestPickedNum <= 10){
                pc_out_edge->push_back(pc_in->points[ind]);
                point_info_count++;
            }else{
                break;
            }

            for(int k=1;k<=5;k++){
                double diffX = pc_in->points[ind + k].x - pc_in->points[ind + k - 1].x;
                double diffY = pc_in->points[ind + k].y - pc_in->points[ind + k - 1].y;
                double diffZ = pc_in->points[ind + k].z - pc_in->points[ind + k - 1].z;
                if (diffX * diffX + diffY * diffY + diffZ * diffZ > 0.05){
                    break;
                }
                picked_points.push_back(ind+k);
            }
            for(int k=-1;k>=-5;k--){
                double diffX = pc_in->points[ind + k].x - pc_in->points[ind + k + 1].x;
                double diffY = pc_in->points[ind + k].y - pc_in->points[ind + k + 1].y;
                double diffZ = pc_in->points[ind + k].z - pc_in->points[ind + k + 1].z;
                if (diffX * diffX + diffY * diffY + diffZ * diffZ > 0.05){
                    break;
                }
                picked_points.push_back(ind+k);
            }

        }
    }

    for (int i = 0; i <= (int)cloudCurvature.size()-1; i++)
    {
        int ind = cloudCurvature[i].id; 
        if( std::find(picked_points.begin(), picked_points.end(), ind)==picked_points.end())
        {
            pc_out_surf->push_back(pc_in->points[ind]);
        }
    }
}

LaserProcessingClass::LaserProcessingClass(){
    
}

Double2d::Double2d(int id_in, double value_in){
    id = id_in;
    value =value_in;
};

PointsInfo::PointsInfo(int layer_in, double time_in){
    layer = layer_in;
    time = time_in;
};
